#include "Workload_LU.hpp"
#include <climits>

using namespace std;

void Workload_LU::setupDataInfo(){
  datas_info.push_back(data_info("L", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  datas_info.push_back(data_info("U", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  datas_info.push_back(data_info("A", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
}

void Workload_LU::genSubblkSet(vector<array<int,3>>& sb_dim_set){
  int step = 20;
  for(int subblk_dimi=1; subblk_dimi<=para.axes["i"].input_size; subblk_dimi+=step){
    if(subblk_dimi+1+1 > global_sp->total_port){
      break;
    }
    for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj+=step){
      if(subblk_dimi+subblk_dimj+1 > global_sp->total_port){
        break;
      }
      //too small
      if(subblk_dimi+subblk_dimj+subblk_dimi*subblk_dimj < (global_sp->total_port-2)){
        continue;
      }
      sb_dim_set.push_back(array<int,3>{subblk_dimi, subblk_dimj, 1});
    }
  }
}

void Workload_LU::updatePara_LU(){
  int l_size = para.axes["i"].subblk_dim;
  int a_size = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;

  int a_row_size = para.axes["i"].subblk_dim;
  int cycles_read_a_row = (a_row_size%para.dblks["A"].num_port==0)?a_row_size/para.dblks["A"].num_port:a_row_size/para.dblks["A"].num_port+1;
  int cycles_read_a = (a_size%para.dblks["A"].num_port==0)?a_size/para.dblks["A"].num_port:a_size/para.dblks["A"].num_port+1;
  int cycles_read_a_remain = cycles_read_a - cycles_read_a_row;
  int cycles_read_l = (l_size%para.dblks["L"].num_port==0)?l_size/para.dblks["L"].num_port:l_size/para.dblks["L"].num_port+1;

  int num_cb_div = para.axes["i"].subblk_dim;
  int num_cb_accmul = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;

  int max_num_subblk_ingroup = fus["sub"];

  int cur_num_cycles = 0;
  if(cycles_read_l > 1){
    //required more than one cycle to access l: share l with multiple batches
    for(int l_it=0; l_it<para.axes["i"].blk_dim; l_it+=para.axes["i"].subblk_dim){
      int l_depth = min(para.axes["i"].blk_dim,l_it+para.axes["i"].subblk_dim); //max depth for batches in this l_it
      int group_span = max_num_subblk_ingroup*para.axes["j"].subblk_dim;
      for(int group_start=0; group_start<para.axes["j"].blk_dim; group_start+=group_span){
        //for each group
	int layers_start_with_writel = max(min(group_span, l_depth-group_start),0);
	if(layers_start_with_writel >= 1){
	  cur_num_cycles += max(cycles_read_l, fus["div"]+fus["mul"]+fus["sub"])*(layers_start_with_writel-1)+fus["div"];
	}
	int layers_start_with_readl = min(group_start,l_depth);
	cur_num_cycles += max(fus["sub"],cycles_read_l)*layers_start_with_readl;
	if(group_start == 0){
	  cur_num_cycles += max_num_subblk_ingroup*(cycles_read_a_row-1);
	  cur_num_cycles += max_num_subblk_ingroup*max(0,cycles_read_a_remain-(fus["div"]+fus["mul"]));
	}
	else{
	  cur_num_cycles += max_num_subblk_ingroup*(cycles_read_a-1);
	}
      }
    }
  }
  else{
    int num_cbatches = para.axes["i"].num_subblk_inblk*para.axes["j"].num_subblk_inblk;
    vector<int> cbatch_depth(num_cbatches,0);
    for(int si=0; si<para.axes["i"].num_subblk_inblk; si++){
      for(int sj=0; sj<para.axes["j"].num_subblk_inblk; sj++){
        cbatch_depth[si*para.axes["j"].num_subblk_inblk+sj] = min((si+1)*para.axes["i"].subblk_dim, (sj+1)*para.axes["j"].subblk_dim)-1;
      }
    }
    map<int,int> active_sb;
    //initialize active_sb;
    int si = 0;
    while((active_sb.size()!=max_num_subblk_ingroup) && (si<num_cbatches)){
      if(cbatch_depth[si] != 0){
        active_sb[si] = 0;
      }
      si++;
    }
    int num_latency = 0;
    int num_batch_lastrun;
    while(!active_sb.empty()){
      vector<int> full;
      num_latency++;
      int num_sb = active_sb.size();
      for(auto& sb: active_sb){
        sb.second++;
	if(sb.second == cbatch_depth[sb.first]){
	  full.push_back(sb.first);
	}
      }
      for(auto& sb: full){
        int next_sb = (active_sb.rbegin()->first)+1;
	active_sb.erase(sb);
	if((next_sb < num_cbatches) && (cbatch_depth[next_sb] != 0)){
	  active_sb[next_sb] = 0;
	}
      }
      if(active_sb.empty()){
        num_batch_lastrun = num_sb;
      }
    }
    cur_num_cycles = num_latency*fus["sub"];

    //add latency because of l
    int num_gen_l = 0;
    for(int li=0; li<para.axes["i"].num_subblk_inblk; li++){
      num_gen_l += (li+1)*para.axes["i"].subblk_dim-1;
    }
    cur_num_cycles += num_gen_l*(fus["div"]+fus["mul"]-1);
    cur_num_cycles += num_cbatches*(cycles_read_a-1);
    cur_num_cycles += num_batch_lastrun-1;
  }
  para.blk_cycles["lu"] = cur_num_cycles;
  int use_sram = 0;
  for(int si=0; si<para.axes["i"].num_subblk_inblk; si++){
    for(int sj=0; sj<para.axes["j"].num_subblk_inblk; sj++){
      int i_start = si*para.axes["i"].subblk_dim;
      int j_start = sj*para.axes["j"].subblk_dim;
      int i_end = min((si+1)*para.axes["i"].subblk_dim, para.axes["i"].blk_dim);
      int j_end = min((sj+1)*para.axes["j"].subblk_dim, para.axes["j"].blk_dim);
      use_sram += (i_end-i_start)*(j_end-j_start); //A access

      int depth = min(i_end, j_end);
      for(int d=0; d<depth; d++){
        use_sram += i_end-max(i_start,d);
	use_sram += j_end-max(j_start,d);
      }
    }
  }
  para.mntemps["lu"].use_sram = use_sram;

  para.total_num_fus = 0;
  para.cbs["lu"]["submul"].num_fus = num_cb_accmul;
  para.cbs["lu"]["submul"].num_cb = num_cb_accmul;
  para.cbs["lu"]["submul"].k_stage = 1;
  para.cbs["lu"]["submul"].num_regs = 4*para.cbs["lu"]["submul"].num_fus;
  para.total_num_fus += para.cbs["lu"]["submul"].num_fus;
  para.cbs["lu"]["div"].num_fus = num_cb_div;
  para.cbs["lu"]["div"].num_cb = num_cb_div;
  para.cbs["lu"]["div"].num_regs = num_cb_div;
  para.total_num_fus += para.cbs["lu"]["div"].num_fus;
}

void Workload_LU::updatePara_LUCPL(){
  int l_size = para.axes["i"].subblk_dim;
  int a_size = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;
  int a_row_size = para.axes["i"].subblk_dim;
  int cycles_read_a_row = (a_row_size%para.dblks["A"].num_port==0)?a_row_size/para.dblks["A"].num_port:a_row_size/para.dblks["A"].num_port+1;
  int cycles_read_a = (a_size%para.dblks["A"].num_port==0)?a_size/para.dblks["A"].num_port:a_size/para.dblks["A"].num_port+1;
  int cycles_read_a_remain = cycles_read_a - cycles_read_a_row;
  int cycles_read_l = (l_size%para.dblks["L"].num_port==0)?l_size/para.dblks["L"].num_port: l_size/para.dblks["L"].num_port+1;

  int num_cb_div = para.axes["i"].subblk_dim;
  int num_cb_accmul = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;

  int max_num_subblk_ingroup = fus["sub"];
  int cur_num_cycles = 0;

  if(cycles_read_l > 1){
    //per l iteration
    int group_span = max_num_subblk_ingroup*para.axes["j"].subblk_dim;
    for(int group_start=0; group_start<para.axes["j"].blk_dim; group_start+=group_span){
      //layers start with writing l
      int layers_start_with_writel = min(group_span, para.axes["j"].blk_dim-group_start);
      cur_num_cycles += max(cycles_read_l, fus["div"]+fus["mul"]+fus["sub"])*(layers_start_with_writel-1)+fus["div"];
      //layers start with reading l
      cur_num_cycles += group_start*max(fus["sub"],cycles_read_l);
      if(group_start == 0){
        cur_num_cycles += max_num_subblk_ingroup*(cycles_read_a_row-1);
	cur_num_cycles += max_num_subblk_ingroup*max(0,cycles_read_a_remain-(fus["div"]+fus["mul"]));
      }
      else{
        cur_num_cycles += max_num_subblk_ingroup*(cycles_read_a-1);
      }
    }
    //number of l iterations
    cur_num_cycles *= para.axes["i"].num_subblk_inblk;
  }
  else{
    int num_cbatches = para.axes["i"].num_subblk_inblk*para.axes["j"].num_subblk_inblk;
    vector<int> cbatch_depth(num_cbatches,0);
    for(int si=0; si<para.axes["i"].num_subblk_inblk; si++){
      for(int sj=0; sj<para.axes["j"].num_subblk_inblk; sj++){
        cbatch_depth[si*para.axes["j"].num_subblk_inblk+sj] = (sj+1)*para.axes["j"].subblk_dim;
      }
    }
    map<int,int> active_sb;
    int si=0;
    while((active_sb.size()!=max_num_subblk_ingroup) && (si<num_cbatches)){
      if(cbatch_depth[si] != 0){
        active_sb[si] = 0;
      }
      si++;
    }
    int num_latency = 0;
    int num_batch_lastrun;
    while(!active_sb.empty()){
      vector<int> full;
      num_latency++;
      int num_sb = active_sb.size();
      for(auto& sb: active_sb){
        sb.second++;
	if(sb.second == cbatch_depth[sb.first]){
	  full.push_back(sb.first);
	}
      }
      for(auto& sb: full){
        int next_sb = (active_sb.rbegin()->first)+1;
	active_sb.erase(sb);
	if((next_sb < num_cbatches) && (cbatch_depth[next_sb] !=0)){
	  active_sb[next_sb] = 0;
	}
      }
      if(active_sb.empty()){
        num_batch_lastrun = num_sb;
      }
    }
    cur_num_cycles = num_latency*fus["sub"];
    int num_gen_l = para.axes["i"].num_subblk_inblk*para.axes["l"].blk_dim;
    cur_num_cycles += num_gen_l*(fus["div"]+fus["mul"]-1);
    cur_num_cycles += num_cbatches*(cycles_read_a-1);
    cur_num_cycles += num_batch_lastrun-1;
  }

  int use_sram = 0;
  for(int si=0; si<para.axes["i"].num_subblk_inblk; si++){
    for(int sj=0; sj<para.axes["j"].num_subblk_inblk; sj++){
      int i_start = si*para.axes["i"].subblk_dim;
      int j_start = sj*para.axes["j"].subblk_dim;
      int i_end = min((si+1)*para.axes["i"].subblk_dim,para.axes["i"].blk_dim);
      int j_end = min((sj+1)*para.axes["j"].subblk_dim,para.axes["j"].blk_dim);
      use_sram += (i_end-i_start)*(j_end-j_start); //A access

      int depth = min(i_end, j_end);
      for(int d=0; d<depth; d++){
        use_sram += i_end-i_start;
	use_sram += j_end-max(j_start,d);
      }
    }
  }
  para.mntemps["lucpl"].use_sram = use_sram;
  para.blk_cycles["lucpl"] = cur_num_cycles;

  para.total_num_fus = 0;
  para.cbs["lucpl"]["submul"].num_fus = num_cb_accmul;
  para.cbs["lucpl"]["submul"].num_cb = num_cb_accmul;
  para.cbs["lucpl"]["submul"].k_stage = 1;
  para.cbs["lucpl"]["submul"].num_regs = 4*para.cbs["lucpl"]["submul"].num_fus;
  para.total_num_fus += para.cbs["lucpl"]["submul"].num_fus;
  para.cbs["lucpl"]["div"].num_fus = num_cb_div;
  para.cbs["lucpl"]["div"].num_cb = num_cb_div;
  para.cbs["lucpl"]["div"].num_regs = num_cb_div;
  para.total_num_fus += para.cbs["lucpl"]["div"].num_fus;
}

void Workload_LU::updatePara_TRS(){
  int u_size = para.axes["j"].subblk_dim;
  int a_size = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;
  int a_row_size = para.axes["j"].subblk_dim;
  int cycles_read_a_row = (a_row_size%para.dblks["A"].num_port==0)?a_row_size/para.dblks["A"].num_port:a_row_size/para.dblks["A"].num_port+1;
  int cycles_read_a = (a_size%para.dblks["A"].num_port==0)?a_size/para.dblks["A"].num_port:a_size/para.dblks["A"].num_port+1;
  int cycles_read_a_remain = cycles_read_a - cycles_read_a_row;
  int cycles_read_u = (u_size%para.dblks["U"].num_port==0)?u_size/para.dblks["U"].num_port:u_size/para.dblks["U"].num_port+1;

  int num_cb_div = para.axes["j"].subblk_dim;
  int num_cb_accmul = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;

  int max_num_subblk_ingroup = fus["sub"];
  int cur_num_cycles = 0;

  if(cycles_read_u > 1){
    int group_span = max_num_subblk_ingroup*para.axes["i"].subblk_dim;
    for(int group_start=0; group_start<para.axes["i"].blk_dim; group_start+=group_span){
      //layers start with writing u
      int layers_start_with_writeu = min(group_span,para.axes["i"].blk_dim-group_start);
      cur_num_cycles += max(cycles_read_u, fus["div"]+fus["mul"]+fus["sub"])*(layers_start_with_writeu-1)+fus["div"];
      //layers start with reading u
      cur_num_cycles += group_start*max(fus["sub"],cycles_read_u);
      if(group_start == 0){
        cur_num_cycles += max_num_subblk_ingroup*(cycles_read_a_row-1);
	cur_num_cycles += max_num_subblk_ingroup*max(0,cycles_read_a_remain-(fus["div"]+fus["mul"]));
      }
      else{
        cur_num_cycles += max_num_subblk_ingroup*(cycles_read_a-1);
      }
    }

    //number of u iterations
    cur_num_cycles *= para.axes["j"].num_subblk_inblk;
  }
  else{
    int num_cbatches = para.axes["i"].num_subblk_inblk*para.axes["j"].num_subblk_inblk;
    vector<int> cbatch_depth(num_cbatches,0);
    for(int sj=0; sj<para.axes["j"].num_subblk_inblk; sj++){
      for(int si=0; si<para.axes["i"].num_subblk_inblk; si++){
        cbatch_depth[sj*para.axes["i"].num_subblk_inblk+si] = (si+1)*para.axes["i"].subblk_dim;
      }
    }
    map<int,int> active_sb;
    //initialize active_sb
    int si = 0;
    while((active_sb.size()!=max_num_subblk_ingroup) && (si<num_cbatches)){
      if(cbatch_depth[si] != 0){
        active_sb[si] = 0;
      }
      si++;
    }
    int num_latency = 0;
    int num_batch_lastrun;
    while(!active_sb.empty()){
      vector<int> full;
      num_latency++;
      int num_sb = active_sb.size();
      for(auto& sb: active_sb){
        sb.second++;
	if(sb.second == cbatch_depth[sb.first]){
	  full.push_back(sb.first);
	}
      }
      for(auto& sb: full){
        int next_sb = (active_sb.rbegin()->first)+1;
	active_sb.erase(sb);
	if((next_sb < num_cbatches) && (cbatch_depth[next_sb] != 0)){
	  active_sb[next_sb] = 0;
	}
      }
      if(active_sb.empty()){
        num_batch_lastrun = num_sb;
      }
    }
    cur_num_cycles = num_latency*fus["sub"];
    int num_gen_u = para.axes["j"].num_subblk_inblk*para.axes["l"].blk_dim;
    cur_num_cycles += num_gen_u*(fus["div"]+fus["mul"]-1);
    cur_num_cycles += num_cbatches*(cycles_read_a-1);
    cur_num_cycles += num_batch_lastrun-1;
  }

  int use_sram = 0;
  for(int si=0; si<para.axes["i"].num_subblk_inblk; si++){
    for(int sj=0; sj<para.axes["j"].num_subblk_inblk; sj++){
      int i_start = si*para.axes["i"].subblk_dim;
      int j_start = sj*para.axes["j"].subblk_dim;
      int i_end = min((si+1)*para.axes["i"].subblk_dim,para.axes["i"].blk_dim);
      int j_end = min((sj+1)*para.axes["j"].subblk_dim,para.axes["j"].blk_dim);

      use_sram += (i_end-i_start)*(j_end-j_start); //A access
      int depth = min(i_end, j_end);
      for(int d=0; d<depth; d++){
        use_sram += i_end-max(i_start,d);
	use_sram += j_end-j_start;
      }
    }
  }
  para.mntemps["trs"].use_sram = use_sram;

  para.blk_cycles["trs"] = cur_num_cycles;
  para.total_num_fus = 0;
  para.cbs["trs"]["submul"].num_fus = num_cb_accmul;
  para.cbs["trs"]["submul"].num_cb = num_cb_accmul;
  para.cbs["trs"]["submul"].k_stage = 1;
  para.cbs["trs"]["submul"].num_regs = 4*para.cbs["trs"]["submul"].num_fus;
  para.total_num_fus += para.cbs["trs"]["submul"].num_fus;
  para.cbs["trs"]["div"].num_fus = num_cb_div;
  para.cbs["trs"]["div"].num_cb = num_cb_div;
  para.cbs["trs"]["div"].num_regs = num_cb_div;
  para.total_num_fus += para.cbs["trs"]["div"].num_fus;
}

void Workload_LU::updatePara_SUBMM(){
  int num_cb = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;
  int k_stage = para.axes["l"].subblk_dim;
  int cur_num_fus = num_cb*k_stage;

  int cur_num_regs = 4*cur_num_fus;
  int max_num_subblk_ingroup = k_stage*fus["sub"];

  int num_cbatches = para.axes["i"].num_subblk_inblk*para.axes["j"].num_subblk_inblk;
  int num_group = (num_cbatches%max_num_subblk_ingroup==0)?num_cbatches/max_num_subblk_ingroup:num_cbatches/max_num_subblk_ingroup+1;

  //get the latency of each group
  int pipeline_latency = fus["sub"]*para.axes["l"].blk_dim*num_group;
  int tail_latency = num_cbatches-max_num_subblk_ingroup*(num_group-1)-1;
  int c_size = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;
  int cycles_write_c = (c_size%para.dblks["A"].num_port==0)?c_size/para.dblks["A"].num_port:c_size/para.dblks["A"].num_port+1;
  int write_penalty = 2*num_cbatches*(cycles_write_c-1);
  int initial_latency = fus["mul"];
  int cur_num_cycles = initial_latency+pipeline_latency+tail_latency+write_penalty;

  int sram_a_cbatch = para.axes["i"].subblk_dim*para.axes["l"].blk_dim;
  int sram_b_cbatch = para.axes["j"].subblk_dim*para.axes["l"].blk_dim;
  int sram_c_cbatch = 2*para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;

  int use_sram_cbatch = sram_a_cbatch+sram_b_cbatch+sram_c_cbatch;
  para.mntemps["submm"].use_sram = use_sram_cbatch*num_cbatches;

  para.blk_cycles["submm"] = cur_num_cycles;
  para.cbs["submm"]["submul"].num_fus = cur_num_fus;
  para.cbs["submm"]["submul"].num_cb = num_cb;
  para.cbs["submm"]["submul"].k_stage = k_stage;
  para.cbs["submm"]["submul"].num_regs = cur_num_regs;
  para.total_num_fus = para.cbs["submm"]["submul"].num_fus;
}

unsigned long long Workload_LU::getSpill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2){
  int num_blk = para.axes["i"].num_blk;
  if(lo.loop_idc[2] == "l"){
    if(lo.loop_idc[0] == "i"){
      blk_info& d1 = para.dblks["L"];
      spill1 = SpillCase1(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["U"];
      spill2 = SpillCase2(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      blk_info& d1 = para.dblks["U"];
      spill1 = SpillCase1(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["L"];
      spill2 = SpillCase2(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  else if(lo.loop_idc[2] == "i"){
    if(lo.loop_idc[0] == "j"){
      blk_info& d1 = para.dblks["A"];
      spill1 = SpillCase3(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["L"];
      spill2 = SpillCase4(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      blk_info& d1 = para.dblks["L"];
      spill1 = SpillCase5(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["A"];
      spill2 = SpillCase6(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  else{
    if(lo.loop_idc[0] == "i"){
      blk_info& d1 = para.dblks["A"];
      spill1 = SpillCase3(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["U"];
      spill2 = SpillCase4(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      blk_info& d1 = para.dblks["U"];
      spill1 = SpillCase5(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["A"];
      spill2 = SpillCase6(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  unsigned long long spill = spill1 + spill2;
  return spill;
}

void Workload_LU::optiPara(){
  unsigned long long num_spill = ULLONG_MAX;
  unsigned long long perf = ULLONG_MAX;
  unsigned long long sram_use = ULLONG_MAX;

  vector<array<int,3>> sb_dim_set;
  genSubblkSet(sb_dim_set);

  for(auto &i: sb_dim_set){
    cout << "subblk dim " << i[0] << " " << i[1] << " " << i[2] << endl;
    para.axes["i"].subblk_dim = i[0];
    para.axes["j"].subblk_dim = i[1];
    para.axes["l"].subblk_dim = i[2];

    for(auto &ax: para.axes){
      ax.second.num_subblk = (ax.second.input_size%ax.second.subblk_dim==0)?ax.second.input_size/ax.second.subblk_dim: ax.second.input_size/ax.second.subblk_dim+1;
    }

    int port_per_bank = Memory::membanks[0].num_port;
    para.dblks["L"].num_port = para.axes["i"].subblk_dim;
    para.dblks["U"].num_port = para.axes["j"].subblk_dim;
    para.dblks["A"].num_port = global_sp->total_port-para.dblks["L"].num_port-para.dblks["U"].num_port;
    for(auto &d: para.dblks){
      d.second.num_bank = d.second.num_port;
    }

    for(int blk_dim=max(para.axes["i"].subblk_dim,para.axes["j"].subblk_dim); blk_dim<=para.axes["i"].input_size; blk_dim++){
      if(MinMem(blk_dim, blk_dim, 1) > global_sp->total_size){
        break;
      }
      
      para.axes["i"].blk_dim = blk_dim;
      para.axes["j"].blk_dim = blk_dim;
      para.axes["l"].blk_dim = blk_dim;

      for(auto &ax: para.axes){
        ax.second.num_subblk_inblk = (ax.second.blk_dim%ax.second.subblk_dim==0)?ax.second.blk_dim/ax.second.subblk_dim:ax.second.blk_dim/ax.second.subblk_dim+1;
	ax.second.num_blk = (ax.second.input_size%ax.second.blk_dim==0)?ax.second.input_size/ax.second.blk_dim: ax.second.input_size/ax.second.blk_dim+1;
	ax.second.ex_input_size = ax.second.num_blk*ax.second.blk_dim;
      }

      para.dblks["L"].size = para.axes["i"].blk_dim*para.axes["l"].blk_dim;
      para.dblks["U"].size = para.axes["j"].blk_dim*para.axes["l"].blk_dim;
      para.dblks["A"].size = para.axes["i"].blk_dim*para.axes["j"].blk_dim;

      int bank_size = Memory::membanks[0].size;
      bool can_fit = true;
      for(auto &d: para.dblks){
        d.second.interval = (d.second.size%d.second.num_bank==0)?d.second.size/d.second.num_bank: d.second.size/d.second.num_bank+1;
	d.second.num_blk_mem = bank_size/d.second.interval;
	can_fit = can_fit && (d.second.num_blk_mem >= 1);
      }

      if(can_fit){
        unsigned long long cur_spill = ULLONG_MAX;
	unsigned long long cur_spill1;
	unsigned long long cur_spill2;

	unsigned long long cur_perf;
	unsigned long long cur_sram_use;

	updatePara_LU();
	updatePara_LUCPL();
	updatePara_TRS();
	updatePara_SUBMM();

	para.total_compute_cycles = 0;
	cur_sram_use = 0;
	for(int l=para.axes["i"].num_blk; l>=1; l--){
	  para.total_compute_cycles += para.blk_cycles["lu"];
	  cur_sram_use += para.mntemps["lu"].use_sram;

	  para.total_compute_cycles += (l-1)*para.blk_cycles["lucpl"];
	  cur_sram_use += (l-1)*para.mntemps["lucpl"].use_sram;

	  para.total_compute_cycles += (l-1)*para.blk_cycles["trs"];
	  cur_sram_use += (l-1)*para.mntemps["trs"].use_sram;

	  para.total_compute_cycles += (l-1)*(l-1)*para.blk_cycles["submm"];
	  cur_sram_use += (l-1)*(l-1)*para.mntemps["subm"].use_sram;
	}
	cur_perf = para.total_compute_cycles;

	LoopOrder order1("i","j","l");
	unsigned long long spill11;
	unsigned long long spill12;
	unsigned long long spill1 = getSpill(order1, spill11, spill12);
	if(spill1 <= cur_spill){
	  cur_spill = spill1;
	  para.loop_order = order1;
	  cur_spill1 = spill11;
	  cur_spill2 = spill12;
	}

	LoopOrder order2("j","i","l");
	unsigned long long spill21;
	unsigned long long spill22;
	unsigned long long spill2 = getSpill(order2, spill21, spill22);
	if(spill2 <= cur_spill){
	  cur_spill = spill2;
	  para.loop_order = order2;
	  cur_spill1 = spill21;
	  cur_spill2 = spill22;
	}

	LoopOrder order3("j","l","i");
	unsigned long long spill31;
	unsigned long long spill32;
	unsigned long long spill3 = getSpill(order3, spill31, spill32);
	if(spill3 <= cur_spill){
	  cur_spill = spill3;
	  para.loop_order = order3;
	  cur_spill1 = spill31;
	  cur_spill2 = spill32;
	}

	LoopOrder order4("l","j","i");
	unsigned long long spill41;
	unsigned long long spill42;
	unsigned long long spill4 = getSpill(order4, spill41, spill42);
	if(spill4 <= cur_spill){
	  cur_spill = spill4;
	  para.loop_order = order4;
	  cur_spill1 = spill41;
	  cur_spill2 = spill42;
	}

	LoopOrder order5("i","l","j");
	unsigned long long spill51;
	unsigned long long spill52;
	unsigned long long spill5 = getSpill(order5, spill51, spill52);
	if(spill5 <= cur_spill){
	  cur_spill = spill5;
	  para.loop_order = order5;
	  cur_spill1 = spill51;
	  cur_spill2 = spill52;
	}

	LoopOrder order6("l","i","j");
	unsigned long long spill61;
	unsigned long long spill62;
	unsigned long long spill6 = getSpill(order6, spill61, spill62);
	if(spill6 <= cur_spill){
	  cur_spill = spill6;
	  para.loop_order = order6;
	  cur_spill1 = spill61;
	  cur_spill2 = spill62;
	}

	if((cur_spill < num_spill)
	    || ((cur_spill == num_spill) && (cur_perf < perf))){
	  opti_para = para;
	  opti_para.num_spill = cur_spill;
	  opti_para.num_spill1 = cur_spill1;
	  opti_para.num_spill2 = cur_spill2;

	  num_spill = cur_spill;
	  perf = cur_perf;
	  sram_use = cur_sram_use;
	}
      }
    }
  }
}

void Workload_LU::genMacroNode(){
  int blk_m = opti_para.axes["i"].blk_dim;

  kernal_out_dblk.insert("L");
  kernal_out_dblk.insert("U");

  //generate mns
  map<string, dblk_info> dblks_info_lu;
  dblks_info_lu["L"] = {"LU_L_Tr", "L", "mode_lu_ltr", opti_para.axes["i"].subblk_dim, 0, blk_m, blk_m};
  dblks_info_lu["U"] = {"LU_U_Tr", "U", "mode_lu_utr", 0, opti_para.axes["j"].subblk_dim, blk_m, blk_m};
  dblks_info_lu["A"] = {"LU_A", "A", "mode_lu_a", opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, blk_m, blk_m};
  mn_temp["lu"] = new MN_LU(blk_m, dblks_info_lu);
  mn_temps.push_back(mn_temp["lu"]);

  map<string, dblk_info> dblks_info_lucpl;
  dblks_info_lucpl["A"] = {"LUCPL_A", "A", "mode_lu_a", opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, blk_m, blk_m};
  dblks_info_lucpl["U"] = {"LUCPL_U_Tr", "U", "mode_lu_utr", 0, opti_para.axes["j"].subblk_dim, blk_m, blk_m};
  dblks_info_lucpl["L"] = {"LUCPL_L", "L", "mode_lu_l", opti_para.axes["i"].subblk_dim, 0, blk_m, blk_m};
  mn_temp["lucpl"] = new MN_LUCPL(blk_m, dblks_info_lucpl);
  mn_temps.push_back(mn_temp["lucpl"]);

  map<string, dblk_info> dblks_info_trs;
  dblks_info_trs["A"] = {"TRS_A_Tr", "L", "mode_lu_ltr", opti_para.axes["i"].subblk_dim, 0, blk_m, blk_m};
  dblks_info_trs["B"] = {"TRS_B", "A", "mode_lu_a", opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, blk_m, blk_m};
  dblks_info_trs["X"] = {"TRS_X", "U", "mode_lu_u", 0, opti_para.axes["j"].subblk_dim, blk_m, blk_m};
  dblks_info_trs["Brow"] = {"TRS_Brow", "U", "mode_lu_u", 0, opti_para.axes["j"].subblk_dim, 1, blk_m};
  mn_temp["trs"] = new MN_TRS(blk_m, dblks_info_trs);
  mn_temps.push_back(mn_temp["trs"]);

  map<string, dblk_info> dblks_info_mm;
  dblks_info_mm["A"] = {"A", "L", "mode_mm_a", opti_para.axes["i"].subblk_dim, opti_para.axes["l"].subblk_dim, blk_m, blk_m};
  dblks_info_mm["B"] = {"B", "U", "mode_mm_b", opti_para.axes["l"].subblk_dim, opti_para.axes["j"].subblk_dim, blk_m, blk_m};
  dblks_info_mm["C"] = {"C", "A", "mode_mm_c", opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, blk_m, blk_m};
  mn_temp["submm"] = new MN_SUBMM(blk_m, blk_m, blk_m, dblks_info_mm);
  mn_temps.push_back(mn_temp["submm"]);

  load_store = shared_ptr<LoadStoreDblk>(new LoadStoreDblk);

  func_iteration = &Workload_LU::setDblkUseTime;
  LoopStructure();

  func_iteration = &Workload_LU::runMacroNode;
  LoopStructure();

}

void Workload_LU::LoopStructure(){
  int blk_m = opti_para.axes["i"].num_blk;

  macro_time = 0;
  if(opti_para.loop_order.loop_idc == array<string,3>{"i","j","l"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_j=0; blk_j<blk_m; blk_j++){
        for(blk_l=0; blk_l<=min(blk_i,blk_j); blk_l++){
	  setDblkIdx();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"i","l","j"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_l=0; blk_l<=blk_i; blk_l++){
        for(blk_j=blk_l; blk_j<blk_m; blk_j++){
	  setDblkIdx();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","l","i"}){
    for(blk_j=0; blk_j<blk_m; blk_j++){
      for(blk_l=0; blk_l<=blk_j; blk_l++){
        for(blk_i=blk_l; blk_i<blk_m; blk_i++){
	  setDblkIdx();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","i","l"}){
    for(blk_j=0; blk_j<blk_m; blk_j++){
      for(blk_i=0; blk_i<blk_m; blk_i++){
        for(blk_l=0; blk_l<=min(blk_i, blk_j); blk_l++){
	  setDblkIdx();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"l","i","j"}){
    for(blk_l=0; blk_l<blk_m; blk_l++){
      for(blk_i=blk_l; blk_i<blk_m; blk_i++){
        for(blk_j=blk_l; blk_j<blk_m; blk_j++){
	  setDblkIdx();
	}
      }
    }
  }
  else{
    for(blk_l=0; blk_l<blk_m; blk_l++){
      for(blk_j=blk_l; blk_j<blk_m; blk_j++){
        for(blk_i=blk_l; blk_i<blk_m; blk_i++){
	  setDblkIdx();
	}
      }
    }
  }
}

void Workload_LU::setDblkIdx(){
  dblk_idx.clear();
  dblk_idx["L"] = blk_i*opti_para.axes["i"].num_blk+blk_l;
  dblk_idx["U"] = blk_l*opti_para.axes["i"].num_blk+blk_j;
  dblk_idx["A"] = blk_i*opti_para.axes["i"].num_blk+blk_j;

  //do something
  (this->*func_iteration)();

  macro_time++;
}

void Workload_LU::runMacroNode(){
  in_dblk.clear();
  out_dblk.clear();

  string mn_temp_name;

  if((blk_i == blk_l) && (blk_j == blk_l)){
    in_dblk.insert("A");
    out_dblk.insert("L");
    out_dblk.insert("U");
    mn_temp_name = "lu";
  }
  else if(blk_j == blk_l){
    in_dblk.insert("A");
    in_dblk.insert("U");
    out_dblk.insert("L");
    mn_temp_name = "lucpl";
  }
  else if(blk_i == blk_l){
    in_dblk.insert("A");
    in_dblk.insert("L");
    out_dblk.insert("U");
    mn_temp_name = "trs";
  }
  else{
    in_dblk.insert("L");
    in_dblk.insert("U");
    in_dblk.insert("A");
    out_dblk.insert("A");
    mn_temp_name = "submm";
  }

  prepareDblks();
  //mn
  MacroNode mn(mn_temp[mn_temp_name], mns.size());
  mns_perf.cycle += mn.mn_temp->cycle_length;
  mns_perf.num_compute_cycles += mn.mn_temp->cycle_length;
  mns_perf.total_use += mn.mn_temp->total_use;
  mns_perf.use_sram += mn.mn_temp->use_sram;
  mns_perf.use_pipe += mn.mn_temp->use_pipe;
  mns.push_back(mn);
}
