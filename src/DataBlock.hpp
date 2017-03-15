#ifndef DATABLOCK
#define DATABLOCK

#include <vector>
#include <set>
#include <string>

#include "Memory.hpp"

using namespace std;

class DataBlock{
  public:
    DataBlock(string in_mtx_name, string in_region_name, int in_dimi, int in_dimj);
    DataBlock(int in_idx, string in_mtx_name, string in_region_name, int in_dimi, int in_dimj, int in_blk_i, int in_blk_j);
    ~DataBlock(){};

    int AddrIdx();
    void setSPAddr();
    void setSPAddr(DblkAddr& in_sp_addr);
    
    array<int,2> getElementSPAddr_mtxmul_a(int i, int j);
    array<int,2> getElementSPAddr_mtxmul_b(int i, int j);
    array<int,2> getElementSPAddr_mtxmul_c(int i, int j);
    array<int,2> getElementSPAddr_lu_l_tr(int i, int j);
    array<int,2> getElementSPAddr_lu_u_tr(int i, int j);
    array<int,2> getElementSPAddr_lu_l(int i, int j);
    array<int,2> getElementSPAddr_lu_u(int i, int j);
    array<int,2> getElementSPAddr_lu_a(int i, int j);

    array<int,2> getElementSPAddr(int i, int j);

    int size;
    vector<int> ops;
    vector<int> live_cycle;
    vector<int> mem_addr;

    int idx;
    string matrix_name;
    string region_name;
    int blk_dimi;
    int blk_dimj;
    int blk_i;
    int blk_j;

    DblkAddr sp_addr;

    vector<int> pred_mns;
    set<int> post_mns;
};


#endif
