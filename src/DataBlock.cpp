#include <vector>

#include "DataBlock.hpp"

using namespace std;

DataBlock::DataBlock(int in_idx, string in_name, int in_dimi, int in_dimj, int in_blk_i, int in_blk_j){
  idx = in_idx;
  matrix_name = in_name;

  blk_dimi = in_dimi;
  blk_dimj = in_dimj;
  blk_i = in_blk_i;
  blk_j = in_blk_j;
}

void DataBlock::setSPAddr(){
  if(matrix_name == "A"){
    if(blk_i >= global_sp->num_region_a-1){
      sp_addr = global_sp->getDblkAddr("A", global_sp->num_region_a-1);
    }
    else{
      sp_addr = global_sp->getDblkAddr("A", blk_i);
    }
  }
  else if(matrix_name == "B"){
    int blk_m = opti_para.k_ex/opti_para.blk_diml;
    if(blk_j*blk_m+blk_i >= global_sp->num_region_b-1){
      sp_addr = global_sp->getDblkAddr("B", global_sp->num_region_b-1);
    }
    else{
      sp_addr = global_sp->getDblkAddr("B", blk_j*blk_m+blk_i);
    }
  }
  else if(matrix_name == "C"){
    sp_addr = global_sp->getDblkAddr("C", 0);
  }
}

