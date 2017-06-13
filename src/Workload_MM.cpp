#include "limits.h"
#include "Workload_MM.hpp"

void Workload_MM::setupDataInfo(){
  datas_info.push_back(data_info("A", opti_para.axes["i"].ex_input_size, opti_para.axes["l"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["l"].blk_dim));
  datas_info.push_back(data_info("B", opti_para.axes["l"].ex_input_size, opti_para.axes["j"].ex_input_size, opti_para.axes["l"].blk_dim, opti_para.axes["j"].blk_dim));
  datas_info.push_back(data_info("C", opti_para.axes["i"].ex_input_size, opti_para.axes["j"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["j"].blk_dim));
}

int MinPort(int subblk_dimi, int subblk_dimj, int subblk_diml){
  int port_a = subblk_dimi*subblk_diml;
  int port_b = subblk_dimj*subblk_diml;
  int port_c = 1;
  return (port_a+port_b+port_c);
}

void Workload_MM::genSubblkSet(vector<array<int,3>> &sb_dim_set){
  int step = 20;
  for(int subblk_dimi=1; subblk_dimi<=para.axes["i"].input_size; subblk_dimi+=step){
    if(MinPort(subblk_dimi, 1, 1) > global_sp->total_port){
      break;
    }
    for(int subblk_dimj=1; subblk_dimj<=para.axes["j"].input_size; subblk_dimj+=step){
      if(MinPort(subblk_dimi, subblk_dimj, 1) > global_sp->total_port){
        break;
      }
      for(int subblk_diml=1; subblk_diml<=para.axes["l"].input_size; subblk_diml+=step){
        if(MinPort(subblk_dimi, subblk_dimj, subblk_diml) > global_sp->total_port){
	  break;
	}
	//too small
	if(subblk_dimi*subblk_diml+subblk_dimj*subblk_diml+subblk_dimi*subblk_dimj < 0.9*global_sp->total_port){
	  continue;
	}
	sb_dim_set.push_back(array<int,3>{subblk_dimi, subblk_dimj, subblk_diml});
      }
    }
  }
}

void Workload_MM::updatePara(){
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

  //latency of pipelines
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

unsigned long long normal_spill(blk_info& spill_dblk, int num_exist, int num_reuse, int iterate){
  unsigned long long spill;
  if(num_exist <= spill_dblk.num_blk_mem){
    spill = 0;
  }
  else{
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

unsigned long long Workload_MM::getSpill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2){
  int blk_m = para.axes["i"].num_blk;
  int blk_n = para.axes["j"].num_blk;
  int blk_k = para.axes["l"].num_blk;

  string matrix_nospill = (lo.loop_idc[2]=="i")?"B":(lo.loop_idc[2]=="j")?"A":"C";
  if(matrix_nospill == "A"){
    if(lo.loop_idc[0] == "i"){
      lo.matrix_spilltype1 = "C";
      spill1 = normal_spill(para.dblks["C"],blk_n,blk_k,blk_m);
      lo.idx_mode1 = COL_IDX;
      
      lo.matrix_spilltype2 = "B";
      spill2 = normal_spill(para.dblks["B"],blk_k*blk_n,blk_m,1);
      lo.idx_mode2 = ROWMAJOR_IDX;
    }
    else{
      lo.matrix_spilltype1 = "B";
      spill1 = normal_spill(para.dblks["B"],blk_n,blk_m,blk_k);
      lo.idx_mode1 = COL_IDX;

      lo.matrix_spilltype2 = "C";
      spill2 = normal_spill(para.dblks["C"],blk_m*blk_n,blk_k,1);
      lo.idx_mode2 = ROWMAJOR_IDX;
    }
  }
  else if(matrix_nospill == "B"){
    if(lo.loop_idc[0] == "l"){
      lo.matrix_spilltype1 = "A";
      spill1 = normal_spill(para.dblks["A"],blk_m,blk_n,blk_k);
      lo.idx_mode1 = ROW_IDX;

      lo.matrix_spilltype2 = "C";
      spill2 = normal_spill(para.dblks["C"],blk_m*blk_n,blk_k,1);
      lo.idx_mode2 = COLMAJOR_IDX;
    }
    else{
      lo.matrix_spilltype1 = "C";
      spill1 = normal_spill(para.dblks["C"],blk_m,blk_k,blk_n);
      lo.idx_mode1 = ROW_IDX;

      lo.matrix_spilltype2 = "A";
      spill2 = normal_spill(para.dblks["A"],blk_m*blk_k,blk_n,1);
      lo.idx_mode2 = COLMAJOR_IDX;
    }
  }
  else{
    if(lo.loop_idc[0] == "i"){
      lo.matrix_spilltype1 = "A";
      spill1 = normal_spill(para.dblks["A"],blk_k,blk_n,blk_m);
      lo.idx_mode1 = COL_IDX;

      lo.matrix_spilltype2 = "B";
      spill2 = normal_spill(para.dblks["B"],blk_k*blk_n,blk_m,1);
      lo.idx_mode2 = COLMAJOR_IDX;
    }
    else{
      lo.matrix_spilltype1 = "B";
      spill1 = normal_spill(para.dblks["B"],blk_k,blk_m,blk_n);
      lo.idx_mode1 = ROW_IDX;

      lo.matrix_spilltype2 = "A";
      spill2 = normal_spill(para.dblks["A"],blk_k*blk_m,blk_n,1);
      lo.idx_mode2 = ROWMAJOR_IDX;
    }
  }

  unsigned long long spill = spill1+spill2;
  return spill;
}

void Workload_MM::optiPara(){
  cout << "insode workload mm optipara" << endl; 

  unsigned long long num_spill = ULLONG_MAX;
  unsigned long long perf = ULLONG_MAX;
  unsigned long long sram_use = ULLONG_MAX;
  unsigned long long mem_access = ULLONG_MAX;

  vector<array<int,3>> sb_dim_set;
  genSubblkSet(sb_dim_set);

  for(auto &i: sb_dim_set){
    para.axes["i"].subblk_dim = i[0];
    para.axes["j"].subblk_dim = i[1];
    para.axes["l"].subblk_dim = i[2];

    for(auto &ax: para.axes){
      ax.second.num_subblk = (ax.second.input_size%ax.second.subblk_dim==0)?ax.second.input_size/ax.second.subblk_dim:ax.second.input_size/ax.second.subblk_dim+1;
    }

    //assume each bank has one port
    para.dblks["A"].num_port = para.axes["l"].subblk_dim*para.axes["i"].subblk_dim;
    para.dblks["B"].num_port = para.axes["l"].subblk_dim*para.axes["j"].subblk_dim;
    para.dblks["C"].num_port = global_sp->total_port-para.dblks["A"].num_port-para.dblks["B"].num_port;

    for(auto &d: para.dblks){
      d.second.num_bank = d.second.num_port;
    }

    int blk_dimi_sb = 0;
    int blk_dimj_sb = 0;
    int blk_diml_sb = 0;
    for(blk_dimi_sb=1; blk_dimi_sb<=para.axes["i"].num_subblk; blk_dimi_sb++){
      int blk_dimi = blk_dimi_sb*para.axes["i"].subblk_dim;
      if(MinMem(blk_dimi, 1, 1) > global_sp->total_size){
        break;
      }
      for(blk_dimj_sb=1; blk_dimj_sb<=para.axes["j"].num_subblk; blk_dimj_sb++){
        int blk_dimj = blk_dimj_sb*para.axes["j"].subblk_dim;
	if(MinMem(blk_dimi, blk_dimj, 1) > global_sp->total_size){
	  break;
	}
	for(blk_diml_sb=1; blk_diml_sb<=para.axes["l"].num_subblk; blk_diml_sb++){
	  int blk_diml = blk_diml_sb*para.axes["l"].subblk_dim;
	  if(MinMem(blk_dimi, blk_dimj, blk_diml) > global_sp->total_size){
	    break;
	  }

	  para.axes["i"].num_subblk_inblk = blk_dimi_sb;
	  para.axes["j"].num_subblk_inblk = blk_dimj_sb;
	  para.axes["l"].num_subblk_inblk = blk_diml_sb;

	  para.axes["i"].blk_dim = blk_dimi;
	  para.axes["j"].blk_dim = blk_dimj;
	  para.axes["l"].blk_dim = blk_diml;

	  for(auto &ax: para.axes){
	    ax.second.num_blk = (ax.second.input_size%ax.second.blk_dim==0)?ax.second.input_size/ax.second.blk_dim:ax.second.input_size/ax.second.blk_dim+1;
	    ax.second.ex_input_size = ax.second.num_blk*ax.second.blk_dim;
	  }

	  para.dblks["A"].size = para.axes["i"].blk_dim*para.axes["l"].blk_dim;
	  para.dblks["B"].size = para.axes["j"].blk_dim*para.axes["l"].blk_dim;
	  para.dblks["C"].size = para.axes["i"].blk_dim*para.axes["j"].blk_dim;

	  int bank_size = Memory::membanks[0].size;
	  bool can_fit = true;
	  for(auto &d: para.dblks){
	    d.second.interval = (d.second.size%d.second.num_bank==0)?d.second.size/d.second.num_bank: d.second.size/d.second.num_bank+1;
	    d.second.num_blk_mem = bank_size/d.second.interval;
	    can_fit = can_fit && (d.second.num_blk_mem >= 1);
	  }

	  if(can_fit){
	    unsigned long long cur_spill = LLONG_MAX;
	    unsigned long long cur_spill1;
	    unsigned long long cur_spill2;
	    unsigned long long cur_perf;
	    unsigned long long cur_blk_perf;
	    unsigned long long cur_sram_use;

	    updatePara();
	    unsigned long long num_mn = para.axes["i"].num_blk*para.axes["j"].num_blk*para.axes["l"].num_blk;
	    
	    para.total_compute_cycles = para.blk_cycles["mm"]*num_mn;
	    cur_blk_perf = para.blk_cycles["mm"];
	    cur_perf = para.total_compute_cycles;

	    cur_sram_use = para.mntemps["mm"].use_sram*num_mn;

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

	    unsigned long long cur_mem_access = 100*cur_spill+cur_sram_use;
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
}

void Workload_MM::genMacroNode(){
  //generate mns
  kernal_out_dblk.insert("C"); 
  int blk_m = opti_para.axes["i"].blk_dim;
  int blk_n = opti_para.axes["j"].blk_dim;
  int blk_k = opti_para.axes["l"].blk_dim;

  map<string, dblk_info> dblks_info;
  dblks_info["A"] = {"A","A","mode_mm_a",opti_para.axes["i"].subblk_dim,opti_para.axes["l"].subblk_dim,blk_m,blk_k};
  dblks_info["B"] = {"B","B","mode_mm_b",opti_para.axes["l"].subblk_dim,opti_para.axes["j"].subblk_dim,blk_k,blk_n};
  dblks_info["C"] = {"C","C","mode_mm_c",opti_para.axes["i"].subblk_dim,opti_para.axes["j"].subblk_dim,blk_m,blk_n};

  mn_temp["mm"] = new MN_mtxmul(true, blk_m, blk_n, blk_k, dblks_info);
  mn_temps.push_back(mn_temp["mm"]);

  //transaction
  load_store = shared_ptr<LoadStoreDblk>(new LoadStoreDblk);

  func_iteration = &Workload_MM::setDblkUseTime;
  LoopStructure();

  func_iteration = &Workload_MM::runMacroNode;
  LoopStructure();
}

void Workload_MM::LoopStructure(){
  int blk_m = opti_para.axes["i"].num_blk;
  int blk_n = opti_para.axes["j"].num_blk;
  int blk_k = opti_para.axes["l"].num_blk;

  macro_time = 0;
  if(opti_para.loop_order.loop_idc == array<string,3>{"i","j","l"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_j=0; blk_j<blk_n; blk_j++){
        for(blk_l=0; blk_l<blk_k; blk_l++){
	  setDblkIdx();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"i","l","j"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_l=0; blk_l<blk_k; blk_l++){
        for(blk_j=0; blk_j<blk_n; blk_j++){
	  setDblkIdx();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","l","i"}){
    for(blk_j=0; blk_j<blk_n; blk_j++){
      for(blk_l=0; blk_l<blk_k; blk_l++){
        for(blk_i=0; blk_i<blk_m; blk_i++){
	  setDblkIdx();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","i","l"}){
    for(blk_j=0; blk_j<blk_n; blk_j++){
      for(blk_i=0; blk_i<blk_m; blk_i++){
        for(blk_l=0; blk_l<blk_k; blk_l++){
	  setDblkIdx();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"l","i","j"}){
    for(blk_l=0; blk_l<blk_k; blk_l++){
      for(blk_i=0; blk_i<blk_m; blk_i++){
        for(blk_j=0; blk_j<blk_n; blk_j++){
	  setDblkIdx();
	}
      }
    }
  }
  else{
    for(blk_l=0; blk_l<blk_k; blk_l++){
      for(blk_j=0; blk_j<blk_n; blk_j++){
        for(blk_i=0; blk_i<blk_m; blk_i++){
	  setDblkIdx();
	}
      }
    }
  }
}

void Workload_MM::setDblkIdx(){
  dblk_idx.clear();
  dblk_idx["A"] = blk_i*opti_para.axes["l"].num_blk+blk_l;
  dblk_idx["B"] = blk_l*opti_para.axes["j"].num_blk+blk_j;
  dblk_idx["C"] = blk_i*opti_para.axes["j"].num_blk+blk_j;

  //do something
  (this->*func_iteration)();

  macro_time++;
}

void Workload_MM::runMacroNode(){
  in_dblk.clear();
  in_dblk.insert("A");
  in_dblk.insert("B");
  in_dblk.insert("C");

  out_dblk.clear();
  out_dblk.insert("C");

  prepareDblks();

  //mn
  MacroNode mn(mn_temp["mm"], mns.size());
  mns_perf.cycle += mn.mn_temp->cycle_length;
  mns_perf.total_use += mn.mn_temp->total_use;
  mns_perf.use_sram += mn.mn_temp->use_sram;
  mns_perf.use_pipe += mn.mn_temp->use_pipe;
  mns_perf.num_compute_cycles += mn.mn_temp->cycle_length;
  mns.push_back(mn);

}
