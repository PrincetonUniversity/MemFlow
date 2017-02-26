#include<iostream>
#include<vector>
#include<memory>

#include "CGScheduling.hpp"
#include "MacroNode.hpp"
#include "Pattern.hpp"
#include "Memory.hpp"
#include "OptiMacroNode.hpp"

using namespace std;

CGScheduling::CGScheduling():mem(*global_sp){
}

CGScheduling::~CGScheduling(){
}

void CGScheduling::genDblks(string mtx_name, int m, int n, int blk_dimi, int blk_dimj){
  int blk_m = m/blk_dimi;
  int blk_n = n/blk_dimj;

  for(int blk_i=0; blk_i<blk_m; blk_i++){
    for(int blk_j=0; blk_j<blk_n; blk_j++){
      DataBlock db(dblks.size(), mtx_name, blk_dimi, blk_dimj, blk_i, blk_j);
      db.setSPAddr();
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
	MacroNode mn(mn_temp, mns.size());

	LoadDblk ld;
	StoreDblk sd;
	//a blk: dblks_base_a+blk_i*blk_k+blk_l
	int ablk_idx = dblks_base_a+blk_i*blk_k+blk_l;
	if((blk_l >= mem.num_region_a-1) || (blk_j == 0)){
	  ld.dblk_idx.push_back(ablk_idx);
	  cout << "load a block" << endl;
	}
 	mn.a_sp_addr = dblks[ablk_idx].sp_addr;
	mn.pred_dblks.push_back(ablk_idx);

	//b blk: dblks_base_b+blk_l*blk_n+blk_j
	int bblk_idx = dblks_base_b+blk_l*blk_n+blk_j;
	if((blk_j*blk_k+blk_l >= mem.num_region_b-1) || (blk_i == 0)){
	  ld.dblk_idx.push_back(bblk_idx);
	  cout << "load b block" << endl;
	}
	mn.b_sp_addr = dblks[bblk_idx].sp_addr;
	mn.pred_dblks.push_back(bblk_idx);

	//c blk:
	int cblk_idx = dblks_base_c+blk_i*blk_n+blk_j;
	if(blk_l == 0){
	  ld.dblk_idx.push_back(cblk_idx);
	  cout << "load c block" << endl;
	}
	mn.c_sp_addr = dblks[cblk_idx].sp_addr;
	mn.pred_dblks.push_back(cblk_idx);

	num_cycles += ld.getLatency();
	
	//mn computation
	mns.push_back(mn);
	cout << "executing mn" << endl;
	num_cycles += mn.mn_temp->cycle_length;
	
	//store cblk if needed
	if(blk_l == blk_k-1){
	  sd.dblk_idx.push_back(cblk_idx);
	  cout << "store c block" << endl;
	}
	mn.post_dblks.push_back(cblk_idx);

	num_cycles += sd.getLatency();

	load.push_back(ld);
	store.push_back(sd);

      }
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
	MacroNode mn(mn_temp, mns.size());

	LoadDblk ld;
	StoreDblk sd;
	//a blk: dblks_base_a+blk_i*blk_k+blk_l
	int ablk_idx = dblks_base_a+blk_i*blk_k+blk_l;
	if((blk_l >= mem.num_region_a-1) || (blk_j == 0)){
	  ld.dblk_idx.push_back(ablk_idx);
	  cout << "load a block" << endl;
	}
 	mn.a_sp_addr = dblks[ablk_idx].sp_addr;
	mn.pred_dblks.push_back(ablk_idx);

	//b blk: dblks_base_b+blk_l*blk_n+blk_j
	int bblk_idx = dblks_base_b+blk_l*blk_n+blk_j;
	if((blk_j*blk_k+blk_l >= mem.num_region_b-1) || (blk_i == 0)){
	  ld.dblk_idx.push_back(bblk_idx);
	  cout << "load b block" << endl;
	}
	mn.b_sp_addr = dblks[bblk_idx].sp_addr;
	mn.pred_dblks.push_back(bblk_idx);

	//c blk:
	int cblk_idx = dblks_base_c+blk_i*blk_n+blk_j;
	if(blk_l == 0){
	  ld.dblk_idx.push_back(cblk_idx);
	  cout << "load c block" << endl;
	}
	mn.c_sp_addr = dblks[cblk_idx].sp_addr;
	mn.pred_dblks.push_back(cblk_idx);

	num_cycles += ld.getLatency();
	
	//mn computation
	mns.push_back(mn);
	cout << "executing mn" << endl;
	num_cycles += mn.mn_temp->cycle_length;
	
	//store cblk if needed
	if(blk_l == blk_k-1){
	  sd.dblk_idx.push_back(cblk_idx);
	  cout << "store c block" << endl;
	}
	mn.post_dblks.push_back(cblk_idx);

	num_cycles += sd.getLatency();

	load.push_back(ld);
	store.push_back(sd);

      }
    }
  }
}

void CGScheduling::PrintPerf(){
  cout << "number of cycles: " << num_cycles << endl;
 // mem->getMaxNumLive();
}


