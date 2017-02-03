#ifndef DATABLOCK
#define DATABLOCK

#include <vector>
#include <set>

using namespace std;

class DataBlock{
  public:
    DataBlock(int in_idx){idx=in_idx;};
    ~DataBlock(){};

    int idx;
    int size;
    vector<int> ops;

    vector<int> pred_mns;
    set<int> post_mns;

    vector<int> live_cycle;

    vector<int> mem_addr;
};

#endif
