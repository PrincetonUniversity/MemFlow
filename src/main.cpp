#include<iostream>
#include<vector>
#include<array>
#include<map>
#include<fstream>
#include<string>
#include<ctime>
#include<memory>

#include "OptiMacroNode.hpp"
#include "ComputationGraph.hpp"
#include "Hardware.hpp"
#include "CGScheduling.hpp"
#include "Util.hpp"
#include "Setting.hpp"
#include "ProcessConfig.hpp"
#include "Memory.hpp"
#include "Data.hpp"
#include "./../DRAMSim2/DRAMSim.h"

using namespace std;

namespace ComputeBlockLib{
  map<string, map<string, ComputeBlock*>> cbs;
  map<string, map<string, int>> num_cb;
}

namespace Memory{
  int num_bank;
  vector<MemBank> membanks;
}

vector<MacroNodeTemplate*> mn_temps;

Parameters opti_para;
MemoryTrack* global_sp;

DRAMSim::MultiChannelMemorySystem* dram;

map<string, DataArray*> data_arrays;

int main(int argc, char* argv[]){

  //uncommited change
  //input data size
  int m;
  int n;
  int k;

  cout << "number of arg " << argc << endl;

  if(argc == 3){
    m = atoi(argv[1]);
    n = atoi(argv[2]);
  }
  else if(argc == 4){
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    k = atoi(argv[3]);
  }

  //*************************
  //process config file
  readConfig();

  //SRAM(based on config info)
  MemoryTrack mem;
  global_sp = &mem;
  
  //DRAM
  dram = DRAMSim::getMemorySystemInstance("ini/DDR2_micron_16M_8b_x8_sg3E.ini", "system.ini.example","./DRAMSim2/"," ",16384);
  //assume acc and dram has same clock freq
  dram->setCPUClockSpeed(666666666);

  //infrastructure only for matrix multiplication
  //Parameters opti_para;
  OptiMacroNode opti(m, n, k, opti_para);
  //opti.optiPara();
  opti.setPara();
  opti_para.PrintInfo();
  
  /*
  DataArray *a = new DataArray("A",opti_para.m_ex,opti_para.k_ex);
  data_arrays["A"] = a;
  DataArray *b = new DataArray("B",opti_para.k_ex,opti_para.n_ex);
  data_arrays["B"] = b;
  DataArray *c = new DataArray("C",opti_para.m_ex,opti_para.n_ex);
  data_arrays["C"] = c;

  //set dram addr
  //row major order
  data_arrays["A"]->setAddrMappingMode(opti_para.m_ex, opti_para.k_ex, false, false);
  data_arrays["A"]->setStartAddr(0);
  data_arrays["B"]->setAddrMappingMode(opti_para.k_ex, opti_para.n_ex, false, false);
  data_arrays["B"]->setStartAddr(data_arrays["A"]->size);
  data_arrays["C"]->setAddrMappingMode(opti_para.m_ex, opti_para.n_ex, false, false);
  data_arrays["C"]->setStartAddr(data_arrays["A"]->size+data_arrays["B"]->size);

  //set block addr idx mode based on loop order
  opti_para.loop_order.setDblkSPAddrIdx();

  //generate dblks
  data_arrays["A"]->genDblks(opti_para.blk_dim["i"], opti_para.blk_dim["l"]);
  data_arrays["B"]->genDblks(opti_para.blk_dim["l"], opti_para.blk_dim["j"]);
  data_arrays["C"]->genDblks(opti_para.blk_dim["i"], opti_para.blk_dim["j"]);
  */

  //create cb library

  ComputeBlockLib::cbs["mm"]["store"] = new CB_Store("store",1);
  ComputeBlockLib::cbs["mm"]["load"] = new CB_Load("load",1);
  ComputeBlockLib::cbs["mm"]["acc_mul"] = new CB_MulAcc(true,"mm_acc_mul",1,opti_para.k_stage["mm"]["acc_mul"]);
  ComputeBlockLib::num_cb["mm"]["acc_mul"] = opti_para.num_cb["mm"]["acc_mul"];

  ComputeBlockLib::cbs["lu"]["store"] = new CB_Store("store",1);
  ComputeBlockLib::cbs["lu"]["load"] = new CB_Load("load",1);
  ComputeBlockLib::cbs["lu"]["copy"] = new CB_Copy("copy",1);
  ComputeBlockLib::cbs["lu"]["div"] = new CB_Div("lu_div",1);
  ComputeBlockLib::cbs["lu"]["sub_mul"] = new CB_MulAcc(false,"lu_sub_mul",1,opti_para.k_stage["lu"]["sub_mul"]);
  ComputeBlockLib::num_cb["lu"]["div"] = opti_para.num_cb["lu"]["div"];
  ComputeBlockLib::num_cb["lu"]["sub_mul"] = opti_para.num_cb["lu"]["sub_mul"];
  
  ComputeBlockLib::cbs["lucpl"]["store"] = new CB_Store("store",1);
  ComputeBlockLib::cbs["lucpl"]["load"] = new CB_Load("load",1);
  ComputeBlockLib::cbs["lucpl"]["div"] = new CB_Div("div",1);
  ComputeBlockLib::cbs["lucpl"]["sub_mul"] = new CB_MulAcc(false,"sub_mul",1,opti_para.k_stage["lucpl"]["sub_mul"]);
  ComputeBlockLib::num_cb["lucpl"]["div"] = opti_para.num_cb["lucpl"]["div"];
  ComputeBlockLib::num_cb["lucpl"]["sub_mul"] = opti_para.num_cb["lucpl"]["sub_mul"];
  
  ComputeBlockLib::cbs["trs"]["store"] = new CB_Store("store",1);
  ComputeBlockLib::cbs["trs"]["load"] = new CB_Load("load",1);
  ComputeBlockLib::cbs["trs"]["copy"] = new CB_Copy("copy",1);
  ComputeBlockLib::cbs["trs"]["div"] = new CB_Div("div",1);
  ComputeBlockLib::cbs["trs"]["sub_mul"] = new CB_MulAcc(false,"sub_mul",1,opti_para.k_stage["trs"]["sub_mul"]);
  ComputeBlockLib::num_cb["trs"]["div"] = opti_para.num_cb["trs"]["div"];
  ComputeBlockLib::num_cb["trs"]["sub_mul"] = opti_para.num_cb["trs"]["sub_mul"];
 
 
  global_sp->getOptiPara(&opti_para);
  global_sp->Slice2Dblks_debug();
  global_sp->PrintInfo();

  //cout << endl << "Generating macro nodes..." << endl;
  //CGScheduling mn_sche;
  //mn_sche.MacroNodeGen();
  //mn_sche.PrintPerf();

  //MacroNodeTemplate* mn_temp = new MN_mtxmul(opti_para.blk_dim["mm_i"], opti_para.blk_dim["mm_j"], opti_para.blk_dim["mm_l"], true);
  //mn_temps.push_back(mn_temp);
  //MacroNodeTemplate* mn_temp = new MN_LU(opti_para.blk_dim["lu_i"], true);
  //mn_temps.push_back(mn_temp);
  //MacroNodeTemplate* mn_temp = new MN_TRS(opti_para.blk_dim["lu_i"], true);
  //mn_temps.push_back(mn_temp);
  MacroNodeTemplate* mn_temp = new MN_LUCPL(opti_para.blk_dim["lu_i"], true);
  mn_temps.push_back(mn_temp);

  /*
  ofstream outfile;
  outfile.open("plot_data.txt",ios::app);
  outfile << opti_para.blk_dimi << " ";
  outfile << opti_para.blk_dimj << " ";
  outfile << opti_para.blk_diml << " ";
  outfile << opti_para.k_stage << " ";
  outfile << opti_para.num_cb << " ";
  outfile << mn_sche.cycle << " ";
  outfile << mn_sche.num_compute_cycles << endl;
  */


  freeConfig();
}
