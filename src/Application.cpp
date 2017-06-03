#include "Application.hpp"

Application::Application(string app_name, int in_m, int in_n, int in_k){
  name = app_name;
  m = in_m;
  n = in_n;
  k = in_k;
}

void Application::setupDataInfo(){
  if(name == "MM"){
    datas_info.push_back(data_info("A", opti_para.axes["i"].ex_input_size, opti_para.axes["l"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["l"].blk_dim));
    datas_info.push_back(data_info("B", opti_para.axes["l"].ex_input_size, opti_para.axes["j"].ex_input_size, opti_para.axes["l"].blk_dim, opti_para.axes["j"].blk_dim));
    datas_info.push_back(data_info("C", opti_para.axes["i"].ex_input_size, opti_para.axes["j"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["j"].blk_dim));
  }
  else if(name == "LU"){
    datas_info.push_back(data_info("L", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
    datas_info.push_back(data_info("U", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
    datas_info.push_back(data_info("A", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  }
  else if(name == "QR"){
    datas_info.push_back(data_info("Q", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
    datas_info.push_back(data_info("R", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
    datas_info.push_back(data_info("A", opti_para.axes["i"].ex_input_size, opti_para.axes["i"].ex_input_size, opti_para.axes["i"].blk_dim, opti_para.axes["i"].blk_dim));
  }
}

void Application::setup(){
  //generate optimized design para
  OptiMacroNode opti(m,n,k,opti_para);
  opti.optiPara();
  opti_para.PrintInfo();

  //slice DRAM
  global_sp->getOptiPara(&opti_para);
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


void Application::run(){
  CGScheduling mn_sche;
  mn_sche.MacroNodeGen();
  mn_sche.PrintPerf();
  //MacroNodeTemplate* mn_temp = new MN_LUCPL(opti_para.blk_dim[app+"i"]);
  //mn_temps.push_back(mn_temp);
}

void Application::test_mntemp(){
  OptiMacroNode opti(m,n,k,opti_para);
  //opti.setPara();
  opti.optiPara_kernel();
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
