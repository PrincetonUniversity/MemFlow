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


  //Parameters opti_para;
  OptiMacroNode opti(m, n, k, opti_para);
  opti.optiPara_buffer();

  cout << endl << "Optimized blocking " << endl;
  cout << "blk dimi: " << opti_para.blk_dimi << endl;
  cout << "blk dimj: " << opti_para.blk_dimj << endl;
  cout << "blk diml: " << opti_para.blk_diml << endl;
  cout << "subblk dimi: " << opti_para.subblk_dimi << endl;
  cout << "subblk dimj: " << opti_para.subblk_dimj << endl;
  cout << "subblk diml: " << opti_para.subblk_diml << endl;
  cout << "extended m: " << opti_para.m_ex << endl;
  cout << "extended n: " << opti_para.n_ex << endl;
  cout << "extended k: " << opti_para.k_ex << endl;

  cout << "#cb stages: " << opti_para.k_stage << endl;
  cout << "#cb: " << opti_para.num_cb << endl;

  cout << "num spills: " << opti.num_spill << endl;
  cout << "perf: " << opti.perf << endl;

  ComputeBlockLib::cbs["store"] = new CB_Store("store",1,1);
  ComputeBlockLib::cbs["load"] = new CB_Load("load",2,1);
  ComputeBlockLib::cbs["mul_acc"] = new CB_MulAcc("mul_acc",0,1,opti_para.k_stage);
  ComputeBlockLib::num_cb["mul_acc"] = opti_para.num_cb;
 

  MemoryTrack mem(opti_para);
  global_sp = &mem;
  global_sp->Slice2Dblks_buffer();

  CGScheduling mn_sche;
  mn_sche.MacroNodeGen();
  mn_sche.PrintPerf();

  /*
  //**************************
  //inst generation
  ComputationGraph cg;
  cg.m = opti_para.m_ex;
  cg.n = opti_para.n_ex;
  cg.k = opti_para.k_ex; 

  vector<vector<int>> in_mtx1;   
  vector<vector<int>> in_mtx2;   
  vector<vector<int>> out_mtx;   

  cout << "##################################################" << endl;
  cout << "Generate computation graph: " << endl;
  clock_t begin = clock();
  //load input
  cg.CP_Load(opti_para.m_ex, opti_para.k_ex, in_mtx1);
  cg.CP_Load(opti_para.k_ex, opti_para.n_ex, in_mtx2);

  ///computation
  cg.CP_MtxMul(in_mtx1, in_mtx2, out_mtx);

  //store output
  cg.CP_Store(out_mtx);
  //cg.PrintOps();

  clock_t end = clock();
  double elapsed_time = double(end-begin)/CLOCKS_PER_SEC;
  cout << "Time elapsed: " << elapsed_time << endl;	
  cout << "##################################################" << endl << endl << endl;

  //cout << "matrix A: " << m << "x" << k << endl; 
  //cout << "matrix B: " << k << "x" << m << endl; 

  cout << "##################################################" << endl;
  cout << "Generate tiles" << endl;
  //begin = clock();
  
  cg.TileGen();

  //end = clock();
  //elapsed_time = double(end-begin)/CLOCKS_PER_SEC;
  //cout << "Time elapsed: " << elapsed_time << endl;
  cout << "##################################################" << endl << endl << endl;


  //cout << "real tiles: " << endl;
  //cout << "----------------------------------------------" << endl;
  //cg.PrintTiles();
  //cout << "----------------------------------------------" << endl;

  //**code for scheduling all tiles as one mn

  //**code for scheduling mns
  CGScheduling sche(cg, mem);

  cout << "##################################################" << endl;
  cout << "Generate macronodes: " << endl;
  begin = clock();

  sche.MacroNodeGen(opti_para.blk_dimi, opti_para.blk_dimj, opti_para.blk_diml);

  sche.PrintMacroNodes();

  end = clock();
  elapsed_time = double(end-begin)/CLOCKS_PER_SEC;
  cout << "Time elapsed: " << elapsed_time << endl;
  cout << "##################################################" << endl << endl << endl;

  cout << "##################################################" << endl;
  cout << "Schedule macronodes: " << endl;
  begin = clock();

  sche.Scheduling();

  end = clock();
  elapsed_time = double(end-begin)/CLOCKS_PER_SEC;
  cout << "Time elapsed: " << elapsed_time << endl;
  cout << "##################################################" << endl << endl << endl;

  sche.PrintPerf();
  */



  freeConfig();

}
