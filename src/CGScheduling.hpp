#ifndef CGSCHEDULING
#define CGSCHEDULING

#include<vector>
#include<memory>

#include "./ComputationGraph.hpp"
#include "./Scheduling.hpp"
#include "Memory.hpp"
#include "Setting.hpp"

using namespace std;

struct load_dblk{
  int dblk_idx;
  int mem_base;
};

struct store_dblk{
  int dblk_idx;
  int mem_base;
};

class CGScheduling{
  public:
    CGScheduling(MemoryTrack& in_mem);
    ~CGScheduling();

    void genDblks(string mtx_name, int m, int n, int blk_dimi, int blk_dimj);
    void MacroNodeGen();
    
    void PrintPerf();

    //partition computation graph into macronodes
    vector<MacroNode> mns;
    vector<DataBlock> dblks;

    vector<vector<load_dblk>> load;
    vector<vector<store_dblk>> store;

    MemoryTrack& mem;

    int num_cycles;

    int load_latency;
    int store_latency;
};

#endif
