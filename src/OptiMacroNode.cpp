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
}

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

void LoopOrder::setDblkSPAddrIdx(){
  data_arrays[matrix_nospill]->setDblkSPAddrIdx(ZERO_IDX);
  data_arrays[matrix_spilltype1]->setDblkSPAddrIdx(idx_mode1);
  data_arrays[matrix_spilltype2]->setDblkSPAddrIdx(idx_mode2);
}

Parameters::Parameters(){
}

void Parameters::PrintInfo(){
  cout << endl << "Extended input size: " << endl;
  for(auto &i: ex_input_size){
    cout << "ex_" << i.first << ": " << i.second << endl;
  }

  cout << endl << "First blocking level: " << endl;
  cout << "Block dimension: " << endl;
  for(auto &i: blk_dim){
    cout << "    blk_dimi_" << i.first << ": " << i.second << endl;
  }
  cout << "Data block size: " << endl;
  for(auto &i: blk_size){
    cout << "    block size of " << i.first << ": " << i.second << endl;
  }
  cout << "Number of data blocks: " << endl;
  for(auto &i: num_blk){
    cout << "    num_blk_" << i.first << ": " << i.second << endl;
  }

  cout << endl << "Second blocking level: " << endl;
  cout << "Sub-block dimension: " << endl;
  for(auto &i: subblk_dim){
    cout << "   subblk_dim_" << i.first << ": " << i.second << endl;
  }

  cout << endl << "Compute block parameters: " << endl;
  for(auto &i: num_cb){
    for(auto &j: i.second){
      cout << "Parallelization of cb " << i.first+"_"+j.first << ": " << j.second << endl;
    }
  }

  cout << endl << "Loop order: " << endl;
  cout << loop_order.loop_idc[0] << "->" << loop_order.loop_idc[1] << "->" << loop_order.loop_idc[2] << endl;

  cout << endl << "Estimated number of spills: " << num_spill << endl;
  cout << "Estimated #compute cycles per block: " << blk_compute_cycles << endl;
  cout << "Estimated #compute cycles: " << total_compute_cycles << endl;
}

OptiMacroNode::OptiMacroNode(int in_m, int in_n, int in_k, Parameters& in_opti_para):opti_para(in_opti_para){
  m = in_m;
  n = in_n;
  k = in_k;
}

int OptiMacroNode::MinMem(int blk_dimi, int blk_dimj, int blk_diml){
    return (blk_dimi*blk_diml+blk_dimj*blk_diml+blk_dimi*blk_dimj);
}

int OptiMacroNode::MinBank(int m_subblk, int n_subblk, int k_subblk){
  int port_per_bank = Memory::membanks[0].num_port;

  int num_port_a = k_subblk*m_subblk;
  num_bank_a = (num_port_a%port_per_bank==0)?num_port_a/port_per_bank:num_port_a/port_per_bank+1;

  int num_port_b = k_subblk*n_subblk;
  num_bank_b = (num_port_b%port_per_bank==0)?num_port_b/port_per_bank:num_port_b/port_per_bank+1;

  int num_port_c = 2*m_subblk*n_subblk;
  num_bank_c = (num_port_c%port_per_bank==0)?num_port_c/port_per_bank:num_port_c/port_per_bank+1;

  return (num_bank_a+num_bank_b+num_bank_c);
}


int OptiMacroNode::MinPort(int sb_dimi, int sb_dimj, int sb_diml){
  return (sb_diml*sb_dimi + sb_diml*sb_dimj + 2*sb_dimi*sb_dimj);
}

void OptiMacroNode::genSubblkSet(){
  num_port_used = 0;
  for(subblk_dimi=1; subblk_dimi<=m; subblk_dimi++){
    if(MinPort(subblk_dimi, 1, 1) > global_sp->total_port){
      break;
    }
    for(subblk_dimj=1; subblk_dimj<=n; subblk_dimj++){
      if(MinPort(subblk_dimi, subblk_dimj, 1) > global_sp->total_port){
        break;
      }
      for(subblk_diml=1; subblk_diml<=k; subblk_diml++){
	int min_bank = MinBank(subblk_dimi, subblk_dimj, subblk_diml);
	if(min_bank < Memory::num_bank){
	  int min_port = MinPort(subblk_dimi, subblk_dimj, subblk_diml);
	  //cout << endl << "min port" << min_port << endl;
	  //cout << "num port used " << num_port_used << endl;
	  if(min_port >= num_port_used){
	    array<int,3> subblk_dim = {subblk_dimi, subblk_dimj, subblk_diml};
	    if(min_port > num_port_used){
	      sb_dim_set.clear();
	      num_port_used = min_port;
	    }
	    sb_dim_set.push_back(subblk_dim);
	  }
	}
	else{
	  break;
	}
      }
    }
  }
}


