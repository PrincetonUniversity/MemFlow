#include "Workload.hpp"

Workload::Workload(string app_name, int in_m, int in_n, int in_k){
  name = app_name;
  m = in_m;
  n = in_n;
  k = in_k;
  
  para.axes["i"].input_size = m;
  para.axes["j"].input_size = n;
  para.axes["l"].input_size = k;
  //opti = shared_ptr<OptiMacroNode>(new OptiMacroNode(this, opti_para));
}

void Workload::setupDataInfo(){
  //if(name == "MM"){
  //  datas_info.push_back(data_info("A", opti_para.axes["i"].ex_input_size, opti_para.axes["l"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["l"].blk_dim));
  //  datas_info.push_back(data_info("B", opti_para.axes["l"].ex_input_size, opti_para.axes["j"].ex_input_size, opti_para.axes["l"].blk_dim, opti_para.axes["j"].blk_dim));
  //  datas_info.push_back(data_info("C", opti_para.axes["i"].ex_input_size, opti_para.axes["j"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["j"].blk_dim));
  //}
  //if(name == "LU"){
  //  datas_info.push_back(data_info("L", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  //  datas_info.push_back(data_info("U", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  //  datas_info.push_back(data_info("A", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  //}
  //else if(name == "QR"){
   // datas_info.push_back(data_info("Q", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
    //datas_info.push_back(data_info("R", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
    //datas_info.push_back(data_info("A", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  //}
}

void Workload::setup(){
  //generate optimized design para
  //aopti->optiPara(this);
  optiPara();

  opti_para.PrintInfo();

  //slice DRAM
  //global_sp->getOptiPara(&opti_para);
  global_sp->Slice2Dblks();
  global_sp->PrintInfo();
  
  setupDataInfo();
  //initialize data arrays
  int start_addr = 0;
  for(auto &data: datas_info){
    DataArray* da = new DataArray(data.name, data.dimi, data.dimj);
    //set dram addr mode
    da->setAddrMappingMode(data.dimi, data.dimj, false, false);
    da->setStartAddr(start_addr);
    start_addr += da->size;

    data_arrays[data.name] = da;
  }

  for(auto &data: datas_info){
    data_arrays[data.name]->genDblks(data.blk_dimi, data.blk_dimj);
  }

  //build cbs
  //for(auto &cb: opti_para.cbs){
  //  ComputeBlockLib::cbs[cb.first]["load"] = iniCBWrapper("load",1,1);
  //  ComputeBlockLib::cbs[cb.first]["store"] = iniCBWrapper("store",1,1);
  //  for(auto &mn_cb: cb.second){
  //    ComputeBlockLib::cbs[cb.first][mn_cb.first] = iniCBWrapper(mn_cb.first, mn_cb.second.num_cb, mn_cb.second.k_stage);
  //  }
  //}
}

void Workload::run(){
  //CGScheduling mn_sche;
  //mn_sche.MacroNodeGen();
  //mn_sche.PrintPerf();
  genMacroNode();
  printPerf();
  //MacroNodeTemplate* mn_temp = new MN_LUCPL(opti_para.blk_dim[app+"i"]);
  //mn_temps.push_back(mn_temp);
}

void Workload::test_mntemp(){
  //OptiMacroNode opti(m,n,k,opti_para);
  //opti.setPara();
  //opti.optiPara_kernel();
  //opti_para.PrintInfo();
 
/*
  global_sp->getOptiPara(&opti_para);
  global_sp->Slice2Dblks();
  global_sp->PrintInfo();
 
  for(auto &cb: opti_para.cbs){
    ComputeBlockLib::cbs[cb.first]["load"] = iniCBWrapper("load",1,1,1);
    ComputeBlockLib::cbs[cb.first]["store"] = iniCBWrapper("store",1,1,1);
    for(auto &mn_cb: cb.second){
      ComputeBlockLib::cbs[cb.first][mn_cb.first] = iniCBWrapper(mn_cb.first, mn_cb.second.num_cb, mn_cb.second.k_stage, mn_cb.second.width);
    }
  }

  map<string, dblk_info> dblks_info;
*/




  //dblks_info["Q"] = {"QR_Q_Tr","Q","mode_lu_ltr",opti_para.axes["i"].subblk_dim, 0, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["R"] = {"QR_R","R","mode_lu_u",0,opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["A"] = {"QR_A","A","mode_lu_a",opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};

  //MacroNodeTemplate* mn = new MN_QR(opti_para.axes["i"].blk_dim,dblks_info);
  //mn_temps.push_back(mn);
  
  //dblks_info["Q"] = {"QR_Q","Q","mode_lu_l",opti_para.axes["i"].subblk_dim, 0, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["T"] = {"QR_T","Q","mode_lu_u",0,opti_para.axes["i"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["R0"] = {"QR_R0_Tr","R","mode_lu_utr",0,opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["R"] = {"QR_R","R","mode_lu_u",0,opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["A"] = {"QR_A","A","mode_lu_a",opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};

  //MacroNodeTemplate* mn = new MN_QRCPL(opti_para.axes["i"].blk_dim,dblks_info);
  //mn_temps.push_back(mn);
  
  //dblks_info["T"] = {"QR_T","Q","mode_lu_l",opti_para.axes["i"].subblk_dim,0, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["A"] = {"QR_A","A","mode_lu_a",opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};

  //MacroNodeTemplate* mn = new MN_QRUpdateDiag(opti_para.axes["i"].blk_dim,opti_para.axes["i"].num_blk-1,dblks_info);
  //mn_temps.push_back(mn);

  //dblks_info["Q"] = {"QR_Q","Q","mode_lu_l",opti_para.axes["i"].subblk_dim,0, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["A"] = {"QR_A","A","mode_lu_u",opti_para.axes["i"].subblk_dim, opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};

  //MacroNodeTemplate* mn = new MN_QRUpdateDiag(opti_para.axes["i"].blk_dim,opti_para.axes["i"].num_blk-1,dblks_info);
  //mn_temps.push_back(mn);
  
  //dblks_info["Q"] = {"QR_Q_Tr","Q","mode_lu_ltr",opti_para.axes["i"].subblk_dim,0, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["A"] = {"QR_A","R","mode_lu_u",0,opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["Atemp"] = {"QR_Atemp","A","mode_lu_a",opti_para.axes["i"].subblk_dim,opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};

  //MacroNodeTemplate* mn = new MN_QRUpdateTr(opti_para.axes["i"].blk_dim,dblks_info);
  //mn_temps.push_back(mn);
  
  //dblks_info["Q"] = {"QR_Q_Tr","Q","mode_lu_l",opti_para.axes["i"].subblk_dim,0, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["A"] = {"QR_A","R","mode_lu_u",0,opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};
  //dblks_info["Atemp"] = {"QR_Atemp","A","mode_lu_a",opti_para.axes["i"].subblk_dim,opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim};

  //MacroNodeTemplate* mn = new MN_QRUpdate(opti_para.axes["i"].blk_dim,dblks_info);
  //mn_temps.push_back(mn);
  
 /*
  dblks_info["A"] = {"MM_A","A","mode_mm_a",opti_para.axes["i"].subblk_dim, opti_para.axes["l"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["l"].blk_dim};
  dblks_info["B"] = {"MM_B","B","mode_mm_b",opti_para.axes["l"].subblk_dim, opti_para.axes["j"].subblk_dim, opti_para.axes["l"].blk_dim, opti_para.axes["j"].blk_dim};
  dblks_info["C"] = {"MM_C","C","mode_mm_c",opti_para.axes["i"].subblk_dim,opti_para.axes["j"].subblk_dim, opti_para.axes["i"].blk_dim, opti_para.axes["j"].blk_dim};

  MacroNodeTemplate* mn = new MN_mtxmul(true, opti_para.axes["i"].blk_dim,opti_para.axes["j"].blk_dim,opti_para.axes["l"].blk_dim, dblks_info);
  mn_temps.push_back(mn);
 */

}

