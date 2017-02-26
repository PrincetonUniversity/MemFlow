#ifndef DATABLOCK
#define DATABLOCK

#include <vector>
#include <set>
#include <string>

#include "Setting.hpp"
#include "Memory.hpp"

using namespace std;

class DataBlock{
  public:
    DataBlock(int in_idx, string in_name, int in_dimi, int in_dimj, int in_blk_i, int in_blk_j);
    ~DataBlock(){};

    void setSPAddr();
    
    int size;
    vector<int> ops;
    vector<int> live_cycle;
    vector<int> mem_addr;

    int idx;
    string matrix_name;
    int blk_dimi;
    int blk_dimj;
    int blk_i;
    int blk_j;

    DblkAddr sp_addr;

    vector<int> pred_mns;
    set<int> post_mns;
};

#endif
