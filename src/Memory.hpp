#ifndef MEMORY
#define MEMORY

#include <vector>
#include <map>

#include "Hardware.hpp"

using namespace std;

class MemoryTrack{
  public:
    MemoryTrack();
    MemoryTrack(vector<int> &ops, bool use_compute_size);
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


    void Slice2Blocks(int blk_dimi, int blk_dimj, int blk_diml, int m, int n, int k);
    int getBase_a(int idx);
    int getBase_b(int idx);
    int getBase_c(int idx);

    int begin;

    int num_bank;
    vector<MemBank> membanks;   

    int a_base;
    int b_base;
    int c_base;

    int a_space;
    int b_space;
    int c_space;

    int a_row_space;
    int b_row_space;
    int c_row_space;
    //slice mem into a,b,c regions
    //vector length: number of blks in this region
    //vector value: dblk being stored
    vector<int> a;
    vector<int> b;
    vector<int> c;

    map<int,vector<array<int,2>>> op_in_bank;
    vector<vector<int>> num_port;
    vector<vector<vector<int>>> num_live;

    vector<int> max_num_live;
};

#endif


