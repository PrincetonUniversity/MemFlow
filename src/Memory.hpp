#ifndef MEMORY
#define MEMORY

#include <vector>
#include <map>
#include <queue>
#include <utility>

#include "Hardware.hpp"
#include "OptiMacroNode.hpp"
#include "Data.hpp"

using namespace std;

extern map<string, DataArray*> data_arrays;

class SPRegion{
  public:
  SPRegion(){};
  SPRegion(string in_name, int in_start_bank, int in_end_bank);
  ~SPRegion(){};

  void setupSPRegion(int in_num_blks);
  void updateNextBase2Replace(int changed_base);
  
  string name;
  int start_bank;
  int end_bank;
  int num_blks;

  vector<int> dblk_idx;
  queue<int> next_empty;

  int farest_next_use;
  int next_base_2replace;
};

struct SPBaseComp{
  bool operator()(const pair<SPRegion*,int>& b1, const pair<SPRegion*,int>& b2){
    int dblk1 = b1.first->dblk_idx[b1.second];
    int dblk2 = b2.first->dblk_idx[b2.second];
    int next_use1 = data_arrays[b1.first->name]->dblks[dblk1].next_use_t.front();
    int next_use2 = data_arrays[b2.first->name]->dblks[dblk2].next_use_t.front();
    return (next_use1 < next_use2);
  }
};

//struct DblkAddr{
//  string region;
//  int base;
  //DblkAddr(){};
  //DblkAddr(string in_region, int in_base):region(in_region),base(in_base){};
//};

class MemoryTrack{
  public:
    MemoryTrack();
    ~MemoryTrack(){};

    void setBankSize(int bank_i, int size);
    void useComputeSize();

    void ExtendLength(int to_size);
    int getLength();

    /*
    void AddMN(int cycle, MacroNode* mn);
    void RemoveMN(int cycle, MacroNode* mn);
    bool CanAddMN(int cycle, MacroNode* mn);
*/

    bool CanPortInc(int bank_i, int cycle);
    bool CanLiveInc(int bank_i, int loc_j, int cycle);
    void PortInc(int bank_i, int cycle);
    void LiveInc(int bank_i, int loc_j, int cycle);

    int CanLiveInc_range(int bank_i, int start_c, int end_c);
    void LiveInc_range(int bank_i, int loc_j, int start_c, int end_c);

    void OpinbankSet(int op, int bank_i, int loc_j, int cycle);
    void OpinbankSet_range(int op, int bank_i, int loc_j, int start_c, int end_c);

    int getOpBank(int op, int cycle);
    int getOpLoc(int op, int cycle);

    void PrintPort(int bank_i);
    void PrintLive(int bank_i, int loc_j);

    void CutMem(int new_begin);

    void VerifyPortBound();
    void VerifyLiveBound();

    int getLastCycle();

    void getMaxNumLive();
  
    void PrintInfo();


    int begin;

    int num_bank;
    vector<MemBank> membanks;   

    int total_port;
    int total_size;

    int a_base;
    int b_base;
    int c_base;

    int a_space;
    int b_space;
    int c_space;

    int a_row_space;
    int b_row_space;
    int c_row_space;

    map<int,vector<array<int,2>>> op_in_bank;
    vector<vector<int>> num_port;
    vector<vector<vector<int>>> num_live;

    vector<int> max_num_live;

    Parameters* opti_para;

    void getOptiPara(Parameters* in_opti_para);
    void Slice2Dblks();
    void Slice2Dblks_debug();
    void Slice2Dblks_buffer();

    DblkAddr getDblkAddr(string mtx_name, int blk_idx);
    array<int,2> getAddr_a_ele(DblkAddr dblk_addr, int m, int n, int i, int j);
    array<int,2> getAddr_b_ele(DblkAddr dblk_addr, int m, int n, int i, int j);
    array<int,2> getAddr_c_ele(DblkAddr dblk_addr, int m, int n, int i, int j, int in_out_latency);

    //Parameters& opti_para;
    map<string, SPRegion> sp_regions;

    map<string, int> blk_interval;
};

#endif


