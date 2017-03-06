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
  blk_dimi = 0;
  blk_dimj = 0;
  blk_diml = 0;

  subblk_dimi = 0;
  subblk_dimj = 0;
  subblk_diml = 0;

  k_stage = 0;
  num_cb = 0;

  num_bank_a = 0;
  num_bank_b = 0;
  num_bank_c = 0;

  m_ex = 0;
  n_ex = 0;
  k_ex = 0;

}

void Parameters::PrintInfo(){
  int blk_m = m_ex/blk_dimi;
  int blk_n = n_ex/blk_dimj;
  int blk_k = k_ex/blk_diml;
  
  cout << endl << "Matrix Multiplication" << endl;
  cout << "m: " << m_ex << endl;
  cout << "n: " << n_ex << endl;
  cout << "k: " << k_ex << endl;

  cout << endl << "First blocking level: " << endl;
  cout << "Block dimension: " << endl;
  cout << "    blk_dim_i: " << blk_dimi << endl;
  cout << "    blk_dim_j: " << blk_dimj << endl;
  cout << "    blk_dim_l: " << blk_diml << endl;
  cout << "Data block size: " << endl;
  cout << "    block of A (blk_dim_i*blk_diml): " << blk_dimi*blk_diml << endl;
  cout << "    block of B (blk_dim_j*blk_diml): " << blk_dimj*blk_diml << endl;
  cout << "    block of C (blk_dim_i*blk_dimj): " << blk_dimi*blk_dimj << endl;
  cout << "Number of data blocks: " << endl;
  cout << "    number of blocks in A: " << blk_m << "x" << blk_k << "=" << blk_m*blk_k << endl;
  cout << "    number of blocks in B: " << blk_k << "x" << blk_n << "=" << blk_k*blk_n << endl;
  cout << "    number of blocks in C: " << blk_m << "x" << blk_n << "=" << blk_m*blk_n << endl;

  cout << endl << "Second blocking level: " << endl;
  cout << "Sub-block dimension: " << endl;
  cout << "    subblk_dim_i: " << subblk_dimi << endl;
  cout << "    subblk_dim_j: " << subblk_dimj << endl;
  cout << "    subblk_dim_l: " << subblk_diml << endl;

  cout << endl << "Compute block parameters: " << endl;
  cout << "Number of stages: " << subblk_diml << endl;
  cout << "Parallelization: " << subblk_dimi*subblk_dimj << endl;

  cout << endl << "Loop order: " << endl;
  cout << loop_order.loop_idc[0] << "->" << loop_order.loop_idc[1] << "->" << loop_order.loop_idc[2] << endl;

  cout << endl << "Estimated number of spills: " << num_spill << endl;
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


unsigned long long OptiMacroNode::spill(LoopOrder& loop_order){
  unsigned long long spill1 = spill_type1(loop_order.dblk1_size, loop_order.num_dblk1_mem, loop_order.num_dblk1_need, loop_order.num_reuse1, loop_order.num_iterate);
  unsigned long long spill2 = spill_type2(loop_order.dblk2_size, loop_order.num_dblk2_mem, loop_order.num_dblk2_need, loop_order.num_reuse2);
  
  unsigned long long spill = spill1+spill2;
  return spill;
}

unsigned long long OptiMacroNode::spill_type1(int blk_size, int num_dblk_mem, int num_dblk_need, int num_reuse, int iterate){
  unsigned long long spill;

  if(num_dblk_mem >= num_dblk_need){
    spill = 0;
  }
  else{
    spill = blk_size*(num_dblk_need+1-num_dblk_mem);
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
    spill = blk_size*(num_dblk_need+1-num_dblk_mem);
    spill = spill*(num_reuse-1);
  }
  return spill;
}

unsigned long long OptiMacroNode::getPerf(){
  unsigned long long perf;

  int num_subblk_level = blk_dimi_sb*blk_dimj_sb;
  int latency_cb = (subblk_diml-1)+3+1+2;

  int perf_blk = max(num_subblk_level, latency_cb)*blk_diml_sb+min(num_subblk_level,latency_cb)-1;
  
  perf = perf_blk*blk_m*blk_n*blk_k;
  
  return perf;
}

void OptiMacroNode::optiPara(){
  num_spill = ULLONG_MAX;
  perf = ULLONG_MAX;
  
  genSubblkSet();
  //cout << "subblk dim sets: " << endl;
  //for(auto &i: sb_dim_set){
  //  cout << i[0] << " " << i[1] << " " << i[2] << endl;
  //}

  for(auto &i: sb_dim_set){
	  subblk_dimi = i[0];
	  subblk_dimj = i[1];
	  subblk_diml = i[2];

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
	 	
		  unsigned long long cur_spill;
		  //find the smallest num_dblk
		  if((num_ablk_mem <= num_bblk_mem) && (num_ablk_mem <= num_cblk_mem)){
		    LoopOrder order1("m","k","n");
		    LoopOrder order2("k","m","n");
		    order1.setupLoopOrder(this);
		    order2.setupLoopOrder(this);

		    unsigned long long spill1 = spill(order1);
		    unsigned long long spill2 = spill(order2);

		    if(spill1 <= spill2){
		      cur_spill = spill1;
		      loop_order = order1;
		    }
		    else{
		      cur_spill = spill2;
		      loop_order = order2;
		    }
		  }
		  else if((num_bblk_mem <= num_ablk_mem) && (num_bblk_mem <= num_cblk_mem)){
		    LoopOrder order1("n","k","m");
		    LoopOrder order2("k","n","m");
		    order1.setupLoopOrder(this);
		    order2.setupLoopOrder(this);

		    unsigned long long spill1 = spill(order1);
		    unsigned long long spill2 = spill(order2);

		    if(spill1 <= spill2){
		      cur_spill = spill1;
		      loop_order = order1;
		    }
		    else{
		      cur_spill = spill2;
		      loop_order = order2;
		    }
		  }
		  else{
		    LoopOrder order1("m","n","k");
		    LoopOrder order2("n","m","k");
		    order1.setupLoopOrder(this);
		    order2.setupLoopOrder(this);

		    unsigned long long spill1 = spill(order1);
		    unsigned long long spill2 = spill(order2);

		    if(spill1 <= spill2){
		      cur_spill = spill1;
		      loop_order = order1;
		    }
		    else{
		      cur_spill = spill2;
		      loop_order = order2;
		    }
		  }

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
	        
		    opti_para.blk_m = blk_m;
		    opti_para.blk_n = blk_n;
		    opti_para.blk_k = blk_k;

		    opti_para.loop_order = loop_order;

		    opti_para.num_spill = num_spill;
		  }

		}
	      }
	    }
	  }
  }

  opti_para.k_stage = opti_para.subblk_diml;
  opti_para.num_cb = opti_para.subblk_dimi*opti_para.subblk_dimj;
  //cout << "num spill " << num_spill << endl;
  //cout << "perf " << perf << endl;
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



