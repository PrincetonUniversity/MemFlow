#ifndef DATA
#define DATA

#include <cstdint>
#include <string>
#include <vector>

#include "Util.hpp"
#include "DataBlock.hpp"

using namespace std;

class DataArray{
  public:
    DataArray(string in_name, int in_m, int in_n);
    ~DataArray(){};
	
    void genDblks(int in_blk_dimi, int in_blk_dimj);

    void setAddrMappingMode(int in_dramblk_dimi, int in_dramblk_dimj, bool in_isColMajorEle, bool in_isColMajorBlk);
    void setStartAddr(uint64_t addr);
    uint64_t genPhysicalAddr(int i, int j);

    void setDblkSPAddrIdx(DblkSPAddrIdx in_mode);

    string name;
    int m;
    int n;
    int size;

    //data block
    int blk_m;
    int blk_n;

    //this blk is in terms of physical address
    int dramblk_dimi;
    int dramblk_dimj;
    int dramblk_m;
    int dramblk_n;

    int dramblk_size;
    bool isColMajorEle;
    bool isColMajorBlk;

    uint64_t start_addr;

    vector<DataBlock> dblks;

    DblkSPAddrIdx addr_idx_mode;
};

#endif