unsigned long long OptiMacroNode::spill(LoopOrder& loop_order, unsigned long long& spill1, unsigned long long& spill2){
  //cout << "spill type1" << endl;
  //cout << "blk size " << loop_order.dblk1_size << endl;
  //cout << "#blk in mem " << loop_order.num_dblk1_mem << endl;
  //cout << "#blk need " << loop_order.num_dblk1_need << endl;
  //cout << "blk reuse " << loop_order.num_reuse1 << endl;
  //cout << "num iterate " << loop_order.num_iterate << endl;
  spill1 = spill_type1(loop_order.dblk1_size, loop_order.num_dblk1_mem, loop_order.num_dblk1_need, loop_order.num_reuse1, loop_order.num_iterate);

  //cout << "spill type2" << endl;
  //cout << "blk size " << loop_order.dblk2_size << endl;
  //cout << "#blk in mem " << loop_order.num_dblk2_mem << endl;
  //cout << "#blk need " << loop_order.num_dblk2_need << endl;
  //cout << "blk reuse " << loop_order.num_reuse2 << endl;
  spill2 = spill_type2(loop_order.dblk2_size, loop_order.num_dblk2_mem, loop_order.num_dblk2_need, loop_order.num_reuse2);
  
  unsigned long long spill = spill1+spill2;
  return spill;
}

unsigned long long OptiMacroNode::spill_type1(int blk_size, int num_dblk_mem, int num_dblk_need, int num_reuse, int iterate){
  unsigned long long spill;

  if(num_dblk_mem >= num_dblk_need){
    spill = 0;
  }
  else{
    spill = max(num_dblk_need-num_dblk_mem, 0);
    spill = spill*blk_size;
    spill = spill*(num_reuse-1);
    spill = spill*iterate;
  }
  return spill;
}

unsigned long long OptiMacroNode::spill_type2(int blk_size, int num_dblk_mem, int num_dblk_need, int num_reuse){
  unsigned long long spill;

  
  if(num_dblk_mem >= num_dblk_need){
    spill = 0;
  }
  else{
    spill = max(num_dblk_need-num_dblk_mem, 0);
    spill = spill*blk_size;
    spill = spill*(num_reuse-1);
  }
  return spill;
}

void OptiMacroNode::getPerf(unsigned long long& perf, unsigned long long& blk_perf){

  int num_subblk_level = blk_dimi_sb*blk_dimj_sb;
  int latency_cb = (subblk_diml-1)+3+1+2;

  blk_perf = max(num_subblk_level, latency_cb)*blk_diml_sb+min(num_subblk_level,latency_cb)-1;
  perf = blk_perf*blk_m*blk_n*blk_k;
}

