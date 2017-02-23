#include<iostream>
#include<vector>
#include<memory>

#include "CGScheduling.hpp"
#include "MacroNode.hpp"
#include "Pattern.hpp"
#include "Memory.hpp"
#include "OptiMacroNode.hpp"

using namespace std;

CGScheduling::CGScheduling(MemoryTrack& in_mem):mem(in_mem){
}

CGScheduling::~CGScheduling(){
}

void CGScheduling::genDblks(string mtx_name, int m, int n, int blk_dimi, int blk_dimj){
  int blk_m = m/blk_dimi;
  int blk_n = n/blk_dimj;

  for(int blk_i=0; blk_i<blk_m; blk_i++){
    for(int blk_j=0; blk_j<blk_n; blk_j++){
      DataBlock db(dblks.size(), mtx_name, blk_dimi, blk_dimj, blk_i*blk_dimi, blk_j*blk_dimj);
      dblks.push_back(db);
    }
  }
}

void CGScheduling::MacroNodeGen(){
  //only for matrix multiplication
  int dblks_base_a = dblks.size();
  genDblks("A", opti_para.m_ex, opti_para.k_ex, opti_para.blk_dimi, opti_para.blk_diml); 
  int dblks_base_b = dblks.size();
  genDblks("B", opti_para.k_ex, opti_para.n_ex, opti_para.blk_diml, opti_para.blk_dimj); 
  int dblks_base_c = dblks.size();
  genDblks("C", opti_para.m_ex, opti_para.n_ex, opti_para.blk_dimi, opti_para.blk_dimj); 

  //generate mns
  MacroNodeTemplate* mn_temp = new MacroNodeTemplate("mtxmul temp");
  mn_temp->MN_mtxmul(opti_para.blk_dimi, opti_para.blk_dimj, opti_para.blk_diml, true);
  mn_temps.push_back(mn_temp);

  cout << "dblks base a " << dblks_base_a << endl; 
  cout << "dblks base b " << dblks_base_b << endl; 
  cout << "dblks base c " << dblks_base_c << endl; 

  num_cycles = 0;
  int blk_m = opti_para.m_ex/opti_para.blk_dimi;
  int blk_n = opti_para.n_ex/opti_para.blk_dimj;
  int blk_k = opti_para.k_ex/opti_para.blk_diml;
  for(int blk_i=0; blk_i<blk_m; blk_i++){
    for(int blk_j=0; blk_j<blk_n; blk_j++){
      for(int blk_l=0; blk_l<blk_k; blk_l++){
        //new mn
	cout << "mn " << mns.size() << endl;
	cout << "pred blks ";
	MacroNode mn(mn_temp, mns.size());

	vector<load_dblk> lds;
	vector<store_dblk> sds;
	//a blk: dblks_base_a+blk_i*blk_k+blk_l
	int ablk_idx = dblks_base_a+blk_i*blk_k+blk_l;
	int a_mem_addr;
	if(blk_l >= mem.num_region_a-1){
	  a_mem_addr = mem.getBase_a(mem.num_region_a-1);
	}
	else{
	  a_mem_addr = mem.getBase_a(blk_l);
	}
	if((blk_l >= mem.num_region_a-1) || (blk_j == 0)){
	  load_dblk ld;
	  ld.dblk_idx = ablk_idx;
	  ld.mem_base = a_mem_addr;
	  lds.push_back(ld);
	}
 	mn.a_base = a_mem_addr;
	mn.pred_dblks.push_back(ablk_idx);
	cout << *mn.pred_dblks.rbegin() << " ";

	//b blk: dblks_base_b+blk_l*blk_n+blk_j
	int bblk_idx = dblks_base_b+blk_l*blk_n+blk_j;
	int b_mem_addr;
	if(blk_j*blk_k+blk_l >= mem.num_region_b-1){
	  b_mem_addr = mem.getBase_b(mem.num_region_b-1);
	}
	else{
	  b_mem_addr = mem.getBase_b(blk_j*blk_k+blk_l);
	}
	if((blk_j*blk_k+blk_l >= mem.num_region_b-1) || (blk_i == 0)){
	  load_dblk ld;
	  ld.dblk_idx = bblk_idx;
	  ld.mem_base = b_mem_addr;
	  lds.push_back(ld);
	}
	mn.b_base = b_mem_addr;
	mn.pred_dblks.push_back(bblk_idx);
	cout << *mn.pred_dblks.rbegin() << " ";

	//c blk:
	int cblk_idx = dblks_base_c+blk_i*blk_n+blk_j;
	int c_mem_addr = mem.getBase_c(0);
	if(blk_l == 0){
	  load_dblk ld;
	  ld.dblk_idx = cblk_idx;
	  ld.mem_base = c_mem_addr;
	  lds.push_back(ld);
	}
	mn.c_base = c_mem_addr;
	mn.pred_dblks.push_back(cblk_idx);
	cout << *mn.pred_dblks.rbegin() << " " << endl;

	if(blk_l == blk_k-1){
	  store_dblk sd;
	  sd.dblk_idx = cblk_idx;
	  sd.mem_base = c_mem_addr;
	  sds.push_back(sd);
	}
	mn.post_dblks.push_back(cblk_idx);

	mns.push_back(mn);
	load.push_back(lds);
	store.push_back(sds);

      }
    }
  }
}

void CGScheduling::PrintPerf(){
  cout << "number of cycles: " << num_cycles << endl;
 // mem->getMaxNumLive();
}


