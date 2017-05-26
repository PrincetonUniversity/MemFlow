#include<iostream>
#include<fstream>
#include<climits>
#include<vector>
#include<array>

#include "OptiMacroNode.hpp"
#include "Setting.hpp"

using namespace std;

LoopOrder::LoopOrder(string loop1, string loop2, string loop3){
  loop_idc[0] = loop1;
  loop_idc[1] = loop2;
  loop_idc[2] = loop3;

  set = true;
}
/*
void LoopOrder::setupLoopOrder(OptiMacroNode* opti_mn){
  //set nospill matrix
  matrix_nospill = (loop_idc[2] == "m")?"B":(loop_idc[2] == "n")?"A":"C";
  if(matrix_nospill == "A"){
    if(loop_idc[0] == "m"){
      matrix_spilltype1 = "C";
      dblk1_size = opti_mn->c;
      num_dblk1_mem = opti_mn->num_cblk_mem;
      num_dblk1_need = opti_mn->blk_n;
      num_reuse1 = opti_mn->blk_k;
      num_iterate = opti_mn->blk_m;
      idx_mode1 = COL_IDX;

      matrix_spilltype2 = "B";
      dblk2_size = opti_mn->b;
      num_dblk2_mem = opti_mn->num_bblk_mem;
      num_dblk2_need = opti_mn->blk_k*opti_mn->blk_n;
      num_reuse2 = opti_mn->blk_m;
      idx_mode2 = ROWMAJOR_IDX;
    }
    else{
      matrix_spilltype1 = "B";
      dblk1_size = opti_mn->b;
      num_dblk1_mem = opti_mn->num_bblk_mem;
      num_dblk1_need = opti_mn->blk_n;
      num_reuse1 = opti_mn->blk_m;
      num_iterate = opti_mn->blk_k;
      idx_mode1 = COL_IDX;
      
      matrix_spilltype2 = "C";
      dblk2_size = opti_mn->c;
      num_dblk2_mem = opti_mn->num_cblk_mem;
      num_dblk2_need = opti_mn->blk_m*opti_mn->blk_n;
      num_reuse2 = opti_mn->blk_k;
      idx_mode2 = ROWMAJOR_IDX;
    }
  }
  else if(matrix_nospill == "B"){
    if(loop_idc[0] == "k"){
      matrix_spilltype1 = "A";
      dblk1_size = opti_mn->a;
      num_dblk1_mem = opti_mn->num_ablk_mem;
      num_dblk1_need = opti_mn->blk_m;
      num_reuse1 = opti_mn->blk_n;
      num_iterate = opti_mn->blk_k;
      idx_mode1 = ROW_IDX;

      matrix_spilltype2 = "C";
      dblk2_size = opti_mn->c;
      num_dblk2_mem = opti_mn->num_cblk_mem;
      num_dblk2_need = opti_mn->blk_m*opti_mn->blk_n;
      num_reuse2 = opti_mn->blk_k;
      idx_mode2 = COLMAJOR_IDX;
    }
    else{
      matrix_spilltype1 = "C";
      dblk1_size = opti_mn->c;
      num_dblk1_mem = opti_mn->num_cblk_mem;
      num_dblk1_need = opti_mn->blk_m;
      num_reuse1 = opti_mn->blk_k;
      num_iterate = opti_mn->blk_n;
      idx_mode1 = ROW_IDX;
      
      matrix_spilltype2 = "A";
      dblk2_size = opti_mn->a;
      num_dblk2_mem = opti_mn->num_ablk_mem;
      num_dblk2_need = opti_mn->blk_m*opti_mn->blk_k;
      num_reuse2 = opti_mn->blk_n;
      idx_mode2 = COLMAJOR_IDX;
    }
  }
  else{
    if(loop_idc[0] == "m"){
      matrix_spilltype1 = "A";
      dblk1_size = opti_mn->a;
      num_dblk1_mem = opti_mn->num_ablk_mem;
      num_dblk1_need = opti_mn->blk_k;
      num_reuse1 = opti_mn->blk_n;
      num_iterate = opti_mn->blk_m;
      idx_mode1 = COL_IDX;
      
      matrix_spilltype2 = "B";
      dblk2_size = opti_mn->b;
      num_dblk2_mem = opti_mn->num_bblk_mem;
      num_dblk2_need = opti_mn->blk_k*opti_mn->blk_n;
      num_reuse2 = opti_mn->blk_m;
      idx_mode2 = COLMAJOR_IDX;
    }
    else{
      matrix_spilltype1 = "B";
      dblk1_size = opti_mn->b;
      num_dblk1_mem = opti_mn->num_bblk_mem;
      num_dblk1_need = opti_mn->blk_k;
      num_reuse1 = opti_mn->blk_m;
      num_iterate = opti_mn->blk_n;
      idx_mode1 = ROW_IDX;
      
      matrix_spilltype2 = "A";
      dblk2_size = opti_mn->a;
      num_dblk2_mem = opti_mn->num_ablk_mem;
      num_dblk2_need = opti_mn->blk_k*opti_mn->blk_m;
      num_reuse2 = opti_mn->blk_n;
      idx_mode2 = ROWMAJOR_IDX;
    }
  }
}
*/


void LoopOrder::setDblkSPAddrIdx(){
  data_arrays[matrix_nospill]->setDblkSPAddrIdx(ZERO_IDX);
  data_arrays[matrix_spilltype1]->setDblkSPAddrIdx(idx_mode1);
  data_arrays[matrix_spilltype2]->setDblkSPAddrIdx(idx_mode2);
}

Parameters::Parameters(){
}

void Parameters::PrintInfo(){
  cout << endl << "******Parameters**********" << endl;
  cout << "Input size: ";
  for(auto &i: axes){
    cout << i.second.input_size << ",";
  }
  cout << endl;
  cout << "Extended input size: " << endl;
  for(auto &i: axes){
    cout << i.second.ex_input_size << ",";
  }
  cout << endl;
  cout << "Block dimension: ";
  for(auto &i: axes){
    cout << i.second.blk_dim << ",";
  }
  cout << endl; 
  cout << "Subblk dimension: ";
  for(auto &i: axes){
    cout << i.second.subblk_dim << ",";
  }
  cout << endl;
  cout << "Number of block: ";
  for(auto &i: axes){
    cout << i.second.num_blk << ",";
  }
  cout << endl;
  cout << "Number of subblk: ";
  for(auto &i: axes){
    cout << i.second.num_subblk << ",";
  }
  cout << endl;
  cout << "Number of subblk in block: ";
  for(auto &i: axes){
    cout << i.second.num_subblk_inblk << ",";
  }
  cout << endl;

  cout << "Data block: ";
  for(auto &i: dblks){
    cout << i.first << ",";
  }
  cout << endl;
  cout << "Block size: ";
  for(auto &i: dblks){
    cout << i.second.size << ",";
  }
  cout << endl;
  cout << "Number of ports allocated: ";
  for(auto &i: dblks){
    cout << i.second.num_port << ",";
  }
  cout << endl;
  cout << "Number of banks allocated: ";
  for(auto &i: dblks){
    cout << i.second.num_bank << ",";
  }
  cout << endl;
  cout << "Mem interval: ";
  for(auto &i: dblks){
    cout << i.second.interval << ",";
  }
  cout << endl;
  cout << "Number of blks in mem: ";
  for(auto &i: dblks){
    cout << i.second.num_blk_mem << ",";
  }
  cout << endl;
  
  cout << endl << "Compute block parameters: " << endl;
  for(auto &i: cbs){
    for(auto &j: i.second){
      cout << "CB " << i.first+"_"+j.first << ": parallelization " << j.second.num_cb << " stage " << j.second.k_stage << " #fus " << j.second.num_fus << " #regs " << j.second.num_regs << endl;
    }
  }

  cout << "Loop order: " << loop_order.loop_idc[0] << "->" << loop_order.loop_idc[1] << "->" << loop_order.loop_idc[2] << endl;

  for(auto &i: blk_cycles){
    cout << "Computation kernel " << i.first << " estimated #cycles " << i.second << endl;
  }

  cout << "Estimated #compute cycles: " << total_compute_cycles << endl;
  cout << "Estimated number of spills: " << num_spill << endl;
  cout << "Estimated number of spill1: " << num_spill1 << endl;
  cout << "Estimated number of spill2: " << num_spill2 << endl;
}

OptiMacroNode::OptiMacroNode(int in_m, int in_n, int in_k, Parameters& in_opti_para):opti_para(in_opti_para){
  para.axes["i"].input_size = in_m;
  para.axes["j"].input_size = in_n;
  para.axes["l"].input_size = in_k;
}


int MM_MinPort_new(int subblk_dimi, int subblk_dimj, int subblk_diml){
  int port_a = subblk_dimi*subblk_diml;
  int port_b = subblk_dimj*subblk_diml;
  int port_c = 1;
  return port_a+port_b+port_c;
}

void OptiMacroNode::MM_genSubblkSet_new(){
  num_port_used = 0;
  for(int subblk_dimi=1; subblk_dimi<=para.axes["i"].input_size; subblk_dimi++){
    cout << "subblk dimi " << subblk_dimi << endl;
    cout << "min port " << MM_MinPort_new(subblk_dimi, 1,1) << endl;
    if(MM_MinPort_new(subblk_dimi, 1, 1) > global_sp->total_port){
      break;
    }
    for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj++){
      cout << "subblk dimj " << subblk_dimj << endl;
      cout << "min port " << MM_MinPort_new(subblk_dimi, subblk_dimj,1) << endl;
      if(MM_MinPort_new(subblk_dimi, subblk_dimj, 1) > global_sp->total_port){
        break;
      }
      for(int subblk_diml=1; subblk_diml<=para.axes["l"].input_size; subblk_diml++){
        cout << "subblk diml " << subblk_diml << endl;
        cout << "min port " << MM_MinPort_new(subblk_dimi, subblk_dimj, subblk_diml) << endl;
        //too large
        if(MM_MinPort_new(subblk_dimi, subblk_dimj, subblk_diml) > global_sp->total_port){
          break;
        }
	//too small
	//int port_a = subblk_dimi*subblk_diml;
	//int port_b = subblk_dimj*subblk_diml;
	//int port_c = global_sp->total_port-port_a-port_b;
	//if(port_c > subblk_dimi*subblk_dimj){
	//  continue;
	//}
	sb_dim_set.push_back(array<int,3>{subblk_dimi, subblk_dimj, subblk_diml});
      }
    }
  }
}

void OptiMacroNode::MM_updatePara(){
//data path structure
  int num_cb = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;
  int k_stage = para.axes["l"].subblk_dim;

//fu+regs
  int cur_num_fus = num_cb*k_stage;
  int cur_num_regs = 4*cur_num_fus;

//pipeline degree
  int max_num_subblk_ingroup = k_stage*fus["add"];
  int num_cbatches = para.axes["i"].num_subblk_inblk*para.axes["j"].num_subblk_inblk;
  int num_group = (num_cbatches%max_num_subblk_ingroup==0)?num_cbatches/max_num_subblk_ingroup:num_cbatches/max_num_subblk_ingroup+1;

//latency of pipeline
  int pipeline_latency = fus["add"]*para.axes["l"].blk_dim*num_group;
  int batches_in_last_group = num_cbatches-max_num_subblk_ingroup*(num_group-1);
  int tail_latency = batches_in_last_group-1;

//latency of writing c
  int c_size = para.axes["i"].subblk_dim*para.axes["j"].subblk_dim;
  int cycles_write_c = (c_size%para.dblks["C"].num_port==0)?c_size/para.dblks["C"].num_port:c_size/para.dblks["C"].num_port+1;
  int write_penalty = num_cbatches*(cycles_write_c-1);

  int initial_latency = fus["mul"];
  int cur_num_cycles = initial_latency+pipeline_latency+tail_latency+write_penalty;

  para.cbs["mm"]["accmul"].num_fus = cur_num_fus;
  para.cbs["mm"]["accmul"].num_cb = num_cb;
  para.cbs["mm"]["accmul"].k_stage = k_stage;
  para.cbs["mm"]["accmul"].num_regs = cur_num_regs;

  int use_sram = 0;
  use_sram += para.axes["i"].blk_dim*para.axes["l"].blk_dim*para.axes["j"].num_subblk_inblk; //sram access a
  use_sram += para.axes["j"].blk_dim*para.axes["l"].blk_dim*para.axes["i"].num_subblk_inblk; //sram access b
  use_sram += para.axes["i"].blk_dim*para.axes["j"].blk_dim; //sram access c
  para.mntemps["mm"].use_sram = use_sram;

  para.mntemps["mm"].total_use = 4*para.axes["i"].blk_dim*para.axes["j"].blk_dim*para.axes["l"].blk_dim-para.axes["i"].blk_dim*para.axes["j"].blk_dim;
  para.mntemps["mm"].use_pipe = para.mntemps["mm"].total_use-para.mntemps["mm"].use_sram;
  para.mntemps["mm"].cycles = cur_num_cycles;

  para.blk_cycles["mm"] = cur_num_cycles;
  para.total_num_fus = para.cbs["mm"]["accmul"].num_fus;
}

