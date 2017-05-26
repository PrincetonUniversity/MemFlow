#ifndef CGSCHEDULING
#define CGSCHEDULING

#include<vector>
#include<memory>

#include "./ComputationGraph.hpp"
#include "./Scheduling.hpp"
#include "Memory.hpp"
#include "Setting.hpp"
#include "MacroNodeTemplate.hpp"

using namespace std;

extern vector<MacroNodeTemplate*> mn_temps;

//typedef void (*func_p)(int,int,int,int);

class CGScheduling{
  public:
    CGScheduling();
    ~CGScheduling();

    void LoopIteration(int blk_i, int blk_j, int blk_l);
    void genDblks(string mtx_name, int m, int n, int blk_dimi, int blk_dimj);
    void MacroNodeGen();
    void MacroNodeGen_buffer();
 
    void setDblkUseTime();
    void prepareDblks();

    void setDblkIdx_MM();
    void runMacroNode_MM();
    void LoopStructure_MM();
    void MacroNodeGen_MM();

    void setDblkIdx_LU();
    void runMacroNode_LU();
    void LoopStructure_LU();
    void MacroNodeGen_LU();
    
    void setDblkIdx_QR();
    void runMacroNode_QR();
    void LoopStructure_QR();
    void MacroNodeGen_QR();
    
    void PrintPerf();

    int blk_m;
    int blk_n;
    int blk_k;

    int blk_i;
    int blk_j;
    int blk_l;

    int dblks_base_a;
    int dblks_base_b;
    int dblks_base_c;

    map<string,MacroNodeTemplate*> mn_temp;

    shared_ptr<LoadStoreDblk> load_store;

    //partition computation graph into macronodes
    vector<MacroNode> mns;
    vector<DataBlock> dblks;

    map<string,int> dblk_idx;
    set<string> in_dblk;
    set<string> out_dblk;
    set<string> kernal_out_dblk;

    MemoryTrack& mem;

    unsigned long long cycle;
    unsigned long long total_use;
    unsigned long long use_sram;
    unsigned long long use_pipe;
    unsigned long long num_compute_cycles;
    unsigned long long num_spills;
    unsigned long long num_dram_read;
    unsigned long long num_dram_write;
    unsigned long long num_dram_spill;
    unsigned long long num_sram_update;
    unsigned long long num_blk_replaced_a;
    unsigned long long num_blk_replaced_b;
    unsigned long long num_blk_replaced_c;

    unsigned long long num_spill_u;
    unsigned long long num_spill_l;
    unsigned long long num_spill_a;


    int num_dblk_write;
 
    int macro_time;
    //int load_latency;
    //int store_latency;

    map<string, uint64_t> load_latency;
    map<string, uint64_t> store_latency;

    typedef void(CGScheduling::*func_p)();
    func_p func_iteration;
};

#endif