void OptiMacroNode::optiPara(){
  num_spill = ULLONG_MAX;
  perf = ULLONG_MAX;
  
  genSubblkSet();
  //cout << endl << "***********" << "subblk dim sets: " << endl;
  //for(auto &i: sb_dim_set){
  //  cout << i[0] << " " << i[1] << " " << i[2] << endl;
  //}

  for(auto &i: sb_dim_set){
	  subblk_dimi = i[0];
	  subblk_dimj = i[1];
	  subblk_diml = i[2];

	  //cout << endl << "*************subblk" << endl;
	  //cout << "subblk_dimi " << subblk_dimi << endl;
	  //cout << "subblk_dimj " << subblk_dimj << endl;
	  //cout << "subblk_diml " << subblk_diml << endl;

	  int port_per_bank = 2;
	  int num_port_a = subblk_diml*subblk_dimi;
	  num_bank_a = (num_port_a%port_per_bank==0)?num_port_a/port_per_bank:num_port_a/port_per_bank+1;

	  int num_port_b = subblk_diml*subblk_dimj;
	  num_bank_b = (num_port_b%port_per_bank==0)?num_port_b/port_per_bank:num_port_b/port_per_bank+1;

	  int num_port_c = 2*subblk_dimi*subblk_dimj;
	  num_bank_c = (num_port_c%port_per_bank==0)?num_port_c/port_per_bank:num_port_c/port_per_bank+1;


	  int subblk_m = (m%subblk_dimi==0)?m/subblk_dimi:m/subblk_dimi+1;
	  int subblk_n = (n%subblk_dimj==0)?n/subblk_dimj:n/subblk_dimj+1;
	  int subblk_k = (k%subblk_diml==0)?k/subblk_diml:k/subblk_diml+1;
	
	  for(blk_dimi_sb=1; blk_dimi_sb<=subblk_m; blk_dimi_sb++){
	    blk_dimi = blk_dimi_sb*subblk_dimi;
	    if(MinMem(blk_dimi, 1, 1) > global_sp->total_size){
	      break;
	    }
	    for(blk_dimj_sb=1; blk_dimj_sb<=subblk_n; blk_dimj_sb++){
	      blk_dimj = blk_dimj_sb*subblk_dimj;
	      if(MinMem(blk_dimi, blk_dimj, 1) > global_sp->total_size){
	        break;
	      }
	      for(blk_diml_sb=1; blk_diml_sb<=subblk_k; blk_diml_sb++){
		blk_diml = blk_diml_sb*subblk_diml;
	        if(MinMem(blk_dimi, blk_dimj, blk_diml) > global_sp->total_size){
	          break;
	        }

	        blk_m = (m%blk_dimi==0)?m/blk_dimi: m/blk_dimi+1;
	        blk_n = (n%blk_dimj==0)?n/blk_dimj: n/blk_dimj+1;
	        blk_k = (k%blk_diml==0)?k/blk_diml: k/blk_diml+1;

		a = blk_dimi*blk_diml;
		b = blk_dimj*blk_diml;
		c = blk_dimi*blk_dimj;
                
		a_interval = (a%num_bank_a==0)?a/num_bank_a:a/num_bank_a+1;
		b_interval = (b%num_bank_b==0)?b/num_bank_b:b/num_bank_b+1;
		c_interval = (a%num_bank_c==0)?c/num_bank_c:c/num_bank_c+1;

		int bank_size = Memory::membanks[0].size;
		num_ablk_mem = bank_size/a_interval;
		num_bblk_mem = bank_size/b_interval;
		num_cblk_mem = bank_size/c_interval;

		if((num_ablk_mem >= 1)
		    && (num_bblk_mem >= 1)
		    && (num_cblk_mem >= 1)){
		    
		  //cout << endl << "blk_dimi " << blk_dimi << endl;
		  //cout << "blk_dimj " << blk_dimj << endl;
		  //cout << "blk_diml " << blk_diml << endl;
		  //cout << "blk_m " << blk_m << endl;
		  //cout << "blk_n " << blk_n << endl;
		  //cout << "blk_k " << blk_k << endl;
	 	
		  unsigned long long cur_spill;
		  unsigned long long cur_spill1;
		  unsigned long long cur_spill2;
	
		  LoopOrder order("m","n","k");
		  order.setupLoopOrder(this);
		  cur_spill = spill(order, cur_spill1, cur_spill2);
		  loop_order = order;

		  //find the smallest num_dblk
		  if((num_ablk_mem <= num_bblk_mem) && (num_ablk_mem <= num_cblk_mem)){
		    LoopOrder order1("m","k","n");
		    LoopOrder order2("k","m","n");
		    order1.setupLoopOrder(this);
		    order2.setupLoopOrder(this);

		    unsigned long long spill11;
		    unsigned long long spill12;
		    unsigned long long spill1 = spill(order1, spill11, spill12);

		    unsigned long long spill21;
		    unsigned long long spill22;
		    unsigned long long spill2 = spill(order2, spill21, spill22);

		    if(spill1 <= spill2){
		      cur_spill = spill1;
		      loop_order = order1;
		      cur_spill1 = spill11;
		      cur_spill2 = spill12;
		    }
		    else{
		      cur_spill = spill2;
		      loop_order = order2;
		      cur_spill1 = spill21;
		      cur_spill2 = spill22;
		    }
		  }
		  else if((num_bblk_mem <= num_ablk_mem) && (num_bblk_mem <= num_cblk_mem)){
		    LoopOrder order1("n","k","m");
		    LoopOrder order2("k","n","m");
		    order1.setupLoopOrder(this);
		    order2.setupLoopOrder(this);

		    unsigned long long spill11;
		    unsigned long long spill12;
		    unsigned long long spill1 = spill(order1, spill11, spill12);
		    
		    unsigned long long spill21;
		    unsigned long long spill22;
		    unsigned long long spill2 = spill(order2, spill21, spill22);

		    if(spill1 <= spill2){
		      cur_spill = spill1;
		      loop_order = order1;
		      cur_spill1 = spill11;
		      cur_spill2 = spill12;
		    }
		    else{
		      cur_spill = spill2;
		      loop_order = order2;
		      cur_spill1 = spill21;
		      cur_spill2 = spill22;
		    }
		  }
		  else{
		    LoopOrder order1("m","n","k");
		    LoopOrder order2("n","m","k");
		    order1.setupLoopOrder(this);
		    order2.setupLoopOrder(this);

		    unsigned long long spill11;
		    unsigned long long spill12;
		    unsigned long long spill1 = spill(order1, spill11, spill12);
		    
		    unsigned long long spill21;
		    unsigned long long spill22;
		    unsigned long long spill2 = spill(order2, spill21, spill22);

		    if(spill1 <= spill2){
		      cur_spill = spill1;
		      loop_order = order1;
		      cur_spill1 = spill11;
		      cur_spill2 = spill12;
		    }
		    else{
		      cur_spill = spill2;
		      loop_order = order2;
		      cur_spill1 = spill21;
		      cur_spill2 = spill22;
		    }
		  }
			
		  unsigned long long cur_perf;
		  unsigned long long cur_blk_perf;
		  getPerf(cur_perf, cur_blk_perf);
		    
		  //cout << "num spill " << num_spill << endl;
		  //cout << "cur spill " << cur_spill << endl;
		  //cout << "cur spill1 " << cur_spill1 << endl;
		  //cout << "cur spill2 " << cur_spill2 << endl;
		  //cout << "cur perf " << cur_perf << endl;
		
		  if((cur_spill < num_spill)
		      || ((cur_spill == num_spill) && (cur_perf < perf))){
		    
		    //cout << "replace !" << endl;
		    //cout << endl << "blk_dimi " << blk_dimi << endl;
		    //cout << "blk_dimj " << blk_dimj << endl;
		    //cout << "blk_diml " << blk_diml << endl;
		  
		    //cout << "num spill " << num_spill << endl;
		    //cout << "cur spill " << cur_spill << endl;
		    //cout << "cur spill1 " << cur_spill1 << endl;
		    //cout << "cur spill2 " << cur_spill2 << endl;
		    //cout << "perf " << cur_perf << endl;

		    opti_para.subblk_dim["i"] = subblk_dimi;
		    opti_para.subblk_dim["j"] = subblk_dimj;
		    opti_para.subblk_dim["l"] = subblk_diml;
		    opti_para.blk_dim["i"] = blk_dimi;
		    opti_para.blk_dim["j"] = blk_dimj;
		    opti_para.blk_dim["l"] = blk_diml;

		    opti_para.ex_input_size["m"] = blk_m*blk_dimi;
		    opti_para.ex_input_size["n"] = blk_n*blk_dimj;
		    opti_para.ex_input_size["k"] = blk_k*blk_diml;

		    opti_para.num_bank["A"] = num_bank_a;
		    opti_para.num_bank["B"] = num_bank_b;
		    opti_para.num_bank["C"] = num_bank_c;
	
		    opti_para.num_port["A"] = opti_para.subblk_dim["i"]*opti_para.subblk_dim["l"];
		    opti_para.num_port["B"] = opti_para.subblk_dim["j"]*opti_para.subblk_dim["l"];
		    opti_para.num_port["C"] = 2*opti_para.subblk_dim["i"]*opti_para.subblk_dim["j"];

		    opti_para.num_blk["i"] = blk_m;
		    opti_para.num_blk["j"] = blk_n;
		    opti_para.num_blk["l"] = blk_k;

		    opti_para.blk_size["A"] = opti_para.blk_dim["i"]*opti_para.blk_dim["l"];
		    opti_para.blk_size["B"] = opti_para.blk_dim["j"]*opti_para.blk_dim["l"];
		    opti_para.blk_size["C"] = opti_para.blk_dim["i"]*opti_para.blk_dim["j"];
		    
		    opti_para.loop_order = loop_order;

		    opti_para.num_spill = cur_spill;
		    opti_para.total_compute_cycles = cur_perf;
		    opti_para.blk_compute_cycles = cur_blk_perf;

		    num_spill = cur_spill;
		    perf = cur_perf;
		  }

		}
	      }
	    }
	  }
  }

  opti_para.k_stage["mm"]["mul_acc"] = opti_para.subblk_dim["l"];
  opti_para.num_cb["mm"]["mul_acc"] = opti_para.subblk_dim["i"]*opti_para.subblk_dim["j"];
  //cout << "num spill " << num_spill << endl;
  //cout << "perf " << perf << endl;
}

