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

void CGScheduling::setDblkUseTime(){
  for(auto &d: dblk_idx){
    data_arrays[d.first]->dblks[d.second].next_use_t.push(macro_time);
  }
}

void CGScheduling::prepareDblks(){
  cout << endl << "macro time " << macro_time << endl;
  cout << "blk i,j,l: " << blk_i << "," << blk_j << "," << blk_l << endl;
  for(auto &d: dblk_idx){
    int idx = d.second;
    DataBlock* dblk = &data_arrays[d.first]->dblks[idx];
    if((!dblk->next_use_t.empty()) && (dblk->next_use_t.front() == macro_time)){
      dblk->next_use_t.pop();
    }

    //all used dblks(no matter read or write) need to be allocated to sram
    int sp_base;
    if(!dblk->in_sp){
      //dblk can be read or write
      cout << d.first << "not in sp" << endl;
      //get sp addr
      if(global_sp->sp_regions[d.first].next_empty.empty()){
        sp_base = global_sp->sp_regions[d.first].next_base_2replace;

	int idx_replace = global_sp->sp_regions[d.first].dblk_idx[sp_base];
	DataBlock* dblk_2replace = &data_arrays[d.first]->dblks[idx_replace];
	
	cout << "   replace " << idx_replace << endl;
	num_dram_spill += dblk_2replace->size;
	cout << "   num spill " << num_dram_spill << endl;

	if(d.first == "L"){
	  num_spill_l += dblk_2replace->size;
	}
	else if(d.first == "U"){
	  num_spill_u += dblk_2replace->size;
	}
	else if(d.first == "A"){
	  num_spill_a += dblk_2replace->size;
	}

	//some case need to store
	dblk_2replace->in_sp = false;
	if(dblk_2replace->dirty){
	  //if(store_latency.find(d.first) == store_latency.end()){
	  //  store_latency[d.first] = load_store->storeDblk(dblk_2replace);
	  //}
	  //cycle += store_latency[d.first];
	  num_dram_write += dblk_2replace->size;
	  cout << "dram write " << d.first << endl;
	}
      }
      else{
        sp_base = global_sp->sp_regions[d.first].next_empty.front();
	global_sp->sp_regions[d.first].next_empty.pop();
      }

      cout << "   sp base " << sp_base << endl;
      //update info
      if(out_dblk.find(d.first) == out_dblk.end()){
	//read from dram
        dblk->dirty = false;
      }
      else{
        dblk->dirty = true;
      }
      dblk->in_sp = true;
      dblk->sp_addr = {d.first, sp_base};
      global_sp->sp_regions[d.first].dblk_idx[sp_base] = idx;
      global_sp->sp_regions[d.first].updateNextBase2Replace(sp_base);

      //load latency
      if(in_dblk.find(d.first) != in_dblk.end()){
	//if(load_latency.find(d.first) == load_latency.end()){
	  //load_latency[d.first] = load_store->loadDblk(dblk);
	//}
	num_dram_read += dblk->size;
	//cycle += load_latency[d.first];
      }
      //end case: not in sp
      num_sram_update += dblk->size;
    }
    else{
      sp_base = dblk->sp_addr.base;
      global_sp->sp_regions[d.first].updateNextBase2Replace(sp_base);
      if(out_dblk.find(d.first) != out_dblk.end()){
        dblk->dirty = true;
      }
    }

    //no future use, remvoe
    if(dblk->next_use_t.empty()){
      dblk->in_sp = false;
      global_sp->sp_regions[d.first].next_empty.push(dblk->sp_addr.base);
      //if(out_dblk.find(d.first) != out_dblk.end()){
	//if(store_latency.find(d.first) == store_latency.end()){
	  //store_latency[d.first] = load_store->storeDblk(dblk);
	//}
      if(kernal_out_dblk.find(d.first) != kernal_out_dblk.end()){
	num_dram_write += dblk->size;
	cout << "dram write " << d.first << endl;
	//cycle += store_latency[d.first];
      }
    }
  }
}

