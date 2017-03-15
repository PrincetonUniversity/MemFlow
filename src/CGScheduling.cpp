#include<iostream>
#include<vector>
#include<memory>
#include<array>
#include<string>

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

void CGScheduling::LoopIteration(int blk_i, int blk_j, int blk_l){
    //new mn
    //cout << endl;
    //cout << "blk i " << blk_i << endl;
    //cout << "blk j " << blk_j << endl;
    //cout << "blk l " << blk_l << endl;
    //cout << "mn " << mns.size() << endl;
    MacroNode mn(mn_temp, mns.size());

    //a blk: dblks_base_a+blk_i*blk_k+blk_l
    int ablk_idx = blk_i*blk_k+blk_l;
    DataBlock* ablk = &data_arrays["A"]->dblks[ablk_idx];

    int first_fetch_ind = (opti_para.loop_order.loop_idc[2] == "k")?blk_j:blk_l;
    if((ablk->AddrIdx() > mem.sp_regions["A"].num_blks-1) || (first_fetch_ind == 0)){
      if(load_latency.find("A") == load_latency.end()){
	cout << "first load " << endl;
        load_latency["A"] = load_store->loadDblk(ablk);
      }
      cycle += load_latency["A"];
      num_dram_read += ablk->size;
    }
    mn.a_sp_addr = ablk->sp_addr;
    mn.pred_dblks.push_back(ablk_idx);

    //b blk: dblks_base_b+blk_l*blk_n+blk_j
    int bblk_idx = blk_l*blk_n+blk_j;
    DataBlock* bblk = &data_arrays["B"]->dblks[bblk_idx];

    first_fetch_ind = (opti_para.loop_order.loop_idc[2] == "k")?blk_i:blk_l;
    if((bblk->AddrIdx() > mem.sp_regions["B"].num_blks-1) || (first_fetch_ind == 0)){
      //cout << "load b block" << endl;
      if(load_latency.find("B") == load_latency.end()){
	cout << "need to load B block " << endl;
        load_latency["B"] = load_store->loadDblk(bblk);
      }
      cycle += load_latency["B"];
      num_dram_read += bblk->size;
    }
    mn.b_sp_addr = bblk->sp_addr;
    mn.pred_dblks.push_back(bblk_idx);

    //c blk:
    int cblk_idx = blk_i*blk_n+blk_j;
    DataBlock* cblk = &data_arrays["C"]->dblks[cblk_idx];

    first_fetch_ind = (opti_para.loop_order.loop_idc[2] == "m")?blk_j:blk_i;
    if((cblk->AddrIdx() > mem.sp_regions["C"].num_blks-1) || (first_fetch_ind == 0)){
      //cout << "load c block" << endl;
      if(load_latency.find("C") == load_latency.end()){
	cout << "need to load C block " << endl; 
        load_latency["C"] = load_store->loadDblk(cblk);
      }
      cycle += load_latency["C"];
      num_dram_read += cblk->size;
    }
    mn.c_sp_addr = cblk->sp_addr;
    mn.pred_dblks.push_back(cblk_idx);

    //mn computation
    mns.push_back(mn);
    //cout << "executing mn" << endl;
    cycle += mn.mn_temp->cycle_length;
    num_compute_cycles += mn.mn_temp->cycle_length;
    
    //store cblk if needed
    if((cblk->AddrIdx() > mem.sp_regions["C"].num_blks-1) || (blk_l == blk_k-1)){
      //cout << "store c block" << endl;
      if(store_latency.find("C") == store_latency.end()){
	cout << "need to store C block " << endl; 
        store_latency["C"] = load_store->storeDblk(cblk);
      }
      cycle += store_latency["C"];
      num_dram_write += cblk->size;
    }
    mn.post_dblks.push_back(cblk_idx);

}

void CGScheduling::MacroNodeGen(){
  //only for matrix multiplication

  //generate mns
  mn_temp = new MN_mtxmul(opti_para.blk_dim["i"], opti_para.blk_dim["j"], opti_para.blk_dim["l"], true);
  mn_temps.push_back(mn_temp);

  //transaction 
  load_store = shared_ptr<LoadStoreDblk>(new LoadStoreDblk);
  //cout << "dblks base a " << dblks_base_a << endl; 
  //cout << "dblks base b " << dblks_base_b << endl; 
  //cout << "dblks base c " << dblks_base_c << endl; 

  num_compute_cycles = 0;
  cycle = 0;
  num_dram_read = 0;
  num_dram_write = 0;
  blk_m = opti_para.num_blk["i"];
  blk_n = opti_para.num_blk["j"];
  blk_k = opti_para.num_blk["l"];

  if(opti_para.loop_order.loop_idc == array<string,3>{"m","n","k"}){
    for(int blk_i=0; blk_i<blk_m; blk_i++){
      for(int blk_j=0; blk_j<blk_n; blk_j++){
	for(int blk_l=0; blk_l<blk_k; blk_l++){
	  LoopIteration(blk_i, blk_j, blk_l);
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"m","k","n"}){
    for(int blk_i=0; blk_i<blk_m; blk_i++){
      for(int blk_l=0; blk_l<blk_k; blk_l++){
	for(int blk_j=0; blk_j<blk_n; blk_j++){
	  LoopIteration(blk_i, blk_j, blk_l);
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"n","k","m"}){
    for(int blk_j=0; blk_j<blk_n; blk_j++){
      for(int blk_l=0; blk_l<blk_k; blk_l++){
	for(int blk_i=0; blk_i<blk_m; blk_i++){
	  LoopIteration(blk_i, blk_j, blk_l);
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"n","m","k"}){
    for(int blk_j=0; blk_j<blk_n; blk_j++){
      for(int blk_i=0; blk_i<blk_m; blk_i++){
	for(int blk_l=0; blk_l<blk_k; blk_l++){
	  LoopIteration(blk_i, blk_j, blk_l);
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"k","m","n"}){
    for(int blk_l=0; blk_l<blk_k; blk_l++){
      for(int blk_i=0; blk_i<blk_m; blk_i++){
	for(int blk_j=0; blk_j<blk_n; blk_j++){
	  LoopIteration(blk_i, blk_j, blk_l);
	}
      }
    }
  }
  else{
    for(int blk_l=0; blk_l<blk_k; blk_l++){
      for(int blk_j=0; blk_j<blk_n; blk_j++){
	for(int blk_i=0; blk_i<blk_m; blk_i++){
	  LoopIteration(blk_i, blk_j, blk_l);
	}
      }
    }
  }


  //cout << "num of dblk write " << num_dblk_write << endl;
}

void CGScheduling::PrintPerf(){
  cout << endl;
  cout << "Number of cycles: " << cycle << endl;
  cout << "Number of compute cycles: " << num_compute_cycles << endl;
  cout << "Number of dram read: " << num_dram_read << endl;
  cout << "Number of dram write: " << num_dram_write << endl;
 // mem->getMaxNumLive();
}


