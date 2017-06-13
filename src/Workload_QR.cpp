#include "limits.h"
#include "Workload_QR.hpp"

void Workload_QR::setupDataInfo(){
  datas_info.push_back(data_info("Q", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  datas_info.push_back(data_info("R", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  datas_info.push_back(data_info("A", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
}

void Workload_QR::genSubblkSet(vector<array<int,3>> &sb_dim_set){
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

void Workload_QR::updatePara_QR(){
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

    //time for generating new a
    int num_w_batches = ((n-col)%para.axes["l"].subblk_dim==0)?(n-col)/para.axes["l"].subblk_dim: (n-col)/para.axes["l"].subblk_dim+1;
    int latency_gen_a_batch = fus["mul"]+fus["sub"]+(num_w_batches-1);

    cur_num_cycles += latency_gen_a_batch*num_batches;
  }

  int use_sram = 0;
  for(int c=0; c<n; c++){
    use_sram += (n-c); //read A col->norm2
    use_sram += (n-c-1); //read A col->div for w
    use_sram += (n-c); //write w col
    use_sram += (n-c)*(n-c-1); //read a trailing matrix
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

void Workload_QR::updatePara_QRCPL(){
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

    //time for generating new a
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
    use_sram += n*(n-c-1); //read a trailing matrix
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

void Workload_QR::updatePara_QRUPDATETR(){
  int cur_num_cycles = 0;
  int n = para.axes["i"].blk_dim;

  for(int col=0; col<n-1; col++){
    int max_num_batches_ingroup = fus["add"];
    int num_batches = (n%para.axes["j"].subblk_dim==0)?n/para.axes["j"].subblk_dim:n/para.axes["j"].subblk_dim+1;
    int num_groups = (num_batches%max_num_batches_ingroup==0)?num_batches/max_num_batches_ingroup:num_batches/max_num_batches_ingroup+1;

    //time for each group
    cur_num_cycles += fus["mul"]+num_groups*(n-col)*fus["add"];

    //time for generating new a
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

void Workload_QR::updatePara_QRUPDATE(){
  int cur_num_cycles = 0;
  int n = para.axes["i"].blk_dim;

  for(int col=0; col<n-1; col++){
    int max_num_batches_ingroup = fus["add"];
    int num_batches = (n%para.axes["j"].subblk_dim==0)?n/para.axes["j"].subblk_dim:n/para.axes["j"].subblk_dim+1;
    int num_groups = (num_batches%max_num_batches_ingroup==0)?num_batches/max_num_batches_ingroup:num_batches/max_num_batches_ingroup+1;

    //time for each group
    cur_num_cycles += fus["mul"]+num_groups*(n+1)*fus["add"];

    //time for generating new a
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



unsigned long long Workload_QR::getSpill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2){
  int num_blk = para.axes["i"].num_blk;
  if(lo.loop_idc[2] == "l"){
    if(lo.loop_idc[0] == "i"){
      blk_info& d1 = para.dblks["Q"];
      spill1 = SpillCase1(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["R"];
      spill2 = SpillCase2(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      blk_info& d1 = para.dblks["R"];
      spill1 = SpillCase1(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["Q"];
      spill2 = SpillCase2(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  else if(lo.loop_idc[2] == "i"){
    if(lo.loop_idc[0] == "j"){
      blk_info& d1 = para.dblks["A"];
      spill1 = SpillCase3(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["Q"];
      spill2 = SpillCase4(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      blk_info& d1 = para.dblks["Q"];
      spill1 = SpillCase5(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["A"];
      spill2 = SpillCase6(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  else{
    if(lo.loop_idc[0] == "i"){
      blk_info& d1 = para.dblks["A"];
      spill1 = SpillCase3(d1.size, num_blk, d1.num_blk_mem);
      
      blk_info& d2 = para.dblks["R"];
      spill2 = SpillCase4(d2.size, num_blk, d2.num_blk_mem);
    }
    else{
      blk_info& d1 = para.dblks["R"];
      spill1 = SpillCase5(d1.size, num_blk, d1.num_blk_mem);

      blk_info& d2 = para.dblks["A"];
      spill2 = SpillCase6(d2.size, num_blk, d2.num_blk_mem);
    }
  }
  unsigned long long spill = spill1 + spill2;
  return spill;
}

void Workload_QR::optiPara(){
  unsigned long long num_spill = ULLONG_MAX;
  unsigned long long perf = ULLONG_MAX;
  unsigned long long sram_use = ULLONG_MAX;

  vector<array<int,3>> sb_dim_set;
  genSubblkSet(sb_dim_set);

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

    for(int blk_dim = max(para.axes["j"].subblk_dim, para.axes["l"].subblk_dim); blk_dim <= para.axes["i"].input_size; blk_dim++){
      if(MinMem(blk_dim, blk_dim, blk_dim) > global_sp->total_size){
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

	updatePara_QR();
	updatePara_QRCPL();
	updatePara_QRUPDATETR();
	updatePara_QRUPDATE();
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
	  cout << "replace " << endl;
	  cout << "port " << opti_para.dblks["A"].num_port << " " << opti_para.dblks["Q"].num_port << " " << opti_para.dblks["R"].num_port << endl;
	  opti_para.num_spill = cur_spill;

	  num_spill = cur_spill;
	  perf = cur_perf;
	  sram_use = cur_sram_use;
	}
      }
    }
  }
}

void Workload_QR::genMacroNode(){
  int blk_m = opti_para.axes["i"].blk_dim;

  kernal_out_dblk.insert("Q");
  kernal_out_dblk.insert("R");

  //generate mns
  map<string, dblk_info> dblks_info_qr;
  mn_temp["qr"] = new MN_QR(blk_m, dblks_info_qr);
  mn_temps.push_back(mn_temp["qr"]);

  map<string, dblk_info> dblks_info_qrcpl;
  mn_temp["qrcpl"] = new MN_QRCPL(blk_m, dblks_info_qrcpl);
  mn_temps.push_back(mn_temp["qrcpl"]);

  map<string, dblk_info> dblks_info_qrupdatetr;
  mn_temp["qrupdatetr"] = new MN_QRUpdateTr(blk_m, dblks_info_qrupdatetr);
  mn_temps.push_back(mn_temp["qrupdatetr"]);

  map<string, dblk_info> dblks_info_qrupdate;
  mn_temp["qrupdate"] = new MN_QRUpdate(blk_m, dblks_info_qrupdate);
  mn_temps.push_back(mn_temp["qrupdate"]);

  load_store = shared_ptr<LoadStoreDblk>(new LoadStoreDblk);

  cout << "create all mn temp " << endl;
  
  func_iteration = &Workload_QR::setDblkUseTime;
  LoopStructure();

  func_iteration = &Workload_QR::runMacroNode;
  LoopStructure();
}

void Workload_QR::LoopStructure(){
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

void Workload_QR::setDblkIdx(){
  dblk_idx.clear();
  dblk_idx["Q"] = blk_i*opti_para.axes["i"].num_blk+blk_l;
  dblk_idx["R"] = blk_l*opti_para.axes["i"].num_blk+blk_j;
  dblk_idx["A"] = blk_i*opti_para.axes["i"].num_blk+blk_j;

  //do something
  (this->*func_iteration)();

  macro_time++;
}

void Workload_QR::runMacroNode(){
  in_dblk.clear();
  out_dblk.clear();

  string mn_temp_name;

  if((blk_i == blk_l) && (blk_j == blk_l)){
    in_dblk.insert("A");
    out_dblk.insert("Q");
    out_dblk.insert("R");

    mn_temp_name = "qr";
  }
  else if(blk_j == blk_l){
    in_dblk.insert("A");
    in_dblk.insert("R");
    out_dblk.insert("Q");
    out_dblk.insert("R");

    mn_temp_name = "qrcpl";
  }
  else if(blk_i == blk_l){
    in_dblk.insert("A");
    in_dblk.insert("Q");
    out_dblk.insert("R");

    mn_temp_name = "qrupdatetr";
  }
  else{
    in_dblk.insert("Q");
    in_dblk.insert("R");
    in_dblk.insert("A");
    out_dblk.insert("A");
    out_dblk.insert("R");

    mn_temp_name = "qrupdate";
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


