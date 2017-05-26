#include <vector>

#include "Setting.hpp"
#include "DataBlock.hpp"

using namespace std;

DataBlock::DataBlock(string in_mtx_name, string in_region_name, int in_dimi, int in_dimj){
  matrix_name = in_mtx_name;
  region_name = in_region_name;

  blk_dimi = in_dimi;
  blk_dimj = in_dimj;

  in_sp = false;
}

DataBlock::DataBlock(int in_idx, string in_mtx_name, string in_region_name, int in_dimi, int in_dimj, int in_blk_i, int in_blk_j){
  idx = in_idx;
  matrix_name = in_mtx_name;
  region_name = in_region_name;

  blk_dimi = in_dimi;
  blk_dimj = in_dimj;
  blk_i = in_blk_i;
  blk_j = in_blk_j;

  size = blk_dimi*blk_dimj;
  
  in_sp = false;
}

void DataBlock::setElementSPAddr(string mode, int in_subblk_dimi, int in_subblk_dimj){
  elm_spaddr_mode = mode;
  subblk_dimi = in_subblk_dimi;
  subblk_dimj = in_subblk_dimj;
}

//TODO:consider pass parameters of blocking dblk into this function
array<int,2> DataBlock::getElementSPAddr_mtxmul_a(int i, int j){
  array<int,2> addr;
 
  int subblk_i = i/subblk_dimi;
  int subblk_idx = (subblk_i*blk_dimj+j)/subblk_dimj;

  int i_insubblk = i%subblk_dimi;
  int j_insubblk = (subblk_i*blk_dimj+j)%subblk_dimj;
  int idx_insubblk = j_insubblk*subblk_dimi+i_insubblk;

  addr[0] = global_sp->sp_regions[sp_addr.region].start_bank+idx_insubblk/2;
  addr[1] = sp_addr.base+2*subblk_idx+idx_insubblk%2;

  return addr;
}

array<int,2> DataBlock::getElementSPAddr_mtxmul_b(int i, int j){
  array<int,2> addr;

  int subblk_n = blk_dimj/subblk_dimi;
  int subblk_i = i/subblk_dimj;
  int subblk_j = j/subblk_dimi;
  int subblk_idx = subblk_i*subblk_n+subblk_j;
  int addr_offset = sp_addr.base+2*subblk_idx;
  
  int i_insubblk = i%subblk_dimi;
  int j_insubblk = j%subblk_dimj;
  int idx_insubblk = i_insubblk*subblk_dimj+j_insubblk;

  addr[0] = global_sp->sp_regions[sp_addr.region].start_bank+idx_insubblk/2;
  addr[1] = addr_offset+idx_insubblk%2;

  return addr;
}


array<int,2> DataBlock::getElementSPAddr_mtxmul_c(int i, int j){
  array<int,2> addr;

  int subblk_i = i/subblk_dimi;
  int subblk_j = j/subblk_dimj;
  int subblk_m = (blk_dimi%subblk_dimi==0)?blk_dimi/subblk_dimi:blk_dimi/subblk_dimi+1;
  int subblk_n = (blk_dimj%subblk_dimj==0)?blk_dimj/subblk_dimj:blk_dimj/subblk_dimj+1;
  
  int subblk_idx = 0;
  for(int tblk=0; tblk<min(subblk_m, subblk_n); tblk++){
    if(subblk_i > tblk){
      subblk_idx += (subblk_n-tblk);
    }
    else{
      subblk_idx += (subblk_j-tblk);
      break;
    }

    if(subblk_j > tblk){
      subblk_idx += (subblk_m-tblk-1);
    }
    else{
      subblk_idx += (subblk_i-tblk-1);
      break;;
    }
  }

  int i_insubblk = i%subblk_dimi;
  int j_insubblk = j%subblk_dimj;
  int idx_insubblk = i_insubblk*subblk_dimj+j_insubblk;

  int latency = 2;
  int bank_section = (subblk_idx/latency)%2;
  int subblk_idx_section = ((subblk_idx/latency)/2)*latency+(subblk_idx%latency);

  int bank_start;
  if(bank_section == 0){
    bank_start = global_sp->sp_regions[sp_addr.region].start_bank;
  }
  else{
    bank_start = (global_sp->sp_regions[sp_addr.region].start_bank+global_sp->sp_regions[sp_addr.region].end_bank+1)/2;
  }
  addr[0] = bank_start+idx_insubblk/2;
  addr[1] = sp_addr.base+subblk_idx_section;

  return addr;
}

array<int,2> DataBlock::getElementSPAddr_lu_l_tr(int i, int j){
  array<int,2> addr;
  int idx = 0;
  for(int tj=0; tj<j; tj++){
    idx += (blk_dimi-tj-1);
  }
  idx += (i-j-1);

  int subblk_idx = idx/subblk_dimi;
  int idx_insubblk = idx%subblk_dimi;

  addr[0] = global_sp->sp_regions[sp_addr.region].start_bank+idx_insubblk/2;
  addr[1] = sp_addr.base+subblk_idx;

  return addr;
}