void OptiMacroNode::MM_optiPara_kernel(){
  int num_cycles=INT_MAX;
  int num_fus=INT_MAX;

  fus["add"] = 10;
  fus["mul"] = 6;

  MM_genSubblkSet_new();

  vector<Parameters> paras;
  for(auto &i: sb_dim_set){
	  para.axes["i"].blk_dim = para.axes["i"].input_size;
	  para.axes["j"].blk_dim = para.axes["j"].input_size;
	  para.axes["l"].blk_dim = para.axes["l"].input_size;
	  para.axes["i"].subblk_dim = i[0];
	  para.axes["j"].subblk_dim = i[1];
	  para.axes["l"].subblk_dim = i[2];
	  
	  for(auto&a: para.axes){
	    a.second.num_subblk_inblk = (a.second.blk_dim%a.second.subblk_dim==0)?a.second.blk_dim/a.second.subblk_dim:a.second.blk_dim/a.second.subblk_dim+1;
	    a.second.blk_dim = a.second.num_subblk_inblk*a.second.subblk_dim;
	  }
	  
	  int port_per_bank = Memory::membanks[0].num_port;
	  para.dblks["A"].num_port = para.axes["l"].subblk_dim*para.axes["i"].subblk_dim;
	  para.dblks["B"].num_port = para.axes["l"].subblk_dim*para.axes["j"].subblk_dim;
	  para.dblks["C"].num_port = global_sp->total_port-para.dblks["A"].num_port-para.dblks["B"].num_port;

	  MM_updatePara();
	
	  if((para.blk_cycles["mm"] < num_cycles) 
	     || ((para.blk_cycles["mm"] == num_cycles) && (para.total_num_fus < num_fus))){
	    paras.clear();
	    paras.push_back(para);
	    num_cycles = para.blk_cycles["mm"];
	    num_fus = para.total_num_fus;
	  }
	  else if((para.blk_cycles["mm"]==num_cycles)&&(para.total_num_fus==num_fus)){
	    paras.push_back(para);
	  }
  }
  for(auto &p: paras){
    p.PrintInfo();
  }
}


void OptiMacroNode::LU_genSubblkSet_new(){
  for(int subblk_dimi=1; subblk_dimi<=para.axes["i"].input_size; subblk_dimi++){
    if(subblk_dimi+1+1 > global_sp->total_port){
      break;
    }
    for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj++){
      if(subblk_dimi+subblk_dimj+1 > global_sp->total_port){
        break;
      }
      sb_dim_set.push_back(array<int,3>{subblk_dimi,subblk_dimj,1});
    }
  }
}

