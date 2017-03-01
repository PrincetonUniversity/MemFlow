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

  size = blk_dimi*blk_dimj;
}

int DataBlock::AddrIdx_a(){
  int idx;
  if(opti_para.loop_order.loop_ind[2] == "n"){
    idx = 0;
  }
  else if(opti_para.loop_order.loop_ind == array<string,3>{"m", "n", "k"}){
    idx = blk_i;
  }
  else if(opti_para.loop_order.loop_ind == array<string,3>{"n","m","k"}){
    idx = blk_i*opti_para.blk_k+blk_j;
  }
  else if(opti_para.loop_order.loop_ind == array<string,3>{"n","k","m"}){
    idx = blk_j*opti_para.blk_m+blk_i;
  }
  else{
    idx = blk_j;
  }
  cout << idx << endl;
  return idx;
}

int DataBlock::AddrIdx_b(){
  int idx;
  if(opti_para.loop_order.loop_ind[2] == "m"){
    idx = 0;
  }
  else if(opti_para.loop_order.loop_ind == array<string,3>{"m", "n", "k"}){
    idx = blk_j*opti_para.blk_k+blk_i;
  }
  else if(opti_para.loop_order.loop_ind == array<string,3>{"m","k","n"}){
    idx = blk_i*opti_para.blk_n+blk_j;
  }
  else if(opti_para.loop_order.loop_ind == array<string,3>{"n","m","k"}){
    idx = blk_i;
  }
  else{
    idx = blk_j;
  }
  cout << idx << endl;
  return idx;
}

int DataBlock::AddrIdx_c(){
  int idx;
  if(opti_para.loop_order.loop_ind[2] == "k"){
    idx = 0;
  }
  else if(opti_para.loop_order.loop_ind == array<string,3>{"m", "k", "n"}){
    idx = blk_j;
  }
  else if(opti_para.loop_order.loop_ind == array<string,3>{"n","k","m"}){
    idx = blk_i;
  }
  else if(opti_para.loop_order.loop_ind == array<string,3>{"k","m","n"}){
    idx = blk_i*opti_para.blk_m+blk_j;
  }
  else{
    idx = blk_j*opti_para.blk_n+blk_i;
  }
  cout << idx << endl;
  return idx;
}

void DataBlock::setSPAddr(){
  if(matrix_name == "A"){
    int idx = AddrIdx_a();
    if(idx >= global_sp->sp_regions["A"].num_blks-1){
      sp_addr = global_sp->getDblkAddr("A", global_sp->sp_regions["A"].num_blks-1);
    }
    else{
      sp_addr = global_sp->getDblkAddr("A", idx);
    }
  }
  else if(matrix_name == "B"){
    int idx = AddrIdx_b();
    if(idx >= global_sp->sp_regions["B"].num_blks-1){
      sp_addr = global_sp->getDblkAddr("B", global_sp->sp_regions["B"].num_blks-1);
    }
    else{
      sp_addr = global_sp->getDblkAddr("B", idx);
    }
  }
  else if(matrix_name == "C"){
    int idx = AddrIdx_c();
    if(idx >= global_sp->sp_regions["C"].num_blks-1){
      sp_addr = global_sp->getDblkAddr("C", global_sp->sp_regions["C"].num_blks-1);
    }
    else{
      sp_addr = global_sp->getDblkAddr("C", idx);
    }
  }
}

