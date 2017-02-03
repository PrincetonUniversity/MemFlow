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

using namespace std;

namespace ComputeBlockLib{
  map<string, ComputeBlock*> cbs;
  map<string, int> num_cb;
}

namespace Memory{
  int num_bank;
  vector<MemBank> membanks;
}

int main(int argc, char* argv[]){


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

  int total_mem_size = 0;
  for(int i=0; i<Memory::num_bank; i++){
    total_mem_size += (Memory::membanks[i].size-Memory::membanks[i].compute_size);
  }

  int tile_dimi = 1;
  int tile_dimj = 1;
  int tile_diml = ComputeBlockLib::cbs["mul_acc"]->max_depth;
  OptiMacroNode opti(total_mem_size, m, n, k, tile_dimi, tile_dimj, tile_diml);
  opti.genMNSize_tile();
  cout << "blk dimi: " << opti.blk_dimi_opti << endl;
  cout << "blk dimj: " << opti.blk_dimj_opti << endl;
  cout << "blk diml: " << opti.blk_diml_opti << endl;
  cout << "extended m: " << opti.m_ex << endl;
  cout << "extended n: " << opti.n_ex << endl;
  cout << "extended k: " << opti.k_ex << endl;

  cout << "sum of block size: " << opti.mi << endl;
  cout << "num of spills: " << opti.num_spill << endl;

  //opti.SweepMemSize();
  MemoryTrack mem;
  mem.Slice2Blocks(opti.blk_dimi_opti, opti.blk_dimj_opti, opti.blk_diml_opti, opti.m_ex, opti.n_ex, opti.k_ex);

  //**************************
  //inst generation
  ComputationGraph cg;
  cg.m = m;
  cg.n = n;
  cg.k = k; 

  vector<vector<int>> in_mtx1;   
  vector<vector<int>> in_mtx2;   
  vector<vector<int>> out_mtx;   

  cout << "##################################################" << endl;
  cout << "Generate computation graph: " << endl;
  clock_t begin = clock();
  //load input
  cg.CP_Load(opti.m_ex, opti.k_ex, in_mtx1);
  cg.CP_Load(opti.k_ex, opti.n_ex, in_mtx2);

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

  sche.MacroNodeGen(opti.blk_dimi_opti, opti.blk_dimj_opti, opti.blk_diml_opti);

  sche.PrintMacroNodes();

  end = clock();
  elapsed_time = double(end-begin)/CLOCKS_PER_SEC;
  cout << "Time elapsed: " << elapsed_time << endl;
  cout << "##################################################" << endl << endl << endl;

  cout << "##################################################" << endl;
  cout << "Schedule macronodes: " << endl;
  begin = clock();

  //sche.Scheduling();

  end = clock();
  elapsed_time = double(end-begin)/CLOCKS_PER_SEC;
  cout << "Time elapsed: " << elapsed_time << endl;
  cout << "##################################################" << endl << endl << endl;

  //sche.PrintPerf(); 

  freeConfig();

}