void CGScheduling::runMacroNode_MM(){

  //dblk need to read
  in_dblk.clear();
  in_dblk.insert("A");
  in_dblk.insert("B");
  in_dblk.insert("C");

  out_dblk.clear();
  out_dblk.insert("C");
  
  prepareDblks();

  //mn
  MacroNode mn(mn_temp["mm"], mns.size());
  cycle += mn.mn_temp->cycle_length;
  total_use += mn.mn_temp->total_use;
  use_sram += mn.mn_temp->use_sram;
  use_pipe += mn.mn_temp->use_pipe;
  num_compute_cycles += mn.mn_temp->cycle_length;
  mns.push_back(mn);
}

void CGScheduling::setDblkIdx_MM(){
  dblk_idx.clear();
  dblk_idx["A"] = blk_i*opti_para.axes["l"].num_blk+blk_l;
  dblk_idx["B"] = blk_l*opti_para.axes["j"].num_blk+blk_j;
  dblk_idx["C"] = blk_i*opti_para.axes["j"].num_blk+blk_j;
  
  //do something
  (this->*func_iteration)();

  macro_time++;
}

void CGScheduling::LoopStructure_MM(){
  blk_m = opti_para.axes["i"].num_blk;
  blk_n = opti_para.axes["j"].num_blk;
  blk_k = opti_para.axes["l"].num_blk;

  macro_time = 0;
  if(opti_para.loop_order.loop_idc == array<string,3>{"i","j","l"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_j=0; blk_j<blk_n; blk_j++){
	for(blk_l=0; blk_l<blk_k; blk_l++){
	  setDblkIdx_MM();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"i","l","j"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_l=0; blk_l<blk_k; blk_l++){
	for(blk_j=0; blk_j<blk_n; blk_j++){
	  setDblkIdx_MM();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","l","i"}){
    for(blk_j=0; blk_j<blk_n; blk_j++){
      for(blk_l=0; blk_l<blk_k; blk_l++){
	for(blk_i=0; blk_i<blk_m; blk_i++){
	  setDblkIdx_MM();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","i","l"}){
    for(blk_j=0; blk_j<blk_n; blk_j++){
      for(blk_i=0; blk_i<blk_m; blk_i++){
	for(blk_l=0; blk_l<blk_k; blk_l++){
	  setDblkIdx_MM();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"l","i","j"}){
    for(blk_l=0; blk_l<blk_k; blk_l++){
      for(blk_i=0; blk_i<blk_m; blk_i++){
	for(blk_j=0; blk_j<blk_n; blk_j++){
	  setDblkIdx_MM();
	}
      }
    }
  }
  else{
    for(blk_l=0; blk_l<blk_k; blk_l++){
      for(blk_j=0; blk_j<blk_n; blk_j++){
	for(blk_i=0; blk_i<blk_m; blk_i++){
	  setDblkIdx_MM();
	}
      }
    }
  }

}

void CGScheduling::MacroNodeGen_MM(){
  //generate mns

  kernal_out_dblk.insert("C");

  int blk_m = opti_para.axes["i"].blk_dim;
  int blk_n = opti_para.axes["j"].blk_dim;
  int blk_k = opti_para.axes["l"].blk_dim;
  
  map<string, dblk_info> dblks_info;
  dblks_info["A"] = {"A","A","mode_mm_a",opti_para.axes["i"].subblk_dim, opti_para.axes["l"].subblk_dim, blk_m,blk_k};
  dblks_info["B"] = {"B","B","mode_mm_b",opti_para.axes["l"].subblk_dim, opti_para.axes["j"].subblk_dim, blk_k,blk_n};
  dblks_info["C"] = {"C","C","mode_mm_c",opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, blk_m,blk_n};

  mn_temp["mm"] = new MN_mtxmul(true, blk_m, blk_n, blk_k, dblks_info);
  mn_temps.push_back(mn_temp["mm"]);

  //transaction 
  load_store = shared_ptr<LoadStoreDblk>(new LoadStoreDblk);

  num_compute_cycles = 0;
  cycle = 0;
  total_use = 0;
  use_sram = 0;
  use_pipe = 0;
  num_dram_read = 0;
  num_dram_spill = 0;
  num_dram_write = 0;
  num_sram_update = 0;
  num_blk_replaced_a = 0;
  num_blk_replaced_b = 0;
  num_blk_replaced_c = 0;
 
  func_iteration = &CGScheduling::setDblkUseTime;
  LoopStructure_MM();

  func_iteration = &CGScheduling::runMacroNode_MM;
  LoopStructure_MM();
}



void CGScheduling::runMacroNode_LU(){

  in_dblk.clear();
  out_dblk.clear();

  string mn_temp_name;

  if((blk_i == blk_l) && (blk_j == blk_l)){
    //dblk need to read
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
  cycle += mn.mn_temp->cycle_length;
  num_compute_cycles += mn.mn_temp->cycle_length;
  total_use += mn.mn_temp->total_use;
  use_sram += mn.mn_temp->use_sram;
  use_pipe += mn.mn_temp->use_pipe;
  mns.push_back(mn);
}

void CGScheduling::setDblkIdx_LU(){
  dblk_idx.clear();
  dblk_idx["L"] = blk_i*opti_para.axes["i"].num_blk+blk_l;
  dblk_idx["U"] = blk_l*opti_para.axes["i"].num_blk+blk_j;
  dblk_idx["A"] = blk_i*opti_para.axes["i"].num_blk+blk_j;
  
  //do something
  (this->*func_iteration)();

  macro_time++;
}

void CGScheduling::LoopStructure_LU(){
  blk_m = opti_para.axes["i"].num_blk;

  macro_time = 0;
  if(opti_para.loop_order.loop_idc == array<string,3>{"i","j","l"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_j=0; blk_j<blk_m; blk_j++){
	for(blk_l=0; blk_l<=min(blk_i,blk_j); blk_l++){
	  setDblkIdx_LU();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"i","l","j"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_l=0; blk_l<=blk_i; blk_l++){
	for(blk_j=blk_l; blk_j<blk_m; blk_j++){
	  setDblkIdx_LU();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","l","i"}){
    for(blk_j=0; blk_j<blk_m; blk_j++){
      for(blk_l=0; blk_l<=blk_j; blk_l++){
	for(blk_i=blk_l; blk_i<blk_m; blk_i++){
	  setDblkIdx_LU();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","i","l"}){

    for(blk_j=0; blk_j<blk_m; blk_j++){
      for(blk_i=0; blk_i<blk_m; blk_i++){
	  for(blk_l=0; blk_l<=min(blk_i,blk_j); blk_l++){
	    setDblkIdx_LU();
	  }
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"l","i","j"}){
    for(blk_l=0; blk_l<blk_m; blk_l++){
      for(blk_i=blk_l; blk_i<blk_m; blk_i++){
	for(blk_j=blk_l; blk_j<blk_m; blk_j++){
	  setDblkIdx_LU();
	}
      }
    }
  }
  else{
    for(blk_l=0; blk_l<blk_m; blk_l++){
      for(blk_j=blk_l; blk_j<blk_m; blk_j++){
	for(blk_i=blk_l; blk_i<blk_m; blk_i++){
	  setDblkIdx_LU();
	}
      }
    }
  }
}

void CGScheduling::MacroNodeGen_LU(){
  int blk_m = opti_para.axes["i"].blk_dim;

  kernal_out_dblk.insert("L");
  kernal_out_dblk.insert("U");
  //generate mns
  map<string, dblk_info> dblks_info_lu;
  dblks_info_lu["L"] = {"LU_L_Tr","L","mode_lu_ltr",opti_para.axes["i"].subblk_dim,0,blk_m,blk_m};
  dblks_info_lu["U"] = {"LU_U_Tr","U","mode_lu_utr",0,opti_para.axes["j"].subblk_dim,blk_m,blk_m};
  dblks_info_lu["A"] = {"LU_A","A","mode_lu_a",opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim,blk_m,blk_m};
  mn_temp["lu"] = new MN_LU(blk_m, dblks_info_lu);
  mn_temps.push_back(mn_temp["lu"]);

  map<string, dblk_info> dblks_info_lucpl;
  dblks_info_lucpl["A"] = {"LUCPL_A","A","mode_lu_a",opti_para.axes["i"].subblk_dim,opti_para.axes["j"].subblk_dim,blk_m,blk_m};
  dblks_info_lucpl["U"] = {"LUCPL_U_Tr","U","mode_lu_utr",0,opti_para.axes["j"].subblk_dim,blk_m,blk_m};
  dblks_info_lucpl["L"] = {"LUCPL_L", "L", "mode_lu_l", opti_para.axes["i"].subblk_dim,0,blk_m,blk_m};
  mn_temp["lucpl"] = new MN_LUCPL(blk_m, dblks_info_lucpl);
  mn_temps.push_back(mn_temp["lucpl"]);

  map<string, dblk_info> dblks_info_trs;
  dblks_info_trs["A"] = {"TRS_A_Tr","L","mode_lu_ltr",opti_para.axes["i"].subblk_dim,0,blk_m,blk_m};
  dblks_info_trs["B"] = {"TRS_B","A","mode_lu_a",opti_para.axes["i"].subblk_dim,opti_para.axes["j"].subblk_dim,blk_m,blk_m};
  dblks_info_trs["X"] = {"TRS_X","U","mode_lu_u",0,opti_para.axes["j"].subblk_dim,blk_m,blk_m};
  dblks_info_trs["Brow"] = {"TRS_Brow","U","mode_lu_u",0,opti_para.axes["j"].subblk_dim,1,blk_m};
  mn_temp["trs"] = new MN_TRS(blk_m, dblks_info_trs);
  mn_temps.push_back(mn_temp["trs"]);
  
  map<string, dblk_info> dblks_info_mm;
  dblks_info_mm["A"] = {"A","L","mode_mm_a",opti_para.axes["i"].subblk_dim, opti_para.axes["l"].subblk_dim, blk_m,blk_m};
  dblks_info_mm["B"] = {"B","U","mode_mm_b",opti_para.axes["l"].subblk_dim, opti_para.axes["j"].subblk_dim, blk_m,blk_m};
  dblks_info_mm["C"] = {"C","A","mode_mm_c",opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, blk_m,blk_m};
  mn_temp["submm"] = new MN_SUBMM(blk_m, blk_m, blk_m, dblks_info_mm);
  mn_temps.push_back(mn_temp["submm"]);

  load_store = shared_ptr<LoadStoreDblk>(new LoadStoreDblk);

  num_compute_cycles = 0;
  cycle = 0;
  num_dram_read = 0;
  num_dram_write = 0;
  num_dram_spill = 0;
  num_sram_update = 0;
  num_spill_u = 0;
  num_spill_l = 0;
  num_spill_a = 0;

  func_iteration = &CGScheduling::setDblkUseTime;
  LoopStructure_LU();

  func_iteration = &CGScheduling::runMacroNode_LU;
  LoopStructure_LU();
}



void CGScheduling::runMacroNode_QR(){

  in_dblk.clear();
  out_dblk.clear();

  string mn_temp_name;

  if((blk_i == blk_l) && (blk_j == blk_l)){
    //dblk need to read
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
  cycle += mn.mn_temp->cycle_length;
  num_compute_cycles += mn.mn_temp->cycle_length;
  total_use += mn.mn_temp->total_use;
  use_sram += mn.mn_temp->use_sram;
  use_pipe += mn.mn_temp->use_pipe;
  mns.push_back(mn);
}

void CGScheduling::setDblkIdx_QR(){
  dblk_idx.clear();
  dblk_idx["Q"] = blk_i*opti_para.axes["i"].num_blk+blk_l;
  dblk_idx["R"] = blk_l*opti_para.axes["i"].num_blk+blk_j;
  dblk_idx["A"] = blk_i*opti_para.axes["i"].num_blk+blk_j;
  
  //do something
  (this->*func_iteration)();

  macro_time++;
}

void CGScheduling::LoopStructure_QR(){
  blk_m = opti_para.axes["i"].num_blk;

  macro_time = 0;
  if(opti_para.loop_order.loop_idc == array<string,3>{"i","j","l"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_j=0; blk_j<blk_m; blk_j++){
	for(blk_l=0; blk_l<=min(blk_i,blk_j); blk_l++){
	  setDblkIdx_QR();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"i","l","j"}){
    for(blk_i=0; blk_i<blk_m; blk_i++){
      for(blk_l=0; blk_l<=blk_i; blk_l++){
	for(blk_j=blk_l; blk_j<blk_m; blk_j++){
	  setDblkIdx_QR();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","l","i"}){
    for(blk_j=0; blk_j<blk_m; blk_j++){
      for(blk_l=0; blk_l<=blk_j; blk_l++){
	for(blk_i=blk_l; blk_i<blk_m; blk_i++){
	  setDblkIdx_QR();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"j","i","l"}){
    for(blk_j=0; blk_j<blk_m; blk_j++){
      for(blk_i=0; blk_i<blk_m; blk_i++){
	for(blk_l=0; blk_l<=min(blk_i,blk_j); blk_l++){
	  setDblkIdx_QR();
	}
      }
    }
  }
  else if(opti_para.loop_order.loop_idc == array<string,3>{"l","i","j"}){
    for(blk_l=0; blk_l<blk_m; blk_l++){
      for(blk_i=blk_l; blk_i<blk_m; blk_i++){
	for(blk_j=blk_l; blk_j<blk_m; blk_j++){
	  setDblkIdx_QR();
	}
      }
    }
  }
  else{
    for(blk_l=0; blk_l<blk_m; blk_l++){
      for(blk_j=blk_l; blk_j<blk_m; blk_j++){
	for(blk_i=blk_l; blk_i<blk_m; blk_i++){
	  setDblkIdx_QR();
	}
      }
    }
  }
}

void CGScheduling::MacroNodeGen_QR(){
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

  cout << "creat all mn temp " << endl;

  num_compute_cycles = 0;
  cycle = 0;
  num_dram_read = 0;
  num_dram_write = 0;
  num_dram_spill = 0;
  num_sram_update = 0;


  func_iteration = &CGScheduling::setDblkUseTime;
  LoopStructure_QR();

  cout << "set dblk use" << endl;

  func_iteration = &CGScheduling::runMacroNode_QR;
  LoopStructure_QR();
}



void CGScheduling::MacroNodeGen(){
  if(app == "MM"){
    MacroNodeGen_MM();
  }
  else if(app == "LU"){
    MacroNodeGen_LU();
  }
  else if(app == "QR"){
    MacroNodeGen_QR();
  }
}

void CGScheduling::PrintPerf(){
  cout << endl;
  cout << "Number of cycles: " << cycle << endl;
  cout << "Number of compute cycles: " << num_compute_cycles << endl;
  cout << "Number of total use: " << total_use << endl;
  cout << "Number of use from sram: " << use_sram << endl;
  cout << "Number of use from pipe: " << use_pipe << endl;
  cout << "Number of dram read: " << num_dram_read << endl;
  cout << "Number of dram write: " << num_dram_write << endl;
  cout << "Number of dram spill: " << num_dram_spill << endl;
  cout << "Number of spill u : " << num_spill_u << endl;
  cout << "Number of spill l : " << num_spill_l << endl;
  cout << "Number of spill a : " << num_spill_a << endl;
  cout << "Number of sram update: " << num_sram_update << endl;
 // mem->getMaxNumLive();
}


