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

using namespace std;

namespace ComputeBlockLib{
  map<string, ComputeBlock*> cbs;
  map<string, int> num_cb;
}

namespace Memory{
  int num_bank;
  vector<MemBank> membanks;
}

vector<MacroNodeTemplate*> mn_temps;

Parameters opti_para;
MemoryTrack* global_sp;


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

  MemoryTrack mem;
  global_sp = &mem;
  
  //Parameters opti_para;
  OptiMacroNode opti(m, n, k, opti_para);
  opti.optiPara();
  
  opti_para.PrintInfo();

  ComputeBlockLib::cbs["store"] = new CB_Store("store",1,1);
  ComputeBlockLib::cbs["load"] = new CB_Load("load",2,1);
  ComputeBlockLib::cbs["mul_acc"] = new CB_MulAcc("mul_acc",0,1,opti_para.k_stage);
  ComputeBlockLib::num_cb["mul_acc"] = opti_para.num_cb;
 
  global_sp->getOptiPara(&opti_para);
  global_sp->Slice2Dblks();
  global_sp->PrintInfo();

  cout << endl << "Generating macro nodes..." << endl;
  CGScheduling mn_sche;
  mn_sche.MacroNodeGen();
  mn_sche.PrintPerf();

  freeConfig();
}
