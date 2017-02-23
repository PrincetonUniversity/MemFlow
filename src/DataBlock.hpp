#ifndef DATABLOCK
#define DATABLOCK

#include <vector>
#include <set>
#include <string>

using namespace std;

class DataBlock{
  public:
    DataBlock(int in_idx, string in_name, int in_m, int in_n, int in_basei, int in_basej);
    ~DataBlock(){};

    int size;
    vector<int> ops;
    vector<int> live_cycle;
    vector<int> mem_addr;

    int idx;
    string matrix_name;
    int size_m;
    int size_n;
    int base_i;
    int base_j;
    vector<int> pred_mns;
    set<int> post_mns;
};

#endif
