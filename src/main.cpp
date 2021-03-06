#include<iostream>
#include<vector>
#include<array>
#include<map>
#include<fstream>
#include<string>
#include<ctime>
#include<memory>

#include "Parameters.hpp"
#include "ComputationGraph.hpp"
#include "Hardware.hpp"
#include "CGScheduling.hpp"
#include "Util.hpp"
#include "Setting.hpp"
#include "ProcessConfig.hpp"
#include "Memory.hpp"
#include "Workload_MM.hpp"
#include "Workload_QR.hpp"
#include "Workload_LU.hpp"
#include "Data.hpp"
#include "./../DRAMSim2/DRAMSim.h"

using namespace std;

namespace ComputeBlockLib{
  map<string, map<string, ComputeBlock*>> cbs;
//  map<string, map<string, int>> num_cb;
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

string app;

map<string, int> fus;

int main(int argc, char* argv[]){

  //uncommited change
  //input data size
  int m;
  int n;
  int k;

  cout << "number of arg " << argc << endl;

  if(argc == 4){
    app = argv[1];
    m = atoi(argv[2]);
    n = atoi(argv[3]);
  }
  else if(argc == 5){
    app = argv[1];
    m = atoi(argv[2]);
    n = atoi(argv[3]);
    k = atoi(argv[4]);
  }

  //*************************
  //process config file
  readConfig();

  //SRAM(based on config info)
  MemoryTrack mem;
  global_sp = &mem;
  
  //DRAM
  dram = DRAMSim::getMemorySystemInstance("ini/DDR3_micron_16M_8B_x8_sg15.ini", "system.ini.example","./DRAMSim2/"," ",16384);
  //assume acc and dram has same clock freq
  //uint64_t cpu_freq = 0.2*1e9;
  uint64_t cpu_freq = 1607*1e6;
  //uint64_t cpu_freq = 0;
  dram->setCPUClockSpeed(cpu_freq);

  //set fus latency
  fus["add"] = 10;
  //fus["add"] = 1;
  fus["mul"] = 5;
  //fus["mul"] = 1;
  fus["sub"] = 10;
  fus["div"] = 5;
  fus["sqrt"] = 10;

  //Workload* p_workload;
  if(app == "MM"){
    cout << "create workload mm" << endl;
    Workload_MM mm(app, m, n, k);
    mm.setup();
    mm.run();
  }
  else if(app == "LU"){
    cout << "create workload lu" << endl;
    Workload_LU lu(app, m, n, k);
    lu.setup();
    lu.run();
  }
  else if(app == "QR"){
    cout << "create workload qr" << endl;
    Workload_QR qr(app, m, n, k);
    qr.setup();
    qr.run();
  }
  //p_workload->setup();
  //p_workload->run();
  //appli.test_mntemp();

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