array<int,2> DataBlock::getElementSPAddr_lu_l(int i, int j){
  array<int,2> addr;
  int idx = blk_dimi*j+i;

  int subblk_idx = idx/subblk_dimi;
  int idx_insubblk = idx%subblk_dimi;

  addr[0] = global_sp->sp_regions[sp_addr.region].start_bank+idx_insubblk/2;
  addr[1] = sp_addr.base+subblk_idx;

  return addr;
}

array<int,2> DataBlock::getElementSPAddr_lu_u_tr(int i, int j){
  array<int,2> addr;
  int idx = 0;
  for(int ti=0; ti<i; ti++){
    idx += (blk_dimj-ti-1);
  }
  idx += (j-i-1);

  int subblk_idx = idx/subblk_dimj;
  int idx_insubblk = idx%subblk_dimj;

  addr[0] = global_sp->sp_regions[sp_addr.region].start_bank+idx_insubblk/2;
  addr[1] = sp_addr.base+subblk_idx;
  return addr;
}

array<int,2> DataBlock::getElementSPAddr_lu_u(int i, int j){
  array<int,2> addr;
  int idx = blk_dimj*i+j;

  int subblk_idx = idx/subblk_dimj;
  int idx_insubblk = idx%subblk_dimj;

  addr[0] = global_sp->sp_regions[sp_addr.region].start_bank+idx_insubblk/2;
  addr[1] = sp_addr.base+subblk_idx;

  return addr;
}

array<int,2> DataBlock::getElementSPAddr_lu_a(int i, int j){
  array<int,2> addr;

  int subblk_i = i/subblk_dimi;
  int subblk_j = j/subblk_dimj;
  int subblk_m = (blk_dimi%subblk_dimi==0)?blk_dimi/subblk_dimi:blk_dimi/subblk_dimi+1;
  int subblk_n = (blk_dimj%subblk_dimj==0)?blk_dimj/subblk_dimj:blk_dimj/subblk_dimj+1;
  
  int subblk_idx = 0;
  for(int tblk=0; tblk<min(subblk_m, subblk_n); tblk++){
    if(subblk_i > tblk){
      subblk_idx += (subblk_n-tblk);
    }
    else{
      subblk_idx += (subblk_j-tblk);
      break;
    }

    if(subblk_j > tblk){
      subblk_idx += (subblk_m-tblk-1);
    }
    else{
      subblk_idx += (subblk_i-tblk-1);
      break;;
    }
  }

  int i_insubblk = i%subblk_dimi;
  int j_insubblk = j%subblk_dimj;
  int idx_insubblk = i_insubblk*subblk_dimj+j_insubblk;

  int latency = 2;
  int bank_section = (subblk_idx/latency)%2;
  int subblk_idx_section = ((subblk_idx/latency)/2)*latency+(subblk_idx%latency);

  int bank_start;
  if(bank_section == 0){
    bank_start = global_sp->sp_regions[sp_addr.region].start_bank;
  }
  else{
    bank_start = (global_sp->sp_regions[sp_addr.region].start_bank+global_sp->sp_regions[sp_addr.region].end_bank+1)/2;
  }
  addr[0] = bank_start+idx_insubblk/2;
  addr[1] = sp_addr.base+subblk_idx_section;

  return addr;
}

array<int,2> DataBlock::getElementSPAddr(int i, int j){
  if(elm_spaddr_mode == "mode_mm_a"){
    return getElementSPAddr_mtxmul_a(i,j);
  }
  else if(elm_spaddr_mode == "mode_mm_b"){
    return getElementSPAddr_mtxmul_b(i,j);
  }
  else if(elm_spaddr_mode == "mode_mm_c"){
    return getElementSPAddr_mtxmul_c(i,j);
  }
  else if(elm_spaddr_mode == "mode_lu_ltr"){
    return getElementSPAddr_lu_l_tr(i,j);
  }
  else if(elm_spaddr_mode == "mode_lu_l"){
    return getElementSPAddr_lu_l(i,j);
  }
  else if(elm_spaddr_mode == "mode_lu_utr"){
    return getElementSPAddr_lu_u_tr(i,j);
  }
  else if(elm_spaddr_mode == "mode_lu_u"){
    return getElementSPAddr_lu_u(i,j);
  }
  else if(elm_spaddr_mode == "mode_lu_a"){
    return getElementSPAddr_lu_a(i,j);
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
  if(idx >= global_sp->sp_regions[region_name].num_blks-1){
    idx = global_sp->sp_regions[region_name].num_blks-1;
  }
  sp_addr = global_sp->getDblkAddr(region_name, idx);
}

void DataBlock::setSPAddr(DblkAddr& in_sp_addr){
  sp_addr = in_sp_addr;
}
