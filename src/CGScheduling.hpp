#ifndef CGSCHEDULING
#define CGSCHEDULING

#include<vector>
#include<memory>

#include "./ComputationGraph.hpp"
#include "./Scheduling.hpp"
#include "Memory.hpp"

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
    CGScheduling(ComputationGraph &in_cg, MemoryTrack &in_global_mem);
    ~CGScheduling();

    //partition computation into macronodes
    void SetMNDep();
    void SetDep();
    void PrintDep();
    void MacroNodeGen(int blk_dimi_tile, int blk_dimj_tile, int blk_diml_tile);
    void PrintMacroNodes();

    //process data structure
    void ExtendRecordData(int to_size);
    void ExtendRecordData(int to_size, vector<vector<int>> &new_num_port, vector<vector<int>> &new_num_live, map<int,vector<int>> &new_op_in_bank);

    void add_mn(int cycle, MacroNode* mn, vector<vector<int>> &new_num_port, vector<vector<int>> &new_num_live);
    void remove_mn(int cycle, MacroNode* mn, vector<vector<int>> &new_num_port, vector<vector<int>> &new_num_live);

    void UpdateMNCycle(int old_cycle, int new_cycle, shared_ptr<MemoryTrack>& new_mem, MacroNode* mn, vector<int>& nonspill_op);
    int FindCycle(MacroNode* mn);	

    //allocate each macronode
    void AllocateMN(MacroNode* mn, int cycle);


    void FindCycle_balancedbank(MacroNode* mn);
    void AllocateMN_balancedbank(MacroNode* mn, int cycle);

    void genMNOrder(vector<int>& order);
    //schedule each macronode
    void AllocateDblks();
    void Scheduling();

    void PrintScheduling();
    void PrintSpill();
    void PrintBankex();
    void PrintPerf();

    //input: computation graph (ops, tiles)
    ComputationGraph& cg;	
    MemoryTrack& global_mem;

    //partition computation graph into macronodes
    vector<MacroNode*> mns;
    vector<DataBlock*> dblks;

    vector<shared_ptr<DataBlock>> dblks_ptr;

    vector<int> mn_order;
    vector<vector<load_dblk>> load;
    vector<vector<store_dblk>> store;

    //output
    vector<tile_sche> tiles_sche;
    vector<spill_sche> spills_sche;
    vector<bankex_sche> bankexs_sche;

    //data structures used to help global scheduling
    vector<vector<int>> new_num_port;
    vector<vector<int>> new_num_live;
    map<int,vector<int>> new_op_in_bank;

    vector<vector<int>> test_num_port;
    vector<vector<int>> test_num_live;
    map<int,vector<int>> test_op_in_bank;


    vector<bankex_sche> bankex_x_mn;
    vector<spill_sche> spill_x_mn;

    map<int, int> mn_startcycle;	

    int begin_cycle;
    map<int,vector<int>> op_in_bank;
    vector<vector<int>> num_port;
    vector<vector<int>> num_live;

    map<int,set<int>> op_usermn;
    map<int,int> op_in_cycle;

    shared_ptr<MemoryTrack> mem;
    shared_ptr<MemoryTrack> new_mem;


    int num_cycles;

    int load_latency;
    int store_latency;
};

class MacroNodePriority{
  public:
    MacroNodePriority(int in_mn_idx, CGScheduling* in_sche);

    int mn_idx;
    CGScheduling* sche;

    int priority;
};

#endif