void OptiMacroNode::LU_updatePara(){
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
	    //latency per layer(starting with writing l):max(cycles_read_l,fus["div"]+fus["mul"]+fus["sub"])
	    //last layer latency: fus["div"]
	    cur_num_cycles += max(cycles_read_l, fus["div"]+fus["mul"]+fus["sub"])*(layers_start_with_writel-1)+fus["div"];
	  }
	  int layers_start_with_readl = min(group_start,l_depth);
	  cur_num_cycles += max(fus["sub"],cycles_read_l)*layers_start_with_readl;
	  
	  if(group_start == 0){
	    //first layer first group need to generate l: can hide part of reading a
	    cur_num_cycles += max_num_subblk_ingroup*(cycles_read_a_row-1);
	    cur_num_cycles += max_num_subblk_ingroup*max(0,cycles_read_a_remain-(fus["div"]+fus["mul"]));
	  }
	  else{
	    //first layer in later groups don't need to generate l: expose all reading a latency
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
      //initialize active_sb
      int si = 0;
      while((active_sb.size()!=max_num_subblk_ingroup) && (si < num_cbatches)){
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

      //add latency becuase of l
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
	int i_end = min((si+1)*para.axes["i"].subblk_dim,para.axes["i"].blk_dim);
	int j_end = min((sj+1)*para.axes["j"].subblk_dim,para.axes["j"].blk_dim);
	use_sram += (i_end-i_start)*(j_end-j_start); //A access

	int depth = min(i_end,j_end);
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


void OptiMacroNode::LU_optiPara_kernel(){
  int num_cycles=INT_MAX;
  int num_fus=INT_MAX;
  
  fus["sub"] = 4;
  fus["mul"] = 2;
  fus["div"] = 8;

  LUCPL_genSubblkSet_new();
  
  vector<Parameters> paras;
  for(auto &i: sb_dim_set){
    para.axes["i"].subblk_dim = i[0];
    para.axes["j"].subblk_dim = i[1];
    para.axes["l"].subblk_dim = i[2];
    cout << endl << "subblk dim " << i[0] << ", " << i[1] << endl;

    for(auto &a: para.axes){
      a.second.blk_dim = a.second.input_size;
      a.second.num_subblk_inblk = (a.second.blk_dim%a.second.subblk_dim==0)?a.second.blk_dim/a.second.subblk_dim:a.second.blk_dim/a.second.subblk_dim+1;
      a.second.blk_dim = a.second.num_subblk_inblk*a.second.subblk_dim;
    }

    int port_per_bank = Memory::membanks[0].num_port;
    int num_port_u = para.axes["j"].subblk_dim;
      for(int num_port_l=1; num_port_l<(global_sp->total_port-1-num_port_u);num_port_l++){
	para.dblks["L"].num_port = num_port_l;
	para.dblks["U"].num_port = num_port_u;
	para.dblks["A"].num_port = global_sp->total_port-num_port_l-num_port_u;
	
	LU_updatePara();

	if((para.blk_cycles["lu"] < num_cycles) 
	   || ((para.blk_cycles["lu"]==num_cycles) && (para.total_num_fus < num_fus))){
	  paras.clear();
	  paras.push_back(para);
	  num_cycles = para.blk_cycles["lu"];
	  num_fus = para.total_num_fus;
	}
	else if((para.blk_cycles["lu"]==num_cycles)&&(para.total_num_fus==num_fus)){
	  paras.push_back(para);
	}
      }
  }
  for(auto &p: paras){
    p.PrintInfo();
  }
}

void OptiMacroNode::LUCPL_genSubblkSet_new(){
  for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj++){
    if(subblk_dimj+1+1 > global_sp->total_port){
      break;
    }
    for(int subblk_dimi=1; subblk_dimi<=para.axes["i"].input_size; subblk_dimi++){
      sb_dim_set.push_back(array<int,3>{subblk_dimi,subblk_dimj,1});
    }
  }
}


void OptiMacroNode::LUCPL_updatePara(){

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
      //per l iteration
      int group_span = max_num_subblk_ingroup*para.axes["j"].subblk_dim;
      for(int group_start=0; group_start<para.axes["j"].blk_dim; group_start+=group_span){
	//layers start with writing l
	int layers_start_with_writel = min(group_span,para.axes["j"].blk_dim-group_start);
	cur_num_cycles += max(cycles_read_l,fus["div"]+fus["mul"]+fus["sub"])*(layers_start_with_writel-1)+fus["div"];
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
      //initialize active_sb
      int si = 0;
      while((active_sb.size()!=max_num_subblk_ingroup) && (si < num_cbatches)){
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

	int depth = min(i_end,j_end);
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


void OptiMacroNode::LUCPL_optiPara_kernel(){
  int num_cycles=INT_MAX;
  int num_fus=INT_MAX;
  
  fus["sub"] = 4;
  fus["mul"] = 2;
  fus["div"] = 8;

  LUCPL_genSubblkSet_new();
  
  vector<Parameters> paras;
  for(auto &i: sb_dim_set){
    para.axes["i"].subblk_dim = i[0];
    para.axes["j"].subblk_dim = i[1];
    para.axes["l"].subblk_dim = i[2];
    cout << endl << "subblk dim " << i[0] << ", " << i[1] << endl;

    for(auto &a: para.axes){
      a.second.blk_dim = a.second.input_size;
      a.second.num_subblk_inblk = (a.second.blk_dim%a.second.subblk_dim==0)?a.second.blk_dim/a.second.subblk_dim:a.second.blk_dim/a.second.subblk_dim+1;
      a.second.blk_dim = a.second.num_subblk_inblk*a.second.subblk_dim;
    }

    int port_per_bank = Memory::membanks[0].num_port;
    int num_port_u = para.axes["j"].subblk_dim;
      for(int num_port_l=1; num_port_l<(global_sp->total_port-1-num_port_u);num_port_l++){
	para.dblks["L"].num_port = num_port_l;
	para.dblks["U"].num_port = num_port_u;
	para.dblks["A"].num_port = global_sp->total_port-num_port_l-num_port_u;
	
	LUCPL_updatePara();

	if((para.blk_cycles["lucpl"] < num_cycles) 
	   || ((para.blk_cycles["lucpl"]==num_cycles) && (para.total_num_fus < num_fus))){
	  paras.clear();
	  paras.push_back(para);
	  num_cycles = para.blk_cycles["lucpl"];
	  num_fus = para.total_num_fus;
	}
	else if((para.blk_cycles["lucpl"]==num_cycles)&&(para.total_num_fus==num_fus)){
	  paras.push_back(para);
	}
      }
  }
  for(auto &p: paras){
    p.PrintInfo();
  }
}


void OptiMacroNode::TRS_genSubblkSet_new(){
  for(int subblk_dimi=1; subblk_dimi<=para.axes["i"].input_size; subblk_dimi++){
    if(subblk_dimi+1+1 > global_sp->total_port){
      break;
    }
    for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj++){
      sb_dim_set.push_back(array<int,3>{subblk_dimi,subblk_dimj,1});
    }
  }
}

void OptiMacroNode::TRS_updatePara(){

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
	cur_num_cycles += max(cycles_read_u,fus["div"]+fus["mul"]+fus["sub"])*(layers_start_with_writeu-1)+fus["div"];
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
      while((active_sb.size()!=max_num_subblk_ingroup) && (si < num_cbatches)){
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

	int depth = min(i_end,j_end);
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

void OptiMacroNode::TRS_optiPara_kernel(){
  int num_cycles=INT_MAX;
  int num_fus=INT_MAX;
  
  fus["sub"] = 4;
  fus["mul"] = 2;
  fus["div"] = 8;

  TRS_genSubblkSet_new();
  
  vector<Parameters> paras;
  for(auto &i: sb_dim_set){
    para.axes["i"].subblk_dim = i[0];
    para.axes["j"].subblk_dim = i[1];
    para.axes["l"].subblk_dim = i[2];

    for(auto &a: para.axes){
      a.second.blk_dim = a.second.input_size;
      a.second.num_subblk_inblk = (a.second.blk_dim%a.second.subblk_dim==0)?a.second.blk_dim/a.second.subblk_dim:a.second.blk_dim/a.second.subblk_dim+1;
      a.second.blk_dim = a.second.num_subblk_inblk*a.second.subblk_dim;
    }

    int port_per_bank = Memory::membanks[0].num_port;
    //para.dblks["L"].num_port = para.axes["i"].subblk_dim;
    //para.dblks["U"].num_port = para.axes["j"].subblk_dim;
    //para.dblks["A"].num_port = global_sp->total_port-para.dblks["L"].num_port-para.dblks["U"].num_port;
    int num_port_l = para.axes["i"].subblk_dim;
      for(int num_port_u=1; num_port_u<(global_sp->total_port-1-num_port_l);num_port_u++){

	para.dblks["L"].num_port = num_port_l;
	para.dblks["U"].num_port = num_port_u;
	para.dblks["A"].num_port = global_sp->total_port-num_port_l-num_port_u;
        
	TRS_updatePara();

	if((para.blk_cycles["trs"] < num_cycles) 
	   || ((para.blk_cycles["trs"]==num_cycles) && (para.total_num_fus < num_fus))){
	  paras.clear();
	  paras.push_back(para);
	  num_cycles = para.blk_cycles["trs"];
	  num_fus = para.total_num_fus;
	}
	else if((para.blk_cycles["trs"]==num_cycles)&&(para.total_num_fus==num_fus)){
	  paras.push_back(para);
	}
      }
  }
  for(auto &p: paras){
    p.PrintInfo();
  }
}

void OptiMacroNode::SUBMM_updatePara(){
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

void OptiMacroNode::SUBMM_optiPara_kernel(){
  int num_cycles=INT_MAX;
  int num_fus=INT_MAX;
  
  fus["sub"] = 4;
  fus["mul"] = 2;

  MM_genSubblkSet_new();

  vector<Parameters> paras;
  for(auto &i: sb_dim_set){
	  para.axes["i"].blk_dim = para.axes["i"].input_size;
	  para.axes["j"].blk_dim = para.axes["j"].input_size;
	  para.axes["l"].blk_dim = para.axes["l"].input_size;
	  para.axes["i"].subblk_dim = i[0];
	  para.axes["j"].subblk_dim = i[1];
	  para.axes["l"].subblk_dim = i[2];
	  cout << endl << "subblk i,j,l " << i[0] <<"," << i[1] << "," << i[2] << endl;
	  
	  for(auto&a: para.axes){
	    a.second.num_subblk_inblk = (a.second.blk_dim%a.second.subblk_dim==0)?a.second.blk_dim/a.second.subblk_dim:a.second.blk_dim/a.second.subblk_dim+1;
	    a.second.blk_dim = a.second.num_subblk_inblk*a.second.subblk_dim;
	  }
	  
	  int port_per_bank = Memory::membanks[0].num_port;
	  para.dblks["L"].num_port = para.axes["l"].subblk_dim*para.axes["i"].subblk_dim;
	  para.dblks["U"].num_port = para.axes["l"].subblk_dim*para.axes["j"].subblk_dim;
	  para.dblks["A"].num_port = global_sp->total_port-para.dblks["L"].num_port-para.dblks["U"].num_port;

	  SUBMM_updatePara();

	  if((para.blk_cycles["submm"] < num_cycles) 
	     || ((para.blk_cycles["submm"]==num_cycles) && (para.total_num_fus < num_fus))){
	    paras.clear();
	    paras.push_back(para);
	    num_cycles = para.blk_cycles["submm"];
	    num_fus = para.total_num_fus;
	  }
	  else if((para.blk_cycles["submm"]==num_cycles)&&(para.total_num_fus==num_fus)){
	    paras.push_back(para);
	  }
  }
  for(auto &p: paras){
    p.PrintInfo();
  }
}

void OptiMacroNode::QR_genSubblkSet_new(){
  int port_used = 0;
  for(int subblk_diml=1; subblk_diml<=para.axes["l"].input_size; subblk_diml++){
    if(subblk_diml+1 > global_sp->total_port){
      break;
    }
    for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj++){
      int cur_port_used = subblk_diml+subblk_dimj+subblk_diml*subblk_dimj;
      if(cur_port_used > global_sp->total_port){
        break;
      }
      else{
	  sb_dim_set.push_back(array<int,3>{1,subblk_dimj,subblk_diml});
      }
    }
  }
}

void OptiMacroNode::QR_updatePara(){
   int cur_num_cycles = 0;
   int n = para.axes["i"].blk_dim;

   for(int col=0; col<n-1; col++){
     int cycles_norm2 = fus["mul"]+fus["add"]*(n-col);
     cur_num_cycles += cycles_norm2;

     int cycles_hhparas = fus["sqrt"]+fus["add"]+max(fus["div"],fus["mul"])+fus["sqrt"];
     cur_num_cycles += cycles_hhparas;

     int num_w_batches_div = ((n-col-1)%para.axes["l"].subblk_dim==0)?(n-col-1)/para.axes["l"].subblk_dim:(n-col-1)/para.axes["l"].subblk_dim+1;
     int cycles_get_w = fus["div"]+(num_w_batches_div-1);

     int max_num_batches_ingroup = fus["add"];
     int num_batches = ((n-col-1)%para.axes["j"].subblk_dim==0)?(n-col)/para.axes["j"].subblk_dim:(n-col)/para.axes["j"].subblk_dim+1;
     int num_groups = (num_batches%max_num_batches_ingroup==0)?num_batches/max_num_batches_ingroup:num_batches/max_num_batches_ingroup+1;

     //time for each group
     cur_num_cycles += fus["mul"]+num_groups*(n-col)*fus["add"];

     //time for generate new a
     int num_w_batches = ((n-col)%para.axes["l"].subblk_dim==0)?(n-col)/para.axes["l"].subblk_dim:(n-col)/para.axes["l"].subblk_dim+1;
     int latency_gen_a_batch = fus["mul"]+fus["sub"]+(num_w_batches-1);

     cur_num_cycles += latency_gen_a_batch*num_batches;
   }

   int use_sram = 0;
   for(int c=0; c<n; c++){
     use_sram += (n-c); //read A col->norm2
     use_sram += (n-c-1); //read A col->div for w
     use_sram += (n-c); //write w col
     use_sram += (n-c)*(n-c-1); //read a tailing matrix
     int batches = ((n-c-1)%para.axes["j"].subblk_dim==0)?(n-c-1)/para.axes["j"].subblk_dim:(n-c-1)/para.axes["j"].subblk_dim+1;
     use_sram += (n-c)*batches; //read w
     for(int b=0; b<batches; b++){
       use_sram += (n-c); //read w
       int batch_width = min(n-b*para.axes["j"].subblk_dim,para.axes["j"].subblk_dim);
       use_sram += 2*(n-c)*batch_width; //read a
       use_sram += batch_width; //write r
     }
   }
   para.mntemps["qr"].use_sram = use_sram;

   para.blk_cycles["qr"] = cur_num_cycles;
   para.total_num_fus = 0;
   para.cbs["qr"]["accmul"].num_cb = para.axes["j"].subblk_dim;
   para.cbs["qr"]["accmul"].k_stage = 1;
   para.cbs["qr"]["accmul"].num_fus = para.axes["j"].subblk_dim;
   para.cbs["qr"]["accmul"].num_regs = 4*para.cbs["qr"]["accmul"].num_fus;
   para.total_num_fus += para.cbs["qr"]["accmul"].num_fus;

   para.cbs["qr"]["div"].num_cb = para.axes["l"].subblk_dim;
   para.cbs["qr"]["div"].num_fus = para.cbs["qr"]["div"].num_cb;
   para.cbs["qr"]["div"].num_regs = 2*para.cbs["qr"]["div"].num_fus;
   para.total_num_fus += para.cbs["qr"]["div"].num_fus;

   para.cbs["qr"]["submul"].num_cb = para.axes["j"].subblk_dim*para.axes["l"].subblk_dim;
   para.cbs["qr"]["submul"].k_stage = 1;
   para.cbs["qr"]["submul"].num_fus = para.cbs["qr"]["submul"].num_cb*para.cbs["qr"]["submul"].k_stage;
   para.cbs["qr"]["submul"].num_regs = 3*para.cbs["qr"]["submul"].num_fus;
   para.total_num_fus += para.cbs["qr"]["submul"].num_fus;

   para.cbs["qr"]["hhpara"].num_cb = 1;
   para.cbs["qr"]["hhpara"].num_fus = 6;
   para.cbs["qr"]["hhpara"].num_regs = 6;
   para.total_num_fus += para.cbs["qr"]["hhpara"].num_fus;
}

void OptiMacroNode::QR_optiPara_kernel(){
  int num_cycles=INT_MAX;
  int num_fus=INT_MAX;
  
  QR_genSubblkSet_new();
  
  fus["mul"] = 2;
  fus["add"] = 4;
  fus["sub"] = 4;
  fus["div"] = 8;
  fus["sqrt"] = 7;

  vector<Parameters> paras;
  for(auto &i: sb_dim_set){
     para.axes["i"].subblk_dim = i[0];
     para.axes["j"].subblk_dim = i[1];
     para.axes["l"].subblk_dim = i[2];
     para.axes["i"].blk_dim = para.axes["i"].input_size;
     para.axes["j"].blk_dim = para.axes["j"].input_size;
     para.axes["l"].blk_dim = para.axes["l"].input_size;
     cout << endl << "subblk dim: " << "j " << i[1] << ",l " << i[2] << endl;

     for(auto &a: para.axes){
       a.second.blk_dim = a.second.input_size;
       a.second.num_subblk_inblk = (a.second.blk_dim%a.second.subblk_dim==0)?a.second.blk_dim/a.second.subblk_dim:a.second.blk_dim/a.second.subblk_dim+1;
     }

     int port_per_bank = Memory::membanks[0].num_port;
     para.dblks["Q"].num_port = para.axes["l"].subblk_dim;
     para.dblks["R"].num_port = para.axes["j"].subblk_dim;
     para.dblks["A"].num_port = para.axes["j"].subblk_dim*para.axes["l"].subblk_dim;

     QR_updatePara();

     if((para.blk_cycles["qr"] < num_cycles) 
       || ((para.blk_cycles["qr"]==num_cycles) && (para.total_num_fus < num_fus))){
       paras.clear();
       paras.push_back(para);
       num_cycles = para.blk_cycles["qr"];
       num_fus = para.total_num_fus;
     }
     else if((para.blk_cycles["qr"]==num_cycles)&&(para.total_num_fus==num_fus)){
       paras.push_back(para);
     }
  }
  for(auto &p: paras){
    p.PrintInfo();
  }
}

void OptiMacroNode::QRCPL_updatePara(){
   int cur_num_cycles = 0;
   int n = para.axes["i"].blk_dim;
   for(int col=0; col<n-1; col++){
     int cycles_norm2 = fus["mul"]+fus["add"]*(n+1);
     cur_num_cycles += cycles_norm2;

     int cycles_hhparas = fus["sqrt"]+fus["add"]+max(fus["mul"],fus["div"])+fus["sqrt"];
     cur_num_cycles += cycles_hhparas;

     int num_w_batches_div = (n%para.axes["l"].subblk_dim==0)?n/para.axes["l"].subblk_dim:n/para.axes["l"].subblk_dim+1;
     int cycles_get_w = fus["div"]+(num_w_batches_div-1);

     int max_num_batches_ingroup = fus["add"];
     int num_batches = ((n-col)%para.axes["j"].subblk_dim==0)?(n-col)/para.axes["j"].subblk_dim:(n-col)/para.axes["j"].subblk_dim+1;
     int num_groups = (num_batches%max_num_batches_ingroup==0)?num_batches/max_num_batches_ingroup:num_batches/max_num_batches_ingroup+1;

     //time for each group
     cur_num_cycles += fus["mul"]+num_groups*(n+1)*fus["add"];

     //time for generate new a
     int num_w_batches = (n%para.axes["l"].subblk_dim==0)?n/para.axes["l"].subblk_dim:n/para.axes["l"].subblk_dim+1;
     int latency_gen_a_batch = fus["mul"]+fus["sub"]+(num_w_batches-1)+1; //need one cycle for r update

     cur_num_cycles += latency_gen_a_batch*num_batches;
   }

   int use_sram = 0;
   for(int c=0; c<n; c++){
     use_sram += (n+1); //read A col->norm2
     use_sram += n; //read A col->div
     use_sram += (n+1); //write w col
     use_sram += (n-c-1); //read r row
     use_sram += n*(n-c-1); //read a tailing matrix
     int batches = ((n-c-1)%para.axes["j"].subblk_dim==0)?(n-c-1)/para.axes["j"].subblk_dim:(n-c-1)/para.axes["j"].subblk_dim+1;
     use_sram += (n+1)*batches; //read w col
     for(int b=0; b<batches; b++){
       use_sram += (n+1); //read w
       int batch_width = min(n-b*para.axes["j"].subblk_dim,para.axes["j"].subblk_dim);
       use_sram += 2*batch_width; //read write r
       use_sram += 2*n*batch_width;
     }
   }
   para.mntemps["qrcpl"].use_sram = use_sram;

   para.blk_cycles["qrcpl"] = cur_num_cycles;

   para.total_num_fus = 0;
   para.cbs["qrcpl"]["accmul"].num_cb = para.axes["j"].subblk_dim;
   para.cbs["qrcpl"]["accmul"].k_stage = 1;
   para.cbs["qrcpl"]["accmul"].num_fus = para.axes["j"].subblk_dim;
   para.cbs["qrcpl"]["accmul"].num_regs = 4*para.cbs["qrcpl"]["accmul"].num_fus;
   para.total_num_fus += para.cbs["qrcpl"]["accmul"].num_fus;

   para.cbs["qrcpl"]["div"].num_cb = para.axes["l"].subblk_dim;
   para.cbs["qrcpl"]["div"].num_fus = para.cbs["qrcpl"]["div"].num_cb;
   para.cbs["qrcpl"]["div"].num_regs = 2*para.cbs["qrcpl"]["div"].num_fus;
   para.total_num_fus += para.cbs["qrcpl"]["div"].num_fus;

   para.cbs["qrcpl"]["submul"].num_cb = para.axes["j"].subblk_dim*para.axes["l"].subblk_dim;
   para.cbs["qrcpl"]["submul"].k_stage = 1;
   para.cbs["qrcpl"]["submul"].num_fus = para.cbs["qrcpl"]["submul"].num_cb*para.cbs["qrcpl"]["submul"].k_stage;
   para.cbs["qrcpl"]["submul"].num_regs = 3*para.cbs["qrcpl"]["submul"].num_fus;
   para.total_num_fus += para.cbs["qrcpl"]["submul"].num_fus;

   para.cbs["qrcpl"]["hhpara"].num_cb = 1;
   para.cbs["qrcpl"]["hhpara"].num_fus = 6;
   para.cbs["qrcpl"]["hhpara"].num_regs = 6;
   para.total_num_fus += para.cbs["qrcpl"]["hhpara"].num_fus;
}

void OptiMacroNode::QRCPL_optiPara_kernel(){
  int num_cycles=INT_MAX;
  int num_fus=INT_MAX;
  
  QR_genSubblkSet_new();
  
  fus["mul"] = 2;
  fus["add"] = 4;
  fus["sub"] = 4;
  fus["div"] = 8;
  fus["sqrt"] = 7;

  vector<Parameters> paras;
  for(auto &i: sb_dim_set){
     para.axes["i"].subblk_dim = i[0];
     para.axes["j"].subblk_dim = i[1];
     para.axes["l"].subblk_dim = i[2];
     para.axes["i"].blk_dim = para.axes["i"].input_size;
     para.axes["j"].blk_dim = para.axes["j"].input_size;
     para.axes["l"].blk_dim = para.axes["l"].input_size;
     cout << endl << "subblk dim: " << "j " << i[1] << ",l " << i[2] << endl;

     for(auto &a: para.axes){
       a.second.blk_dim = a.second.input_size;
       a.second.num_subblk_inblk = (a.second.blk_dim%a.second.subblk_dim==0)?a.second.blk_dim/a.second.subblk_dim:a.second.blk_dim/a.second.subblk_dim+1;
     }

     int port_per_bank = Memory::membanks[0].num_port;
     para.dblks["Q"].num_port = para.axes["l"].subblk_dim;
     para.dblks["R"].num_port = para.axes["j"].subblk_dim;
     para.dblks["A"].num_port = para.axes["j"].subblk_dim*para.axes["l"].subblk_dim;

     QRCPL_updatePara();

     if((para.blk_cycles["qrcpl"] < num_cycles) 
       || ((para.blk_cycles["qrcpl"]==num_cycles) && (para.total_num_fus<num_fus))){
       paras.clear();
       paras.push_back(para);
       num_cycles = para.blk_cycles["qrcpl"];
       num_fus = para.total_num_fus;
     }
     else if((para.blk_cycles["qrcpl"]==num_cycles)&&(para.total_num_fus==num_fus)){
       paras.push_back(para);
     }
  }
  for(auto &p: paras){
    p.PrintInfo();
  }
}

void OptiMacroNode::QRUPDATETR_updatePara(){
   int cur_num_cycles = 0;
   int n = para.axes["i"].blk_dim;
   for(int col=0; col<n-1; col++){
     int max_num_batches_ingroup = fus["add"];
     int num_batches = (n%para.axes["j"].subblk_dim==0)?n/para.axes["j"].subblk_dim:n/para.axes["j"].subblk_dim+1;
     int num_groups = (num_batches%max_num_batches_ingroup==0)?num_batches/max_num_batches_ingroup:num_batches/max_num_batches_ingroup+1;

     //time for each group
     cur_num_cycles += fus["mul"]+num_groups*(n-col)*fus["add"];

     //time for generate new a
     int num_w_batches = ((n-col)%para.axes["l"].subblk_dim==0)?(n-col)/para.axes["l"].subblk_dim:(n-col)/para.axes["l"].subblk_dim+1;
     int latency_gen_a_batch = fus["mul"]+fus["sub"]+(num_w_batches-1);
     latency_gen_a_batch += 1;
     cur_num_cycles += latency_gen_a_batch*num_batches;
   }

   int use_sram = 0;
   for(int c=0; c<n; c++){
     use_sram += (n-c)*(n-c-1); //read a tailing matrix
     int batches = ((n-c-1)%para.axes["j"].subblk_dim==0)?(n-c-1)/para.axes["j"].subblk_dim:(n-c-1)/para.axes["j"].subblk_dim+1;
     use_sram += (n-c)*batches;
     for(int b=0; b<batches; b++){
       use_sram += (n-c); //read w
       int batch_width = min(n-b*para.axes["j"].subblk_dim,para.axes["j"].subblk_dim);
       use_sram += 2*(n-c)*batch_width;
       use_sram += batch_width; //write r
     }
   }
   para.mntemps["qrupdatetr"].use_sram = use_sram;

   para.blk_cycles["qrupdatetr"] = cur_num_cycles;
   para.total_num_fus = 0;
   para.cbs["qrupdatetr"]["accmul"].num_cb = para.axes["j"].subblk_dim;
   para.cbs["qrupdatetr"]["accmul"].k_stage = 1;
   para.cbs["qrupdatetr"]["accmul"].num_fus = para.axes["j"].subblk_dim;
   para.cbs["qrupdatetr"]["accmul"].num_regs = 4*para.cbs["qrupdatetr"]["accmul"].num_fus;
   para.total_num_fus += para.cbs["qrupdatetr"]["accmul"].num_fus;

   para.cbs["qrupdatetr"]["submul"].num_cb = para.axes["j"].subblk_dim*para.axes["l"].subblk_dim;
   para.cbs["qrupdatetr"]["submul"].k_stage = 1;
   para.cbs["qrupdatetr"]["submul"].num_fus = para.cbs["qrupdatetr"]["submul"].num_cb*para.cbs["qrupdatetr"]["submul"].k_stage;
   para.cbs["qrupdatetr"]["submul"].num_regs = 3*para.cbs["qrupdatetr"]["submul"].num_fus;
   para.total_num_fus += para.cbs["qrupdatetr"]["submul"].num_fus;
}

void OptiMacroNode::QRUPDATETR_optiPara_kernel(){
  int num_cycles=INT_MAX;
  int num_fus=INT_MAX;
  
  QR_genSubblkSet_new();
  
  fus["mul"] = 2;
  fus["add"] = 4;
  fus["sub"] = 4;
  fus["div"] = 8;
  fus["sqrt"] = 7;

  vector<Parameters> paras;
  for(auto &i: sb_dim_set){
     para.axes["i"].subblk_dim = i[0];
     para.axes["j"].subblk_dim = i[1];
     para.axes["l"].subblk_dim = i[2];
     para.axes["i"].blk_dim = para.axes["i"].input_size;
     para.axes["j"].blk_dim = para.axes["j"].input_size;
     para.axes["l"].blk_dim = para.axes["l"].input_size;
     cout << endl << "subblk dim: " << "j " << i[1] << ",l " << i[2] << endl;

     for(auto &a: para.axes){
       a.second.blk_dim = a.second.input_size;
       a.second.num_subblk_inblk = (a.second.blk_dim%a.second.subblk_dim==0)?a.second.blk_dim/a.second.subblk_dim:a.second.blk_dim/a.second.subblk_dim+1;
     }

     int port_per_bank = Memory::membanks[0].num_port;
     para.dblks["Q"].num_port = para.axes["l"].subblk_dim;
     para.dblks["R"].num_port = para.axes["j"].subblk_dim;
     para.dblks["A"].num_port = para.axes["j"].subblk_dim*para.axes["l"].subblk_dim;

     QRUPDATETR_updatePara();

     if((para.blk_cycles["qrupdatetr"] < num_cycles) 
       || ((para.blk_cycles["qrupdatetr"]==num_cycles) && (para.total_num_fus < num_fus))){
       paras.clear();
       paras.push_back(para);
       num_cycles = para.blk_cycles["qrupdatetr"];
       num_fus = para.total_num_fus;
     }
     else if((para.blk_cycles["qrupdatetr"]==num_cycles)&&(para.total_num_fus==num_fus)){
       paras.push_back(para);
     }
  }
  for(auto &p: paras){
    p.PrintInfo();
  }
}

void OptiMacroNode::QRUPDATE_updatePara(){
   int cur_num_cycles = 0;
   int n = para.axes["i"].blk_dim;
   for(int col=0; col<n-1; col++){
     int max_num_batches_ingroup = fus["add"];
     int num_batches = (n%para.axes["j"].subblk_dim==0)?n/para.axes["j"].subblk_dim:n/para.axes["j"].subblk_dim+1;
     int num_groups = (num_batches%max_num_batches_ingroup==0)?num_batches/max_num_batches_ingroup:num_batches/max_num_batches_ingroup+1;

     //time for each group
     cur_num_cycles += fus["mul"]+num_groups*(n+1)*fus["add"];

     //time for generate new a
     int num_w_batches = (n%para.axes["l"].subblk_dim==0)?n/para.axes["l"].subblk_dim:n/para.axes["l"].subblk_dim+1;
     int latency_gen_a_batch = fus["mul"]+fus["sub"]+(num_w_batches-1);
     latency_gen_a_batch += 1;
     cur_num_cycles += latency_gen_a_batch*num_batches;
   }

   int use_sram = 0;
   for(int c=0; c<n; c++){
     use_sram += (n-c-1); //read r row
     use_sram += n*(n-c-1); //read a tailing matrix
     int batches = ((n-c-1)%para.axes["j"].subblk_dim==0)?(n-c-1)/para.axes["j"].subblk_dim:(n-c-1)/para.axes["j"].subblk_dim+1;
     use_sram += (n+1)*batches; //read w col
     for(int b=0; b<batches; b++){
       use_sram += (n+1); //read w
       int batch_width = min(n-b*para.axes["j"].subblk_dim,para.axes["j"].subblk_dim);
       use_sram += 2*batch_width; //read write r
       use_sram += 2*n*batch_width;
     }
   }
   para.mntemps["qrupdate"].use_sram = use_sram;

   para.blk_cycles["qrupdate"] = cur_num_cycles;
   para.total_num_fus = 0;
   para.cbs["qrupdate"]["accmul"].num_cb = para.axes["j"].subblk_dim;
   para.cbs["qrupdate"]["accmul"].k_stage = 1;
   para.cbs["qrupdate"]["accmul"].num_fus = para.axes["j"].subblk_dim;
   para.cbs["qrupdate"]["accmul"].num_regs = 4*para.cbs["qrupdate"]["accmul"].num_fus;
   para.total_num_fus += para.cbs["qrupdate"]["accmul"].num_fus;

   para.cbs["qrupdate"]["submul"].num_cb = para.axes["j"].subblk_dim*para.axes["l"].subblk_dim;
   para.cbs["qrupdate"]["submul"].k_stage = 1;
   para.cbs["qrupdate"]["submul"].num_fus = para.cbs["qrupdate"]["submul"].num_cb*para.cbs["qrupdate"]["submul"].k_stage;
   para.cbs["qrupdate"]["submul"].num_regs = 3*para.cbs["qrupdate"]["submul"].num_fus;
   para.total_num_fus += para.cbs["qrupdate"]["submul"].num_fus;
}

void OptiMacroNode::QRUPDATE_optiPara_kernel(){
  int num_cycles=INT_MAX;
  int num_fus=INT_MAX;
  
  QR_genSubblkSet_new();
  
  
  fus["mul"] = 2;
  fus["add"] = 4;
  fus["sub"] = 4;
  fus["div"] = 8;
  fus["sqrt"] = 7;

  vector<Parameters> paras;
  for(auto &i: sb_dim_set){
     para.axes["i"].subblk_dim = i[0];
     para.axes["j"].subblk_dim = i[1];
     para.axes["l"].subblk_dim = i[2];
     para.axes["i"].blk_dim = para.axes["i"].input_size;
     para.axes["j"].blk_dim = para.axes["j"].input_size;
     para.axes["l"].blk_dim = para.axes["l"].input_size;

     for(auto &a: para.axes){
       a.second.blk_dim = a.second.input_size;
       a.second.num_subblk_inblk = (a.second.blk_dim%a.second.subblk_dim==0)?a.second.blk_dim/a.second.subblk_dim:a.second.blk_dim/a.second.subblk_dim+1;
     }

     int port_per_bank = Memory::membanks[0].num_port;
     para.dblks["Q"].num_port = para.axes["l"].subblk_dim;
     para.dblks["R"].num_port = para.axes["j"].subblk_dim;
     para.dblks["A"].num_port = para.axes["j"].subblk_dim*para.axes["l"].subblk_dim;

     QRUPDATE_updatePara();

     if((para.blk_cycles["qrupdate"] < num_cycles) 
       || ((para.blk_cycles["qrupdate"]==num_cycles) && (para.total_num_fus < num_fus))){
       paras.clear();
       paras.push_back(para);
       num_cycles = para.blk_cycles["qrupdate"];
       num_fus = para.total_num_fus;
     }
     else if((para.blk_cycles["qrupdate"]==num_cycles)&&(para.total_num_fus==num_fus)){
       paras.push_back(para);
     }
  }
  for(auto &p: paras){
    p.PrintInfo();
  }
}



//************************************************************************

int OptiMacroNode::MM_MinMem(int blk_dimi, int blk_dimj, int blk_diml){
    return (blk_dimi*blk_diml+blk_dimj*blk_diml+blk_dimi*blk_dimj);
}

int OptiMacroNode::MM_MinBank(int m_subblk, int n_subblk, int k_subblk){
  int port_per_bank = Memory::membanks[0].num_port;

  int num_port_a = k_subblk*m_subblk;
  int num_bank_a = (num_port_a%port_per_bank==0)?num_port_a/port_per_bank:num_port_a/port_per_bank+1;

  int num_port_b = k_subblk*n_subblk;
  int num_bank_b = (num_port_b%port_per_bank==0)?num_port_b/port_per_bank:num_port_b/port_per_bank+1;

  int num_port_c = 2*m_subblk*n_subblk;
  int num_bank_c = (num_port_c%port_per_bank==0)?num_port_c/port_per_bank:num_port_c/port_per_bank+1;

  return (num_bank_a+num_bank_b+num_bank_c);
}

int OptiMacroNode::MM_MinPort(int sb_dimi, int sb_dimj, int sb_diml){
  return (sb_diml*sb_dimi + sb_diml*sb_dimj + 2*sb_dimi*sb_dimj);
}

void OptiMacroNode::MM_genSubblkSet(){
  num_port_used = 0;
  int step = 20;
  for(int subblk_dimi=1; subblk_dimi<=para.axes["i"].input_size; subblk_dimi+=step){
    if(MM_MinPort_new(subblk_dimi, 1, 1) > global_sp->total_port){
      break;
    }
    for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj+=step){
      if(MM_MinPort_new(subblk_dimi, subblk_dimj, 1) > global_sp->total_port){
        break;
      }
      for(int subblk_diml=1; subblk_diml<=para.axes["l"].input_size; subblk_diml+=step){
        //too large
        if(MM_MinPort_new(subblk_dimi, subblk_dimj, subblk_diml) > global_sp->total_port){
          break;
        }
	//too small
	if(subblk_dimi*subblk_diml+subblk_dimj*subblk_diml+subblk_dimi*subblk_dimj < 0.9*global_sp->total_port)
	{
	  continue;
	}
	sb_dim_set.push_back(array<int,3>{subblk_dimi, subblk_dimj, subblk_diml});
      }
    }
  }
}

//reused blocks on line for full matrix
unsigned long long spill_normal(blk_info& spill_dblk, int num_exist, int num_reuse, int iterate){
  unsigned long long spill;
  if(num_exist <= spill_dblk.num_blk_mem){
    spill = 0;
  }
  else{
    //spill = num_exist-spill_dblk.num_blk_mem+1;
    //spill = spill*(num_reuse-1);
    //if(spill > 0){
    //  spill -= 1;
    //}
    //spill = spill*iterate;
    //spill = spill*spill_dblk.size;
    int length = num_exist*(num_reuse-1);
    int period = num_exist-1;
    int num_spill_period = num_exist-spill_dblk.num_blk_mem;
    spill = length/period;
    spill = spill*num_spill_period;
    //last period
    if(length%period!=0){
      spill += max(0,(length%period)-(spill_dblk.num_blk_mem-1));
    }
    spill = spill*iterate;
    spill = spill*spill_dblk.size;
  }
  return spill;
}

unsigned long long OptiMacroNode::MM_spill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2){
  //cout << "direction " << lo.loop_idc[0] << " " << lo.loop_idc[1] << " " << lo.loop_idc[2] << endl;
  int blk_m = para.axes["i"].num_blk;
  int blk_n = para.axes["j"].num_blk;
  int blk_k = para.axes["l"].num_blk;

  string matrix_nospill = (lo.loop_idc[2] == "i")?"B":(lo.loop_idc[2] == "j")?"A":"C";
  if(matrix_nospill == "A"){
    if(lo.loop_idc[0] == "i"){
      lo.matrix_spilltype1 = "C";
      //spill1 = 2*spill_normal(para.dblks["C"],blk_n,blk_k,blk_m);
      spill1 = spill_normal(para.dblks["C"],blk_n,blk_k,blk_m);
      //cout << "spill1 " << "C " << " num_exist " << blk_n << " num_reuse " << blk_k << " iterate " << blk_m << endl;
      lo.idx_mode1 = COL_IDX;

      lo.matrix_spilltype2 = "B";
      spill2 = spill_normal(para.dblks["B"],blk_k*blk_n,blk_m,1);
      //cout << "spill2 " << "B " << " num_exist " << blk_k*blk_n << " num_reuse " << blk_m << " iterate " << 1 << endl;
      lo.idx_mode2 = ROWMAJOR_IDX;
    }
    else{
      lo.matrix_spilltype1 = "B";
      spill1 = spill_normal(para.dblks["B"],blk_n,blk_m,blk_k);
      //cout << "spill1 " << "B " << " num_exist " << blk_n << " num_reuse " << blk_m << " iterate " << blk_k << endl;
      lo.idx_mode1 = COL_IDX;
      
      lo.matrix_spilltype2 = "C";
      spill2 = spill_normal(para.dblks["C"],blk_m*blk_n,blk_k,1);
      //spill2 = 2*spill_normal(para.dblks["C"],blk_m*blk_n,blk_k,1);
      //cout << "spill2 " << "C " << " num_exist " << blk_m*blk_n << " num_reuse " << blk_k << " iterate " << 1 << endl;
      lo.idx_mode2 = ROWMAJOR_IDX;
    }
  }
  else if(matrix_nospill == "B"){
    if(lo.loop_idc[0] == "l"){
      lo.matrix_spilltype1 = "A";
      spill1 = spill_normal(para.dblks["A"],blk_m,blk_n,blk_k);
      //cout << "spill1 " << "A " << " num_exist " << blk_n << " num_reuse " << blk_n << " iterate " << blk_k << endl;
      lo.idx_mode1 = ROW_IDX;

      lo.matrix_spilltype2 = "C";
      //spill2 = 2*spill_normal(para.dblks["C"],blk_m*blk_n,blk_k,1);
      spill2 = spill_normal(para.dblks["C"],blk_m*blk_n,blk_k,1);
      //cout << "spill2 " << "C " << " num_exist " << blk_m*blk_n << " num_reuse " << blk_k << " iterate " << 1 << endl;
      lo.idx_mode2 = COLMAJOR_IDX;
    }
    else{
      lo.matrix_spilltype1 = "C";
      //spill1 = 2*spill_normal(para.dblks["C"],blk_m,blk_k,blk_n);
      spill1 = spill_normal(para.dblks["C"],blk_m,blk_k,blk_n);
      //cout << "spill1 " << "C " << " num_exist " << blk_m << " num_reuse " << blk_k << " iterate " << blk_n << endl;
      lo.idx_mode1 = ROW_IDX;
      
      lo.matrix_spilltype2 = "A";
      spill2 = spill_normal(para.dblks["A"],blk_m*blk_k,blk_n,1);
      //cout << "spill2 " << "A " << " num_exist " << blk_m*blk_k << " num_reuse " << blk_n << " iterate " << 1 << endl;
      lo.idx_mode2 = COLMAJOR_IDX;
    }
  }
  else{
    if(lo.loop_idc[0] == "i"){
      lo.matrix_spilltype1 = "A";
      spill1 = spill_normal(para.dblks["A"],blk_k,blk_n,blk_m);
      //cout << "spill1 " << "A " << " num_exist " << blk_k << " num_reuse " << blk_n << " iterate " << blk_m << endl;
      lo.idx_mode1 = COL_IDX;
      
      lo.matrix_spilltype2 = "B";
      spill2 = spill_normal(para.dblks["B"],blk_k*blk_n,blk_m,1);
      //cout << "spill2 " << "B " << " num_exist " << blk_k*blk_n << " num_reuse " << blk_m << " iterate " << 1 << endl;
      lo.idx_mode2 = COLMAJOR_IDX;
    }
    else{
      lo.matrix_spilltype1 = "B";
      spill1 = spill_normal(para.dblks["B"],blk_k,blk_m,blk_n);
      //cout << "spill1 " << "B " << " num_exist " << blk_k << " num_reuse " << blk_m << " iterate " << blk_n << endl;
      lo.idx_mode1 = ROW_IDX;
      
      lo.matrix_spilltype2 = "A";
      spill2 = spill_normal(para.dblks["A"],blk_k*blk_m,blk_n,1);
      //cout << "spill2 " << "A " << " num_exist " << blk_k*blk_m << " num_reuse " << blk_n << " iterate " << 1 << endl;
      lo.idx_mode2 = ROWMAJOR_IDX;
    }
  }
    unsigned long long spill = spill1+spill2;
    return spill;
}

void OptiMacroNode::MM_optiPara(){
  num_spill = ULLONG_MAX;
  perf = ULLONG_MAX;
  sram_use = ULLONG_MAX;
  mem_access = ULLONG_MAX;
  
  cout << "in mm opti expr " << endl;
  MM_genSubblkSet();

  myfile.open("mtxmul_design_space.txt",ios::trunc);
  
  for(auto &i: sb_dim_set){

	  para.axes["i"].subblk_dim = i[0];
	  para.axes["j"].subblk_dim = i[1];
	  para.axes["l"].subblk_dim = i[2];

	  cout << "subblk dim " << i[0] << "," << i[1] << "," << i[2] << endl;
	  
	  for(auto &ax: para.axes){
	    ax.second.num_subblk = (ax.second.input_size%ax.second.subblk_dim==0)?ax.second.input_size/ax.second.subblk_dim:ax.second.input_size/ax.second.subblk_dim+1;
	  }

	  //assume each bank one port
	  para.dblks["A"].num_port = para.axes["l"].subblk_dim*para.axes["i"].subblk_dim;
	  para.dblks["B"].num_port = para.axes["l"].subblk_dim*para.axes["j"].subblk_dim;
	  para.dblks["C"].num_port = global_sp->total_port-para.dblks["A"].num_port-para.dblks["B"].num_port;
	  for(auto &d: para.dblks){
	    d.second.num_bank = d.second.num_port;
	  }

	  for(blk_dimi_sb=1; blk_dimi_sb<=para.axes["i"].num_subblk; blk_dimi_sb++){
	    int blk_dimi = blk_dimi_sb*para.axes["i"].subblk_dim;
	    if(MM_MinMem(blk_dimi, 1, 1) > global_sp->total_size){
	      break;
	    }
	    for(blk_dimj_sb=1; blk_dimj_sb<=para.axes["j"].num_subblk; blk_dimj_sb++){
	      int blk_dimj = blk_dimj_sb*para.axes["j"].subblk_dim;
	      if(MM_MinMem(blk_dimi, blk_dimj, 1) > global_sp->total_size){
	        break;
	      }
	      for(blk_diml_sb=1; blk_diml_sb<=para.axes["l"].num_subblk; blk_diml_sb++){
		int blk_diml = blk_diml_sb*para.axes["l"].subblk_dim;
	        if(MM_MinMem(blk_dimi, blk_dimj, blk_diml) > global_sp->total_size){
	          break;
	        }

		para.axes["i"].num_subblk_inblk = blk_dimi_sb;
		para.axes["j"].num_subblk_inblk = blk_dimj_sb;
		para.axes["l"].num_subblk_inblk = blk_diml_sb;

		para.axes["i"].blk_dim = blk_dimi;
		para.axes["j"].blk_dim = blk_dimj;
		para.axes["l"].blk_dim = blk_diml;

		for(auto &ax: para.axes){
		  ax.second.num_blk = (ax.second.input_size%ax.second.blk_dim==0)?ax.second.input_size/ax.second.blk_dim: ax.second.input_size/ax.second.blk_dim+1;
		  ax.second.ex_input_size = ax.second.num_blk*ax.second.blk_dim;
		}

		para.dblks["A"].size = para.axes["i"].blk_dim*para.axes["l"].blk_dim;
		para.dblks["B"].size = para.axes["j"].blk_dim*para.axes["l"].blk_dim;
		para.dblks["C"].size = para.axes["i"].blk_dim*para.axes["j"].blk_dim;
                
		int bank_size = Memory::membanks[0].size;
		bool can_fit = true;
		for(auto &d: para.dblks){
		  d.second.interval = (d.second.size%d.second.num_bank==0)? d.second.size/d.second.num_bank: d.second.size/d.second.num_bank+1;
		  d.second.num_blk_mem = bank_size/d.second.interval;
		  can_fit = can_fit && (d.second.num_blk_mem >= 1);
		}

		if(can_fit){

		  //cout << "blk dim " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << endl;

		  unsigned long long cur_spill = LLONG_MAX;
		  unsigned long long cur_spill1;
		  unsigned long long cur_spill2;
		  unsigned long long cur_perf;
		  unsigned long long cur_blk_perf;
		  unsigned long long cur_sram_use;

		  MM_updatePara();
		  para.total_compute_cycles = para.blk_cycles["mm"]*para.axes["i"].num_blk*para.axes["j"].num_blk*para.axes["l"].num_blk;
		  cur_blk_perf = para.blk_cycles["mm"];
		  cur_perf = para.total_compute_cycles;
		  unsigned long long num_mn = para.axes["i"].num_blk*para.axes["j"].num_blk*para.axes["l"].num_blk;
		  //cur_sram_use = para.mntemps["mm"].use_sram*para.axes["i"].num_blk;
		  //cur_sram_use *= para.axes["j"].num_blk;
		  //cur_sram_use *= para.axes["l"].num_blk;
		  cur_sram_use = para.mntemps["mm"].use_sram*num_mn;

		      
		      LoopOrder order1("i","j","l");
		      unsigned long long spill11;
		      unsigned long long spill12;
		      unsigned long long spill1 = MM_spill(order1, spill11, spill12);
		      //cout << "spill a " << spill11 << endl;
		      //cout << "spill b " << spill12 << endl;

		      myfile << i[0] << " " <<i[1] << " " <<i[2] << " i j l " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill1 << " " << cur_sram_use << " " << cur_perf << endl;
		      if(spill1 <= cur_spill){
		        cur_spill = spill1;
			para.loop_order = order1;
			cur_spill1 = spill11;
			cur_spill2 = spill12;
		      }

		      LoopOrder order2("j","i","l");
		      unsigned long long spill21;
		      unsigned long long spill22;
		      unsigned long long spill2 = MM_spill(order2, spill21, spill22);
		      if(spill2 <= cur_spill){
			cur_spill = spill2;
			para.loop_order = order2;
			cur_spill1 = spill21;
			cur_spill2 = spill22;
		      }
		      myfile << i[0] << " " <<i[1] << " " <<i[2] << " j i l " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill2 << " " << cur_sram_use << " " << cur_perf << endl;

		      LoopOrder order3("j","l","i");
		      unsigned long long spill31;
		      unsigned long long spill32;
		      unsigned long long spill3 = MM_spill(order3, spill31, spill32);
		      if(spill3 <= cur_spill){
			cur_spill = spill3;
			para.loop_order = order3;
			cur_spill1 = spill31;
			cur_spill2 = spill32;
		      }
		      myfile << i[0] << " " <<i[1] << " " <<i[2] << " j l i " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill3 << " " << cur_sram_use << " " << cur_perf << endl;
		      
		      LoopOrder order4("l","j","i");
		      unsigned long long spill41;
		      unsigned long long spill42;
		      unsigned long long spill4 = MM_spill(order4, spill41, spill42);
		      if(spill4 <= cur_spill){
			cur_spill = spill4;
			para.loop_order = order4;
			cur_spill1 = spill41;
			cur_spill2 = spill42;
		      }
		      myfile << i[0] << " " <<i[1] << " " <<i[2] << " l j i " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill4 << " " << cur_sram_use << " " << cur_perf << endl;

		      LoopOrder order5("i","l","j");
		      unsigned long long spill51;
		      unsigned long long spill52;
		      unsigned long long spill5 = MM_spill(order5, spill51, spill52);
		      if(spill5 <= cur_spill){
			cur_spill = spill5;
			para.loop_order = order5;
			cur_spill1 = spill51;
			cur_spill2 = spill52;
		      }
		      myfile << i[0] << " " <<i[1] << " " <<i[2] << " i l j " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill5 << " " << cur_sram_use << " " << cur_perf << endl;
		      
		      LoopOrder order6("l","i","j");
		      unsigned long long spill61;
		      unsigned long long spill62;
		      unsigned long long spill6 = MM_spill(order6, spill61, spill62);
		      if(spill6 <= cur_spill){
			cur_spill = spill6;
			para.loop_order = order6;
			cur_spill1 = spill61;
			cur_spill2 = spill62;
		      }
		      myfile << i[0] << " " <<i[1] << " " <<i[2] << " l i j " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill6 << " " << cur_sram_use << " " << cur_perf << endl;
		  
		  unsigned long long cur_mem_access = 100*cur_spill+cur_sram_use;

		  //if((cur_mem_access <= mem_access) || ((cur_spill == num_spill) && (cur_perf < perf))){
		  if((cur_spill < num_spill) || ((cur_spill == num_spill) && (cur_perf < perf))){
		    opti_para = para;
		    opti_para.num_spill = cur_spill;
		    opti_para.num_spill1 = cur_spill1;
		    opti_para.num_spill2 = cur_spill2;

		    num_spill = cur_spill;
		    perf = cur_perf;
		    sram_use = cur_sram_use;
		    mem_access = cur_mem_access;
		  }
		}
	      }
	    }
	  }
  }
  myfile.close();
}


struct blk_use{
  int blk;
  int prev;
  int next;
};

unsigned long long spill_case1(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;

  for(int group=0; group<num_blk; group++){
    vector<int> prev_blk_use(num_blk,-1);
    vector<blk_use> use_list;
    map<int,map<int,int>> u_b_index;

    for(int u=0; u<num_blk; u++){
      for(int b=0; b<min(group,u)+1; b++){
	int prev = prev_blk_use[b];
        use_list.push_back({b,prev,-1});
	if(prev != -1){
	  use_list[prev].next = use_list.size()-1;
	}
	prev_blk_use[b] = use_list.size()-1;
	u_b_index[u][b] = use_list.size()-1;
      }
    }

    //for each group
    for(int u=0; u<num_blk; u++){
      if(u <= group){
        //leve can add new eviction
	int num_b = min(group,u)+1;
	if(num_b > num_blk_mem){
	  //add new eviciton
	  int use = u_b_index[u][num_b-1];
	  while(use_list[use-1].next != -1){
	    spill += blk_size;
 	    use = use_list[use-1].next;
	  }
	}
      }
    }
  }

  //unsigned long long spill = 0;
  /*
  for(int i=0; i<num_blk; i++){
    for(int j=0; j<(num_blk-1); j++){
      //exist: min(j+1,i+1)
      int num_exist = min(j+1,i+1);
      spill += blk_size*max(num_exist-num_blk_mem,0);
    }
  }
  */
  return spill;
}

unsigned long long spill_case2(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  /*
  for(int i=0; i<(num_blk-1); i++){
    int num_exist = 0;
    for(int j=0; j<num_blk; j++){
      num_exist += min(j+1,i+1);
    }
    spill += blk_size*max(num_exist-num_blk_mem,0);
  }
  */

  vector<blk_use> use_list;
  vector<int> prev_blk_use(num_blk*num_blk,-1);
  map<int,map<int,int>> u_b_index;

  for(int u=0; u<num_blk; u++){
    for(int bcol=0; bcol<num_blk; bcol++){
      for(int b=0; b<num_blk; b++){
	if(b < (min(u,bcol)+1)){
	  int b_idx = bcol*num_blk+b;
	  u_b_index[u][b_idx] = use_list.size();
	  int prev = prev_blk_use[b_idx];
	  if(prev != -1){
	    use_list[prev].next = use_list.size();
	  }
	  prev_blk_use[b_idx] = use_list.size();
	  use_list.push_back({b_idx,prev,-1});
	}
      }
    }
  }

  int mem_used = 0;
  for(int u=0; u<num_blk; u++){
    //check all newly added block
    for(int bcol=0; bcol<num_blk; bcol++){
      for(int b=0; b<num_blk; b++){
	if((bcol >= u) && (b == u)){
	  //this is newly added block
	  int b_idx = bcol*num_blk+b;
	  if(mem_used == num_blk_mem){
	    int use = u_b_index[u][b_idx];
	    while(use_list[use-1].next != -1){
	      spill += blk_size;
	      use = use_list[use-1].next;
	    }
	  }
	  else{
	    mem_used++;
	  }
	}
      }
    }
  }
  return spill;
}

unsigned long long spill_case3(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  for(int group=0; group<num_blk; group++){
    vector<int> prev_blk_use(num_blk,-1);
    vector<blk_use> use_list;
    map<int,map<int,int>> u_b_index;

    for(int u=0; u<group+1; u++){              
      for(int b=0; b<num_blk; b++){
	if(b >= u){
	  int prev = prev_blk_use[b];
	  use_list.push_back({b,prev,-1});
	  if(prev != -1){
	    use_list[prev].next = use_list.size()-1;
	  }
	  prev_blk_use[b] = use_list.size()-1;
	  u_b_index[u][b] = use_list.size()-1;
	}
      }
    }

    //for each group
    int u=0;
    for(int b=0; b<num_blk; b++){
      if(b+1 > num_blk_mem){
        int use = u_b_index[u][b];
	while(use_list[use-1].next != -1){
	    spill += blk_size;
 	    use = use_list[use-1].next;
	}
      }
    }
  }
  /*
  for(int i=0; i<num_blk; i++){
    for(int j=num_blk; j>(num_blk-i); j--){
      spill += blk_size*max(j-1-num_blk_mem,0);
    }
  }
  */
  return spill;
}

unsigned long long spill_case4(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  vector<blk_use> use_list;
  vector<int> prev_blk_use(num_blk*num_blk,-1);
  map<int,map<int,int>> u_b_index;

  for(int u=0; u<num_blk; u++){
    for(int bcol=0; bcol<u+1; bcol++){
      for(int b=0; b<num_blk; b++){
	if(b < (num_blk-bcol)){
	  int b_idx = bcol*num_blk+b;
	  u_b_index[u][b_idx] = use_list.size();
	  int prev = prev_blk_use[b_idx];
	  if(prev != -1){
	    use_list[prev].next = use_list.size();
	  }
	  prev_blk_use[b_idx] = use_list.size();
	  use_list.push_back({b_idx,prev,-1});
	}
      }
    }
  }

  int mem_used = 0;
  for(int u=0; u<num_blk; u++){
    //check all newly added block
      int bcol = u;
      for(int b=0; b<num_blk; b++){
	if(b < (num_blk-bcol)){
	  //this is newly added block
	  int b_idx = bcol*num_blk+b;
	  if(mem_used == num_blk_mem){
	    int use = u_b_index[u][b_idx];
	    while(use_list[use-1].next != -1){
	      spill += blk_size;
	      use = use_list[use-1].next;
	    }
	  }
	  else{
	    mem_used++;
	  }
	}
      }
  }
  /*
  for(int i=0; i<(num_blk-1); i++){
    int num_exist = 0;
    for(int j=num_blk; j>=(num_blk-i); j--){
      num_exist += j;
    }
    spill += blk_size*max(num_exist-num_blk_mem,0);
  }
  */
  return spill;
}

unsigned long long spill_case5(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  for(int group=0; group<num_blk; group++){
    vector<int> prev_blk_use(num_blk,-1);
    vector<blk_use> use_list;
    map<int,map<int,int>> u_b_index;

    for(int u=0; u<(num_blk-group); u++){              
      for(int b=0; b<num_blk; b++){
	if(b < (num_blk-group)){
	  int prev = prev_blk_use[b];
	  use_list.push_back({b,prev,-1});
	  if(prev != -1){
	    use_list[prev].next = use_list.size()-1;
	  }
	  prev_blk_use[b] = use_list.size()-1;
	  u_b_index[u][b] = use_list.size()-1;
	}
      }
    }

    //for each group
    int u=0;
    for(int b=0; b<(num_blk-group); b++){
      if(b+1 > num_blk_mem){
        int use = u_b_index[u][b];
	while(use_list[use-1].next != -1){
	    spill += blk_size;
 	    use = use_list[use-1].next;
	}
      }
    }
  }

  /*
  for(int i=num_blk; i>0; i--){
    unsigned long long spill_t = max(i-num_blk_mem,0);
    spill_t = spill_t*(i-1);
    spill_t = spill_t*blk_size;
    spill += spill_t;
  }
  */

  return spill;
}

unsigned long long spill_case6(int blk_size, int num_blk, int num_blk_mem){
  unsigned long long spill = 0;
  vector<blk_use> use_list;
  vector<int> prev_blk_use(num_blk*num_blk,-1);
  map<int,map<int,int>> u_b_index;

  for(int u=0; u<num_blk; u++){
    for(int bcol=u; bcol<num_blk; bcol++){
      for(int b=0; b<num_blk; b++){
	if(b >= u){
	  int b_idx = bcol*num_blk+b;
	  u_b_index[u][b_idx] = use_list.size();
	  int prev = prev_blk_use[b_idx];
	  if(prev != -1){
	    use_list[prev].next = use_list.size();
	  }
	  prev_blk_use[b_idx] = use_list.size();
	  use_list.push_back({b_idx,prev,-1});
	}
      }
    }
  }

  int mem_used = 0;
  int u = 0;
  //check all newly added block
  for(int bcol=0; bcol<num_blk; bcol++){
      for(int b=0; b<num_blk; b++){
	  int b_idx = bcol*num_blk+b;
	  int use = u_b_index[u][b_idx];
	  if(mem_used == num_blk_mem){
	    while(use_list[use-1].next != -1){
	      spill += blk_size;
	      use = use_list[use-1].next;
	    }
	  }
	  else{
	    if(use_list[use].next != -1){
	      mem_used++;
	    }
	  }
      }
  }
  return spill;
}


void OptiMacroNode::LU_genSubblkSet(){
    /*
    for(int subblk_dim=1; subblk_dim<=para.axes["i"].input_size; subblk_dim++){
      if(subblk_dim+subblk_dim+1 > global_sp->total_port){
        break;
      }
      //too small
      if(subblk_dim+subblk_dim+subblk_dim*subblk_dim < (global_sp->total_port-2)){
        continue;
      }
      sb_dim_set.push_back(array<int,3>{subblk_dim,subblk_dim,1});
    }
    */
    int step = 10;
    for(int subblk_dimi=1; subblk_dimi<=para.axes["i"].input_size; subblk_dimi+=20){
      if(subblk_dimi+1+1 > global_sp->total_port){
        break;
      }
      for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj+=20){
        if(subblk_dimi+subblk_dimj+1 > global_sp->total_port){
	  break;
	}
	//too small
	if(subblk_dimi+subblk_dimj+subblk_dimi*subblk_dimj < (global_sp->total_port-2)){
	  continue;
	}
	sb_dim_set.push_back(array<int,3>{subblk_dimi,subblk_dimj,1});
      }
    }
}

unsigned long long OptiMacroNode::LU_spill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2){
  int num_blk = para.axes["i"].num_blk;
  if(lo.loop_idc[2] == "l"){
    if(lo.loop_idc[0] == "i"){
      //continuous reuse: A
      //line reuse: L
      //plane reuse: U
      blk_info& d1 = para.dblks["L"];
      spill1 = spill_case1(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["U"];
      spill2 = spill_case2(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      //continuous reuse: A
      //line reuse: U
      //plane reuse: L
      blk_info& d1 = para.dblks["U"];
      spill1 = spill_case1(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["L"];
      spill2 = spill_case2(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  else if(lo.loop_idc[2] == "i"){
    if(lo.loop_idc[0] == "j"){
      //continously reuse: U
      //line reuse: A
      //plane reuse: L
      blk_info& d1 = para.dblks["A"];
      spill1 = spill_case3(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["L"];
      spill2 = spill_case4(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      //continuously reuse: U
      //line reuse: L
      //plane reuse: A
      blk_info& d1 = para.dblks["L"];
      spill1 = spill_case5(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["A"];
      spill2 = spill_case6(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  else{
    if(lo.loop_idc[0] == "i"){
      //continuously reuse: L
      //line reuse: A
      //plane reuse: U
      blk_info& d1 = para.dblks["A"];
      spill1 = spill_case3(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["U"];
      spill2 = spill_case4(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      //continuously reuse: L
      //line reuse: U
      //plane reuse: A
      blk_info& d1 = para.dblks["U"];
      spill1 = spill_case5(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["A"];
      spill2 = spill_case6(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  unsigned long long spill = spill1 + spill2;
  return spill;
}

void OptiMacroNode::LU_optiPara(){
  num_spill = ULLONG_MAX;
  perf = ULLONG_MAX;
  sram_use = ULLONG_MAX;
  
  LU_genSubblkSet();

  for(auto &i: sb_dim_set){

    	  cout << "subblk dim " << i[0] << " " << i[1] << " " << i[2] << endl;
	  para.axes["i"].subblk_dim = i[0];
	  para.axes["j"].subblk_dim = i[1];
	  para.axes["l"].subblk_dim = i[2];
	  for(auto &ax: para.axes){
	    ax.second.num_subblk = (ax.second.input_size%ax.second.subblk_dim==0)?ax.second.input_size/ax.second.subblk_dim:ax.second.input_size/ax.second.subblk_dim+1;
	  }

	  int port_per_bank = Memory::membanks[0].num_port;
	  para.dblks["L"].num_port = para.axes["i"].subblk_dim;
	  para.dblks["U"].num_port = para.axes["j"].subblk_dim;
	  para.dblks["A"].num_port = global_sp->total_port-para.dblks["L"].num_port-para.dblks["U"].num_port;
	  for(auto &d: para.dblks){
	    d.second.num_bank = d.second.num_port;
	  }

	  for(int blk_dim=max(para.axes["i"].subblk_dim,para.axes["j"].subblk_dim); blk_dim<=para.axes["i"].input_size; blk_dim++){
	    if(MM_MinMem(blk_dim, blk_dim, 1) > global_sp->total_size){
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
		  d.second.interval = (d.second.size%d.second.num_bank==0)? d.second.size/d.second.num_bank: d.second.size/d.second.num_bank+1;
		  d.second.num_blk_mem = bank_size/d.second.interval;
		  can_fit = can_fit && (d.second.num_blk_mem >= 1);
		}

		if(can_fit){
	 	
		  unsigned long long cur_spill = ULLONG_MAX; 
		  unsigned long long cur_spill1;
		  unsigned long long cur_spill2;
		  
		  unsigned long long cur_perf;
	 	  unsigned long long cur_sram_use;

		  LU_updatePara();
		  LUCPL_updatePara();
		  TRS_updatePara();
		  SUBMM_updatePara();

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
		    cur_sram_use += (l-1)*(l-1)*para.mntemps["submm"].use_sram;
		  }
		  cur_perf = para.total_compute_cycles;

		  //find the smallest num_dblk
		      LoopOrder order1("i","j","l");
		      unsigned long long spill11;
		      unsigned long long spill12;
		      unsigned long long spill1 = LU_spill(order1, spill11, spill12);
		      //myfile << i[0] << " " <<i[1] << " " <<i[2] << " i j l " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill11 << " " << spill12 << " " << spill1 << " " << cur_blk_perf << " " << cur_perf << endl;
		      if(spill1 <= cur_spill){
		        cur_spill = spill1;
			para.loop_order = order1;
			cur_spill1 = spill11;
			cur_spill2 = spill12;
		      }

		      LoopOrder order2("j","i","l");
		      unsigned long long spill21;
		      unsigned long long spill22;
		      unsigned long long spill2 = LU_spill(order2, spill21, spill22);
		      if(spill2 <= cur_spill){
			cur_spill = spill2;
			para.loop_order = order2;
			cur_spill1 = spill21;
			cur_spill2 = spill22;
		      }

		      //myfile << i[0] << " " << i[1] << " " << i[2] << " j i l " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill21 << " " << spill22 << " " << spill2 << " " << cur_blk_perf << " " << cur_perf << endl;

		      LoopOrder order3("j","l","i");
		      unsigned long long spill31;
		      unsigned long long spill32;
		      unsigned long long spill3 = LU_spill(order3, spill31, spill32);
		      if(spill3 <= cur_spill){
			cur_spill = spill3;
			para.loop_order = order3;
			cur_spill1 = spill31;
			cur_spill2 = spill32;
		      }
		      //myfile << i[0] <<" " << i[1] <<" " << i[2] << " j l i " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill31 << " " << spill32 << " " << spill3 << " " << cur_blk_perf << " " << cur_perf << endl;

		      LoopOrder order4("l","j","i");
		      unsigned long long spill41;
		      unsigned long long spill42;
		      unsigned long long spill4 = LU_spill(order4, spill41, spill42);
		      if(spill4 <= cur_spill){
			cur_spill = spill4;
			para.loop_order = order4;
			cur_spill1 = spill41;
			cur_spill2 = spill42;
		      }
		      //myfile << i[0] << " " << i[1] << " " << i[2] << " l j i " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill41 << " " << spill42 << " " << spill4 << " " << cur_blk_perf << " " << cur_perf << endl;

		      LoopOrder order5("i","l","j");
		      unsigned long long spill51;
		      unsigned long long spill52;
		      unsigned long long spill5 = LU_spill(order5, spill51, spill52);
		      if(spill5 <= cur_spill){
			cur_spill = spill5;
			para.loop_order = order5;
			cur_spill1 = spill51;
			cur_spill2 = spill52;
		      }
		      //myfile <<i[0] << " " << i[1] << " " << i[2] << " i l j " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill51 << " " << spill52 << " " << spill5 << " " << cur_blk_perf << " " << cur_perf << endl;
		      
		      LoopOrder order6("l","i","j");
		      unsigned long long spill61;
		      unsigned long long spill62;
		      unsigned long long spill6 = LU_spill(order6, spill61, spill62);
		      if(spill6 <= cur_spill){
			cur_spill = spill6;
			para.loop_order = order6;
			cur_spill1 = spill61;
			cur_spill2 = spill62;
		      }

		  //if(((cur_spill <= num_spill) && (cur_sram_use <= sram_use))
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


void OptiMacroNode::QR_genSubblkSet(){
    for(int subblk_diml=1; subblk_diml<=para.axes["l"].input_size; subblk_diml++){
      if(subblk_diml+1+subblk_diml > global_sp->total_port){
        break;
      }
      for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj++){
        if(subblk_dimj+subblk_diml+subblk_dimj*subblk_diml > global_sp->total_port){
	  break;
	}
        //too small
        if(subblk_dimj+subblk_diml+subblk_dimj*subblk_diml < (global_sp->total_port-2)){
          continue;
        }
        sb_dim_set.push_back(array<int,3>{1, subblk_dimj, subblk_diml});
      }
    }
}

unsigned long long OptiMacroNode::QR_spill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2){
  int num_blk = para.axes["i"].num_blk;
  if(lo.loop_idc[2] == "l"){
    if(lo.loop_idc[0] == "i"){
      blk_info& d1 = para.dblks["Q"];
      spill1 = spill_case1(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["R"];
      spill2 = spill_case2(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      blk_info& d1 = para.dblks["R"];
      spill1 = spill_case1(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["Q"];
      spill2 = spill_case2(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  else if(lo.loop_idc[2] == "i"){
    if(lo.loop_idc[0] == "j"){
      blk_info& d1 = para.dblks["A"];
      spill1 = spill_case3(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["Q"];
      spill2 = spill_case4(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      blk_info& d1 = para.dblks["Q"];
      spill1 = spill_case5(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["A"];
      spill2 = spill_case6(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  else{
    if(lo.loop_idc[0] == "i"){
      blk_info& d1 = para.dblks["A"];
      spill1 = spill_case3(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["R"];
      spill2 = spill_case4(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      blk_info& d1 = para.dblks["R"];
      spill1 = spill_case5(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["A"];
      spill2 = spill_case6(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  unsigned long long spill = spill1 + spill2;
  return spill;
}

void OptiMacroNode::QR_optiPara(){
  num_spill = ULLONG_MAX;
  perf = ULLONG_MAX;
  sram_use = ULLONG_MAX;

  QR_genSubblkSet();

  for(auto &i: sb_dim_set){

	  para.axes["i"].subblk_dim = i[0];
	  para.axes["j"].subblk_dim = i[1];
	  para.axes["l"].subblk_dim = i[2];

	  cout << endl << "subblk dim " << i[0] << "," << i[1] << "," << i[2] << endl;

	  for(auto &ax: para.axes){
	    ax.second.num_subblk = (ax.second.input_size%ax.second.subblk_dim==0)?ax.second.input_size/ax.second.subblk_dim:ax.second.input_size/ax.second.subblk_dim+1;
	  }

	  para.dblks["Q"].num_port = para.axes["l"].subblk_dim;
	  para.dblks["R"].num_port = para.axes["j"].subblk_dim;
	  para.dblks["A"].num_port = para.axes["j"].subblk_dim*para.axes["l"].subblk_dim;
	  for(auto &d: para.dblks){
	    d.second.num_bank = d.second.num_port;
	  }

	  for(int blk_dim=max(para.axes["j"].subblk_dim,para.axes["l"].subblk_dim); blk_dim<=para.axes["i"].input_size; blk_dim++){
	    if(MM_MinMem(blk_dim, blk_dim, blk_dim) > global_sp->total_size){
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

		para.dblks["Q"].size = (para.axes["i"].blk_dim+1)*para.axes["i"].blk_dim;
		para.dblks["R"].size = para.axes["i"].blk_dim*para.axes["i"].blk_dim;
		para.dblks["A"].size = para.axes["i"].blk_dim*para.axes["i"].blk_dim;
                
		int bank_size = Memory::membanks[0].size;
		bool can_fit = true;
		for(auto &d: para.dblks){
		  d.second.interval = (d.second.size%d.second.num_bank==0)? d.second.size/d.second.num_bank: d.second.size/d.second.num_bank+1;
		  d.second.num_blk_mem = bank_size/d.second.interval;
		  can_fit = can_fit && (d.second.num_blk_mem >= 1);
		}

		if(can_fit){
	 	
		  unsigned long long cur_spill = ULLONG_MAX;
		  unsigned long long cur_spill1;
		  unsigned long long cur_spill2;
		  
		  unsigned long long cur_perf;
		  unsigned long long cur_sram_use;

		  QR_updatePara();
		  QRCPL_updatePara();
		  QRUPDATETR_updatePara();
		  QRUPDATE_updatePara();
		  para.total_compute_cycles = 0;
		  cur_sram_use = 0;
		  for(int l=para.axes["i"].num_blk; l>=1; l--){
		    para.total_compute_cycles += para.blk_cycles["qr"];
		    cur_sram_use += para.mntemps["qr"].use_sram;
		    para.total_compute_cycles += (l-1)*para.blk_cycles["qrcpl"];
		    cur_sram_use += para.mntemps["qrcpl"].use_sram;
		    para.total_compute_cycles += (l-1)*para.blk_cycles["qrupdatetr"];
		    cur_sram_use += para.mntemps["qrupdatetr"].use_sram;
		    para.total_compute_cycles += (l-1)*(l-1)*para.blk_cycles["qrupdate"];
		    cur_sram_use += para.mntemps["qrupdate"].use_sram;
		  }
		  cur_perf = para.total_compute_cycles;
		  cout << "cur perf " << cur_perf << endl;

		      LoopOrder order1("i","j","l");
		      unsigned long long spill11;
		      unsigned long long spill12;
		      unsigned long long spill1 = QR_spill(order1, spill11, spill12);
		      //myfile << i[0] << " " <<i[1] << " " <<i[2] << " i j l " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill11 << " " << spill12 << " " << spill1 << " " << cur_blk_perf << " " << cur_perf << endl;
		      if(spill1 <= cur_spill){
		        cur_spill = spill1;
			para.loop_order = order1;
			cur_spill1 = spill11;
			cur_spill2 = spill12;
		      }

		      LoopOrder order2("j","i","l");
		      unsigned long long spill21;
		      unsigned long long spill22;
		      unsigned long long spill2 = QR_spill(order2, spill21, spill22);
		      if(spill2 <= cur_spill){
			cur_spill = spill2;
			para.loop_order = order2;
			cur_spill1 = spill21;
			cur_spill2 = spill22;
		      }
		      //myfile << i[0] << " " << i[1] << " " << i[2] << " j i l " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill21 << " " << spill22 << " " << spill2 << " " << cur_blk_perf << " " << cur_perf << endl;

		      LoopOrder order3("j","l","i");
		      unsigned long long spill31;
		      unsigned long long spill32;
		      unsigned long long spill3 = QR_spill(order3, spill31, spill32);
		      if(spill3 <= cur_spill){
			cur_spill = spill3;
			para.loop_order = order3;
			cur_spill1 = spill31;
			cur_spill2 = spill32;
		      }
		      //myfile << i[0] <<" " << i[1] <<" " << i[2] << " j l i " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill31 << " " << spill32 << " " << spill3 << " " << cur_blk_perf << " " << cur_perf << endl;
		      
		      LoopOrder order4("l","j","i");
		      unsigned long long spill41;
		      unsigned long long spill42;
		      unsigned long long spill4 = QR_spill(order4, spill41, spill42);
		      if(spill4 <= cur_spill){
			cur_spill = spill4;
			para.loop_order = order4;
			cur_spill1 = spill41;
			cur_spill2 = spill42;
		      }
		      //myfile << i[0] << " " << i[1] << " " << i[2] << " l j i " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill41 << " " << spill42 << " " << spill4 << " " << cur_blk_perf << " " << cur_perf << endl;

		      LoopOrder order5("i","l","j");
		      unsigned long long spill51;
		      unsigned long long spill52;
		      unsigned long long spill5 = QR_spill(order5, spill51, spill52);
		      if(spill5 <= cur_spill){
			cur_spill = spill5;
			para.loop_order = order5;
			cur_spill1 = spill51;
			cur_spill2 = spill52;
		      }
		      //myfile <<i[0] << " " << i[1] << " " << i[2] << " i l j " << para.axes["i"].blk_dim << " " << para.axes["j"].blk_dim << " " << para.axes["l"].blk_dim << " " << spill51 << " " << spill52 << " " << spill5 << " " << cur_blk_perf << " " << cur_perf << endl;
		      
		      LoopOrder order6("l","i","j");
		      unsigned long long spill61;
		      unsigned long long spill62;
		      unsigned long long spill6 = QR_spill(order6, spill61, spill62);
		      if(spill6 <= cur_spill){
			cur_spill = spill6;
			para.loop_order = order6;
			cur_spill1 = spill61;
			cur_spill2 = spill62;
		      }

			
		  //if(((cur_spill <= num_spill) && (cur_sram_use <= sram_use))
		  if((cur_spill < num_spill)
		      || ((cur_spill == num_spill) && (cur_perf < perf))){
		    opti_para = para;
		    cout << "replace " << endl;
		    cout << "port " << opti_para.dblks["A"].num_port << " " << opti_para.dblks["L"].num_port << " " << opti_para.dblks["U"].num_port << endl;
		    opti_para.num_spill = cur_spill;

		    num_spill = cur_spill;
		    perf = cur_perf;
		    sram_use = cur_sram_use;
		  }

		}
	  }
  }

}


void OptiMacroNode::optiPara(){
  if(app == "MM"){
    MM_optiPara();
  }
  else if(app == "LU"){
    LU_optiPara();
  }
  else if(app == "QR"){
    QR_optiPara();
  }
}

void OptiMacroNode::optiPara_kernel(){
  if(app == "mm"){
    MM_optiPara_kernel();
  }
  else if(app == "lu"){
    LU_optiPara_kernel();
  }
  else if(app == "lucpl"){
    LUCPL_optiPara_kernel();
  }
  else if(app == "trs"){
    TRS_optiPara_kernel();
  }
  else if(app == "submm"){
    SUBMM_optiPara_kernel();
  }
  else if(app == "qr"){
    QR_optiPara_kernel();
  }
  else if(app == "qrcpl"){
    QRCPL_optiPara_kernel();
  }
  else if(app == "qrupdatetr"){
    QRUPDATETR_optiPara_kernel();
  }
  else if(app == "qrupdate"){
    QRUPDATE_optiPara_kernel();
  }
}

void OptiMacroNode::setPara(){
  
  //*****************App level parameters
  opti_para.axes["i"].subblk_dim = 3;
  opti_para.axes["j"].subblk_dim = 4;
  
  //opti_para.subblk_dim["mm_i"] = 6;
  //opti_para.subblk_dim["mm_j"] = 2;
  //opti_para.subblk_dim["mm_l"] = 1;
  
  opti_para.axes["i"].blk_dim = 8;
  opti_para.axes["j"].blk_dim = 8;

  opti_para.axes["i"].num_blk = 8;
  //opti_para.blk_dim["mm_i"] = 48;
  //opti_para.blk_dim["mm_j"] = 6;
  //opti_para.blk_dim["mm_l"] = 2;

  opti_para.axes["i"].ex_input_size = opti_para.axes["i"].blk_dim;
  opti_para.axes["j"].ex_input_size = opti_para.axes["j"].blk_dim;

  //opti_para.ex_input_size["mm_m"] = opti_para.blk_dim["mm_i"];
  //opti_para.ex_input_size["mm_n"] = opti_para.blk_dim["mm_j"];
  //opti_para.ex_input_size["mm_k"] = opti_para.blk_dim["mm_l"];

  //opti_para.dblks["Q"].num_port = 3;
  //opti_para.dblks["R"].num_port = 4;
  //opti_para.dblks["A"].num_port = 24;

  opti_para.dblks["A"].num_port = 6;
  opti_para.dblks["B"].num_port = 2;
  opti_para.dblks["C"].num_port = 24;

  //opti_para.dblks["Q"].num_bank = 2;
  //opti_para.dblks["R"].num_bank = 2;
  //opti_para.dblks["A"].num_bank = 12;

  opti_para.dblks["A"].num_bank = 3;
  opti_para.dblks["B"].num_bank = 1;
  opti_para.dblks["C"].num_bank = 12;


  //opti_para.num_blk["mm_i"] = 1;
  //opti_para.num_blk["mm_j"] = 1;
  //opti_para.num_blk["mm_l"] = 1;
  //***************************************

  //*****************mn temp level parameters
  /*
  opti_para.cbs["qr"]["acc_mul"].num_cb = 1;
  opti_para.cbs["qr"]["acc_mul"].k_stage = 1;
  opti_para.cbs["qr"]["qr_householder_para"].num_cb = 1;

  opti_para.cbs["qr"]["div"].num_cb = 3;

  opti_para.cbs["qr"]["qr_householder_accmul"].num_cb = 3;
  opti_para.cbs["qr"]["qr_householder_accmul"].width = 4;
  opti_para.cbs["qr"]["copy"].num_cb = 4;
 
  opti_para.cbs["qrcpl"]["acc_mul"].num_cb = 1;
  opti_para.cbs["qrcpl"]["qr_householder_para"].num_cb = 1;
  opti_para.cbs["qrcpl"]["div"].num_cb = 3;
  opti_para.cbs["qrcpl"]["qr_householder_accmul"].num_cb = 3;
  opti_para.cbs["qrcpl"]["qr_householder_accmul"].width = 4;
  opti_para.cbs["qrcpl"]["copy"].num_cb = 4;
  opti_para.cbs["qrcpl"]["mul"].num_cb = 4;
 
  opti_para.cbs["qrupdatediag"]["mul"].num_cb = 12;
  
  opti_para.cbs["qrupdatetr"]["qr_householder_accmul"].num_cb = 3;
  opti_para.cbs["qrupdatetr"]["qr_householder_accmul"].width = 4;
  opti_para.cbs["qrupdatetr"]["copy"].num_cb = 4;
  
  opti_para.cbs["qrupdate"]["qr_householder_accmul"].num_cb = 3;
  opti_para.cbs["qrupdate"]["qr_householder_accmul"].width = 4;
  opti_para.cbs["qrupdate"]["copy"].num_cb = 4;
  */

  //opti_para.cbs["lucpl"]["sub_mul"].num_cb = 12;
  //opti_para.cbs["lucpl"]["sub_mul"].k_stage = 1;
  //opti_para.cbs["lucpl"]["div"].num_cb = 3;

  //opti_para.cbs["trs"]["div"].num_cb = 4;
  //opti_para.cbs["trs"]["copy"].num_cb = 4;
  //opti_para.cbs["trs"]["sub_mul"].num_cb = 12;
  //opti_para.cbs["trs"]["sub_mul"].k_stage = 1;

  opti_para.cbs["mm"]["acc_mul"].num_cb = 12;
  opti_para.cbs["mm"]["acc_mul"].k_stage = 1;
  
  //opti_para.loop_order = LoopOrder("m","n","k");

  //opti_para.num_spill = 0;
  //opti_para.blk_compute_cycles = 0;
  //opti_para.total_compute_cycles = 0;
  //opti_para.num_spill = cur_spill;
  //opti_para.total_compute_cycles = cur_perf;
}


/*
bool OptiMacroNode::ConstraintBW_buffer(int k_subblk, int m_subblk, int n_subblk){
  int port_per_bank = Memory::membanks[0].num_port;

  int num_port_a = k_subblk*m_subblk;
  num_bank_a = (num_port_a%port_per_bank==0)?num_port_a/port_per_bank:num_port_a/port_per_bank+1;

  int num_port_b = k_subblk*n_subblk;
  num_bank_b = (num_port_b%port_per_bank==0)?num_port_b/port_per_bank:num_port_b/port_per_bank+1;

  int num_port_c = 2*m_subblk*n_subblk;
  num_bank_c = (num_port_c%port_per_bank==0)?num_port_c/port_per_bank:num_port_c/port_per_bank+1;

  if(2*num_bank_a+2*num_bank_b+2*num_bank_c <= Memory::num_bank){
    return true;
  }
  else{
    return false;
  }
}


unsigned long long OptiMacroNode::spill_a_buffer(){
  unsigned long long spill;
  int num_a_region = 2*Memory::membanks[0].size/a_region;

  if(num_a_region >= blk_k){
    spill = 0;
  }
  else{
    spill = a*(blk_k+1-num_a_region)*(blk_n-1)*blk_m;
  }
  return spill;
}

unsigned long long OptiMacroNode::spill_b_buffer(){
  unsigned long long spill;
  int num_b_region = 2*Memory::membanks[0].size/b_region;

  if(num_b_region >= blk_n*blk_k){
    spill = 0;
  }
  else{
    spill = b*(blk_n*blk_k+1-num_b_region)*(blk_m-1);
  }
  return spill;
}

void OptiMacroNode::optiPara_buffer(){
  //cout << "Memory banks: " << Memory::num_bank << endl;
  //cout << "bank 0 size: " << Memory::membanks[0].size << endl;

  num_spill = LLONG_MAX;
  perf = LLONG_MAX;
  for(subblk_dimi=1; subblk_dimi<=m; subblk_dimi++){
    for(subblk_dimj=1; subblk_dimj<=n; subblk_dimj++){
      for(subblk_diml=1; subblk_diml<=k; subblk_diml++){
 	if(ConstraintBW_buffer(subblk_diml, subblk_dimi, subblk_dimj)){
	  cout << endl;
	  //cout << "subblk dimi: " << subblk_dimi << endl;
	  //cout << "subblk dimj: " << subblk_dimj << endl;
	  //cout << "subblk diml: " << subblk_diml << endl;

	  int subblk_m = (m%subblk_dimi==0)?m/subblk_dimi:m/subblk_dimi+1;
	  int subblk_n = (n%subblk_dimj==0)?n/subblk_dimj:n/subblk_dimj+1;
	  int subblk_k = (k%subblk_diml==0)?k/subblk_diml:k/subblk_diml+1;
	
	  for(blk_dimi_sb=1; blk_dimi_sb<=subblk_m; blk_dimi_sb++){
	    blk_dimi = blk_dimi_sb*subblk_dimi;
	    for(blk_dimj_sb=1; blk_dimj_sb<=subblk_n; blk_dimj_sb++){
	      blk_dimj = blk_dimj_sb*subblk_dimj;
	      for(blk_diml_sb=1; blk_diml_sb<=subblk_k; blk_diml_sb++){
		blk_diml = blk_diml_sb*subblk_diml;

	        blk_m = (m%blk_dimi==0)?m/blk_dimi: m/blk_dimi+1;
	        blk_n = (n%blk_dimj==0)?n/blk_dimj: n/blk_dimj+1;
	        blk_k = (k%blk_diml==0)?k/blk_diml: k/blk_diml+1;

		a = blk_dimi*blk_diml;
		b = blk_dimj*blk_diml;
		c = blk_dimi*blk_dimj;
                
		a_region = (a%num_bank_a==0)?a/num_bank_a:a/num_bank_a+1;
		b_region = (b%num_bank_b==0)?b/num_bank_b:b/num_bank_b+1;
		c_region = (a%num_bank_c==0)?c/num_bank_c:c/num_bank_c+1;

		if((a_region <= Memory::membanks[0].size)
		    && (b_region <= Memory::membanks[0].size)
		    && (c_region <= Memory::membanks[0].size)){
	 
		  unsigned long long cur_spill = spill_a_buffer()+spill_b_buffer();
		  unsigned long long cur_perf = getPerf();

		  if((cur_spill < num_spill)
		      || ((cur_spill == num_spill) && (cur_perf < perf))){
		    num_spill = cur_spill;
		    perf = cur_perf;

		    opti_para.subblk_dimi = subblk_dimi;
		    opti_para.subblk_dimj = subblk_dimj;
		    opti_para.subblk_diml = subblk_diml;
		    opti_para.blk_dimi = blk_dimi;
		    opti_para.blk_dimj = blk_dimj;
		    opti_para.blk_diml = blk_diml;

		    opti_para.m_ex = blk_m*blk_dimi;
		    opti_para.n_ex = blk_n*blk_dimj;
		    opti_para.k_ex = blk_k*blk_diml;

		    opti_para.num_bank_a = num_bank_a;
		    opti_para.num_bank_b = num_bank_b;
		    opti_para.num_bank_c = num_bank_c;
		  }

		}
	      }
	    }
	  }
	  //cout << "num spill " << num_spill << endl;
	}
      }
    }
  }

  opti_para.k_stage = opti_para.subblk_diml;
  opti_para.num_cb = opti_para.subblk_dimi*opti_para.subblk_dimj;

  //cout << "num spill " << num_spill << endl;
  //cout << "perf " << perf << endl;
}
*/


