#include <vector>

#include "Setting.hpp"
#include "DataBlock.hpp"

using namespace std;

DataBlock::DataBlock(string in_name, int in_dimi, int in_dimj){
  matrix_name = in_name;
  blk_dimi = in_dimi;
  blk_dimj = in_dimj;
}

DataBlock::DataBlock(int in_idx, string in_name, int in_dimi, int in_dimj, int in_blk_i, int in_blk_j){
  idx = in_idx;
  matrix_name = in_name;

  blk_dimi = in_dimi;
  blk_dimj = in_dimj;
  blk_i = in_blk_i;
  blk_j = in_blk_j;

  size = blk_dimi*blk_dimj;
}

array<int,2> DataBlock::getElementSPAddr_mtxmul_a(int i, int j){
  array<int,2> addr;
  
  int subblk_i = i/opti_para.subblk_dimi;
  int subblk_idx = (subblk_i*blk_dimj+j)/opti_para.subblk_diml;
  int addr_offset = sp_addr.base+2*subblk_idx;

  int i_insubblk = i%opti_para.subblk_dimi;
  int j_insubblk = (subblk_i*blk_dimj+j)%opti_para.subblk_diml;
  int idx_insubblk = j_insubblk*opti_para.subblk_dimi+i_insubblk;

  addr[0] = global_sp->sp_regions[sp_addr.region].start_bank+idx_insubblk/2;
  addr[1] = addr_offset+idx_insubblk%2;

  return addr;
}

array<int,2> DataBlock::getElementSPAddr_mtxmul_b(int i, int j){
  array<int,2> addr;

  int subblk_n = blk_dimj/opti_para.subblk_dimj;
  int subblk_i = i/opti_para.subblk_diml;
  int subblk_j = j/opti_para.subblk_dimj;
  int subblk_idx = subblk_i*subblk_n+subblk_j;
  int addr_offset = sp_addr.base+2*subblk_idx;
  
  int i_insubblk = i%opti_para.subblk_diml;
  int j_insubblk = j%opti_para.subblk_dimj;
  int idx_insubblk = i_insubblk*opti_para.subblk_dimj+j_insubblk;

  addr[0] = global_sp->sp_regions[sp_addr.region].start_bank+idx_insubblk/2;
  addr[1] = addr_offset+idx_insubblk%2;

  return addr;
}


array<int,2> DataBlock::getElementSPAddr_mtxmul_c(int i, int j){
  array<int,2> addr;

  int subblk_n = blk_dimj/opti_para.subblk_dimj;
  int subblk_i = i/opti_para.subblk_dimi;
  int subblk_j = j/opti_para.subblk_dimj;
  int subblk_idx = subblk_i*subblk_n+subblk_j;

  int i_insubblk = i%opti_para.subblk_dimi;
  int j_insubblk = j%opti_para.subblk_dimj;
  int idx_insubblk = i_insubblk*opti_para.subblk_dimj+j_insubblk;

  addr[0] = global_sp->sp_regions[sp_addr.region].start_bank+idx_insubblk;
  addr[1] = sp_addr.base+subblk_idx;
  return addr;
}

array<int,2> DataBlock::getElementSPAddr(int i, int j){
  if(matrix_name == "A"){
    return getElementSPAddr_mtxmul_a(i,j);
  }
  else if(matrix_name == "B"){
    return getElementSPAddr_mtxmul_b(i,j);
  }
  else{
    return getElementSPAddr_mtxmul_c(i,j);
  }
}

int DataBlock::AddrIdx(){
  int idx;
  if(data_arrays[matrix_name]->addr_idx_mode == ZERO_IDX){
    idx = 0;
  }
  else if(data_arrays[matrix_name]->addr_idx_mode == ROW_IDX){
    idx = blk_i;
  }
  else if(data_arrays[matrix_name]->addr_idx_mode == COL_IDX){
    idx = blk_j;
  }
  else if(data_arrays[matrix_name]->addr_idx_mode == ROWMAJOR_IDX){
    idx = blk_i*data_arrays[matrix_name]->blk_n+blk_j;
  }
  else{
    idx = blk_j*data_arrays[matrix_name]->blk_m+blk_i;
  }
  return idx;
}

void DataBlock::setSPAddr(){
  int idx = AddrIdx();
  if(idx >= global_sp->sp_regions[matrix_name].num_blks-1){
    idx = global_sp->sp_regions[matrix_name].num_blks-1;
  }
  sp_addr = global_sp->getDblkAddr(matrix_name, idx);
}

void DataBlock::setSPAddr(DblkAddr& in_sp_addr){
  sp_addr = in_sp_addr;
}
