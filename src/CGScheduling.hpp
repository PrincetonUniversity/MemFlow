#ifndef CGSCHEDULING
#define CGSCHEDULING

#include<vector>
#include<memory>

#include "./ComputationGraph.hpp"
#include "./Scheduling.hpp"
#include "Memory.hpp"
#include "Setting.hpp"

using namespace std;

class CGScheduling{
  public:
    CGScheduling();
    ~CGScheduling();

    void LoopIteration(int blk_i, int blk_j, int blk_l);
    void genDblks(string mtx_name, int m, int n, int blk_dimi, int blk_dimj);
    void MacroNodeGen();
    void MacroNodeGen_buffer();
    
    void PrintPerf();

    int blk_m;
    int blk_n;
    int blk_k;

    int dblks_base_a;
    int dblks_base_b;
    int dblks_base_c;

    MacroNodeTemplate* mn_temp;

    //partition computation graph into macronodes
    vector<MacroNode> mns;
    vector<DataBlock> dblks;

    vector<LoadDblk> load;
    vector<StoreDblk> store;

    MemoryTrack& mem;

    unsigned long long num_compute_cycles;
    unsigned long long num_spills;
    unsigned long long num_dram_read;
    unsigned long long num_dram_write;
   
    int num_dblk_write;

    int load_latency;
    int store_latency;
};

#endif