void Workload::setDblkUseTime(){
  for(auto &d: dblk_idx){
    data_arrays[d.first]->dblks[d.second].next_use_t.push(macro_time);
  }
}

void Workload::prepareDblks(){
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
      cout << d.first << " not in sp " << endl;
      //get sp addr
      if(global_sp->sp_regions[d.first].next_empty.empty()){
        sp_base = global_sp->sp_regions[d.first].next_base_2replace;

        int idx_replace = global_sp->sp_regions[d.first].dblk_idx[sp_base];
        DataBlock* dblk_2replace = &data_arrays[d.first]->dblks[idx_replace];

        cout << "   replace " << idx_replace << endl;
        mns_perf.num_dram_spill += dblk_2replace->size;
        cout << "   num spill " << mns_perf.num_dram_spill << endl;

        if(d.first == "L"){
          mns_perf.num_spill_l += dblk_2replace->size;
        }
        else if(d.first == "U"){
          mns_perf.num_spill_u += dblk_2replace->size;
        }
        else if(d.first == "A"){
          mns_perf.num_spill_a += dblk_2replace->size;
        }

        //some cases need to store
        dblk_2replace->in_sp = false;
        if(dblk_2replace->dirty){
          mns_perf.num_dram_write += dblk_2replace->size;
	  cout << "dram write" << d.first << endl;
        }
      }
      else{
        sp_base = global_sp->sp_regions[d.first].next_empty.front();
        global_sp->sp_regions[d.first].next_empty.pop();
      }

      cout << "   sp base " << sp_base << endl;
      //update info
      if(out_dblk.find(d.first) == out_dblk.end()){
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
        mns_perf.num_dram_read += dblk->size;
      }
    
      mns_perf.num_sram_update += dblk->size;
    }
    else{
      sp_base = dblk->sp_addr.base;
      global_sp->sp_regions[d.first].updateNextBase2Replace(sp_base);
      if(out_dblk.find(d.first) != out_dblk.end()){
        dblk->dirty = true;
      }
    }

    //no future use, remove
    if(dblk->next_use_t.empty()){
      dblk->in_sp = false;
      global_sp->sp_regions[d.first].next_empty.push(dblk->sp_addr.base);
      if(kernal_out_dblk.find(d.first) != kernal_out_dblk.end()){
        mns_perf.num_dram_write += dblk->size;
        cout << "dram write " << d.first << endl;
      }
    }
  }
}

void Workload::printPerf(){
  cout << endl;
  cout << "Number of cycles: " << mns_perf.cycle << endl;
  cout << "Number of compute cycles: " << mns_perf.num_compute_cycles << endl;
  cout << "Number of total use: " << mns_perf.total_use << endl;
  cout << "Number of use from sram: " << mns_perf.use_sram << endl;
  cout << "Number of use from pipe: " << mns_perf.use_pipe << endl;
  cout << "Number of dram read: " << mns_perf.num_dram_read << endl;
  cout << "Number of dram write: " << mns_perf.num_dram_write << endl;
  cout << "Number of dram spill: " << mns_perf.num_dram_spill << endl;
  cout << "Number of spill u: " << mns_perf.num_spill_u << endl;
  cout << "Number of spill l: " << mns_perf.num_spill_l << endl;
  cout << "Number of spill a: " << mns_perf.num_spill_a << endl;
  cout << "Number of sram update: " << mns_perf.num_sram_update << endl;
}