void OptiMacroNode::setPara(){
  
  //*****************App level parameters
  opti_para.subblk_dim["lu_i"] = 3;
  opti_para.subblk_dim["lu_j"] = 4;
  
  //opti_para.subblk_dim["mm_i"] = 6;
  //opti_para.subblk_dim["mm_j"] = 2;
  //opti_para.subblk_dim["mm_l"] = 1;
  
  opti_para.blk_dim["lu_i"] = 8;
  opti_para.blk_dim["lu_j"] = 8;

  //opti_para.blk_dim["mm_i"] = 48;
  //opti_para.blk_dim["mm_j"] = 6;
  //opti_para.blk_dim["mm_l"] = 2;

  opti_para.ex_input_size["lu_m"] = opti_para.blk_dim["lu_i"];
  opti_para.ex_input_size["lu_n"] = opti_para.blk_dim["lu_j"];

  //opti_para.ex_input_size["mm_m"] = opti_para.blk_dim["mm_i"];
  //opti_para.ex_input_size["mm_n"] = opti_para.blk_dim["mm_j"];
  //opti_para.ex_input_size["mm_k"] = opti_para.blk_dim["mm_l"];

  opti_para.num_port["LU_L"] = 3;
  opti_para.num_port["LU_U"] = 4;
  opti_para.num_port["LU_A"] = 24;

  //opti_para.num_port["MM_A"] = 6;
  //opti_para.num_port["MM_B"] = 2;
  //opti_para.num_port["MM_C"] = 24;

  opti_para.num_bank["LU_L"] = 2;
  opti_para.num_bank["LU_U"] = 2;
  opti_para.num_bank["LU_A"] = 12;

  //opti_para.num_bank["MM_A"] = 3;
  //opti_para.num_bank["MM_B"] = 1;
  //opti_para.num_bank["MM_C"] = 12;

  opti_para.num_blk["lu_i"] = 1;
  opti_para.num_blk["lu_j"] = 1;

  //opti_para.num_blk["mm_i"] = 1;
  //opti_para.num_blk["mm_j"] = 1;
  //opti_para.num_blk["mm_l"] = 1;
  //***************************************

  //*****************mn temp level parameters
  opti_para.num_cb["lu"]["sub_mul"] = 12;
  opti_para.k_stage["lu"]["sub_mul"] = 1;
  
  opti_para.num_cb["lu"]["copy"] = 4;
  opti_para.num_cb["lu"]["div"] = 3;
  
  opti_para.num_cb["lucpl"]["sub_mul"] = 12;
  opti_para.k_stage["lucpl"]["sub_mul"] = 1;
  opti_para.num_cb["lucpl"]["div"] = 3;

  opti_para.num_cb["trs"]["div"] = 4;
  opti_para.num_cb["trs"]["copy"] = 4;
  opti_para.num_cb["trs"]["sub_mul"] = 12;
  opti_para.k_stage["trs"]["sub_mul"] = 1;


  opti_para.num_cb["mm"]["acc_mul"] = 12;
  opti_para.k_stage["mm"]["acc_mul"] = 1;
  
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



