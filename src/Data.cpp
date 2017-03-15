#include <iostream>
#include "Data.hpp"

DataArray::DataArray(string in_name, int in_m, int in_n){
  name = in_name;
  m = in_m;
  n = in_n;
  size = m*n;
}

void DataArray::genDblks(int in_blk_dimi, int in_blk_dimj){
  blk_m = m/in_blk_dimi;
  blk_n = n/in_blk_dimj;

  for(int blk_i=0; blk_i<blk_m; blk_i++){
    for(int blk_j=0; blk_j<blk_n; blk_j++){
      DataBlock db(dblks.size(), name, name, in_blk_dimi, in_blk_dimj, blk_i, blk_j);
      db.setSPAddr();
      dblks.push_back(db);
    }
  }
}

void DataArray::setAddrMappingMode(int in_dramblk_dimi, int in_dramblk_dimj, bool in_isColMajorEle, bool in_isColMajorBlk){
  dramblk_dimi = in_dramblk_dimi;
  dramblk_dimj = in_dramblk_dimj;
  isColMajorEle = in_isColMajorEle;
  isColMajorBlk = in_isColMajorBlk;

  dramblk_m = m/dramblk_dimi;
  dramblk_n = n/dramblk_dimj;
  dramblk_size = dramblk_dimi*dramblk_dimj;
}

void DataArray::setStartAddr(uint64_t addr){
  start_addr = addr;
}

uint64_t DataArray::genPhysicalAddr(int i, int j){
  int dramblk_i = i/dramblk_dimi;
  int dramblk_j = j/dramblk_dimj;

  int dramblk_idx = isColMajorBlk? dramblk_j*dramblk_m+dramblk_i: dramblk_i*dramblk_n+dramblk_j;

  int i_inblk = i%dramblk_dimi;
  int j_inblk = j%dramblk_dimj;

  int idx_inblk = isColMajorEle? j_inblk*dramblk_dimi+i_inblk:i_inblk*dramblk_dimj+j_inblk;

  int idx = dramblk_idx*dramblk_size+idx_inblk;
  return start_addr+4*idx;
}

void DataArray::setDblkSPAddrIdx(DblkSPAddrIdx in_mode){
  addr_idx_mode = in_mode;
}
