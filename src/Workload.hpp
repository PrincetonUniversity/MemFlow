#ifndef WORKLOAD
#define WORKLOAD

#include "MacroNode.hpp"
#include "Parameters.hpp"
#include "Hardware.hpp"
#include "Memory.hpp"
#include "Setting.hpp"
#include "CGScheduling.hpp"
#include <memory>

using namespace std;

extern vector<MacroNodeTemplate*> mn_temps;

struct data_info{
  string name;
  int dimi;
  int dimj;
  int blk_dimi;
  int blk_dimj;
  data_info(string in_name, int in_dimi, int in_dimj, int in_blk_dimi, int in_blk_dimj):name(in_name),dimi(in_dimi),dimj(in_dimj),blk_dimi(in_blk_dimi),blk_dimj(in_blk_dimj){};
};

struct perf{
  unsigned long long cycle = 0;
  unsigned long long num_compute_cycles = 0;
  
  unsigned long long total_use = 0;
  unsigned long long use_sram = 0;
  unsigned long long use_pipe = 0;
  
  unsigned long long num_spills = 0;
  unsigned long long num_dram_read = 0;
  unsigned long long num_dram_write = 0;
  unsigned long long num_dram_spill = 0;
  
  unsigned long long num_sram_update = 0;

  unsigned long long num_spill_u = 0;
  unsigned long long num_spill_l = 0;
  unsigned long long num_spill_a = 0;
};

class Workload{
  public:
  Workload(string app_name, int in_m, int in_n, int in_k);
  ~Workload(){};

  virtual void setupDataInfo();
  void setup();
  void run();
  void test_mntemp();

  virtual void optiPara(){};

  void setDblkUseTime();
  void prepareDblks();
  virtual void runMacroNode(){};
  virtual void LoopStructure(){};
  virtual void genMacroNode(){};
  
  void printPerf();

  Parameters para;
  string name;
  int m;
  int n;
  int k;

  int macro_time;

  int blk_i;
  int blk_j;
  int blk_l;

  vector<data_info> datas_info;
  map<string,int> dblk_idx;
  set<string> in_dblk;
  set<string> out_dblk;

  set<string> kernal_out_dblk;
  map<string, MacroNodeTemplate*> mn_temp;
  shared_ptr<LoadStoreDblk> load_store;

  perf mns_perf;

  vector<MacroNode> mns;
  
  //typedef void(Workload::*func_p)();
  //func_p func_iteration;
};

#endif
