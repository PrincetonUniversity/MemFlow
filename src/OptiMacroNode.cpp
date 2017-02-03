#include<iostream>
#include<fstream>
#include<climits>

#include "OptiMacroNode.hpp"
#include "Setting.hpp"

using namespace std;

OptiMacroNode::OptiMacroNode(int in_mem_size, int in_m, int in_n, int in_k){
  mem_size = in_mem_size;

  m = in_m;
  n = in_n;
  k = in_k;
}

OptiMacroNode::OptiMacroNode(int in_mem_size, int in_m, int in_n, int in_k, int in_tile_dimi, int in_tile_dimj, int in_tile_diml){
  mem_size = in_mem_size;

  m = in_m;
  n = in_n;
  k = in_k;

  tile_dimi = in_tile_dimi;
  tile_dimj = in_tile_dimj;
  tile_diml = in_tile_diml;

  tile_m = (m%tile_dimi==0)?m/tile_dimi:m/tile_dimi+1;
  tile_n = (n%tile_dimj==0)?n/tile_dimj:n/tile_dimj+1;
  tile_k = (k%tile_diml==0)?k/tile_diml:k/tile_diml+1;
}

long OptiMacroNode::num_spills_arow(int mi, int a_space, int b_space, int c_space, int nb_m, int nb_n, int nb_k){
  long spill;

  //long spill_a = max((mb*k-(mem_size-mi))*(nb_n-1)*nb_m, 0);
  //last a row computation
  //long spill_b = max(max(n*k-(mem_size-mi), n*k+k*(mb-nb)-(mem_size-mi)), 0);
  //if(nb_m > 2){
  //   spill_b += max((n*k-max(mem_size-mi-mb*k, 0))*(nb_m-2), 0);
  //} 
  //spill = spill_a + spill_b;

  long spill_a;
  long spill_b;
  int num_ablk = (mem_size-mi)/a_space;
  if(num_ablk >= (nb_k-1)){
    spill_a = 0;
    int num_bblk = (mem_size-mi-a_space*(nb_k-1))/b_space;
    if(num_bblk >= (nb_n*nb_k-1)){
      spill_b = 0;
    }
    else{
      spill_b = b_space*(nb_n*nb_k-num_bblk)*(nb_m-1);
    }
  }
  else{
    spill_a = a_space*(nb_k-num_ablk)*(nb_n-1)*nb_m;
    spill_b = b_space*nb_n*nb_k*(nb_m-1);
  }

  spill = spill_a + spill_b;
  return spill;
}


int OptiMacroNode::num_spills_bcol(int mi, int mb, int nb, int kb, int nb_m, int nb_n, int nb_k){
  int spill;

  int spill_b = max(nb*k-(mem_size-mi)*(nb_m-1)*nb_n, 0);
  //last a row computation
  int spill_a = max(max(m*k-(mem_size-mi), m*k+k*(nb-mb)-(mem_size-mi)), 0);
  if(nb_n > 2){
     spill_a += max((m*k-max(mem_size-mi-nb*k, 0))*(nb_n-2), 0);
  } 
  spill = spill_a + spill_b;


  return spill;
}

int OptiMacroNode::num_spills_c(int mi, int mb, int nb, int kb, int nb_m, int nb_n, int nb_k){
  int spill;

  int spill_b = max(kb*n-(mem_size-mi)*(nb_m-1)*nb_k, 0);
  //last a row computation
  int spill_c = max(max(m*n-(mem_size-mi), m*n+n*(kb-mb)-(mem_size-mi)), 0);
  if(nb_k > 2){
     spill_c += max((m*n-max(mem_size-mi-kb*n, 0))*(nb_k-2), 0);
  } 
  spill = spill_b + spill_c;


  return spill;
}


void OptiMacroNode::genMNSize_tile(){
  //try brute forth algorithm
  num_spill = LONG_MAX;
  for(int mb_t=1; mb_t<=tile_m; mb_t++){
    for(int nb_t=1; nb_t<=tile_n; nb_t++){
      for(int kb_t=1; kb_t<=tile_k; kb_t++){
	 //mb_t, nb_t, kb_t: tiles
	 int mb = mb_t*tile_dimi;
	 int nb = nb_t*tile_dimj;
	 int kb = kb_t*tile_diml;

	 int a = mb*kb;
	 int b = nb*kb;
	 int c = mb*nb;

	 int a_space = (a%Memory::num_bank==0)?a:(a/Memory::num_bank+1)*Memory::num_bank;
	 int b_space = (b%Memory::num_bank==0)?b:(b/Memory::num_bank+1)*Memory::num_bank;
	 int c_space = (c%Memory::num_bank==0)?c:(c/Memory::num_bank+1)*Memory::num_bank;

	 int mi_t = a_space+b_space+2*c_space;
	 if(mi_t < mem_size){
	   int blk_m = (m%mb==0)?m/mb:m/mb+1;
	   int blk_n = (n%nb==0)?n/nb:n/nb+1;
	   int blk_k = (k%kb==0)?k/kb:k/kb+1;

	   long num_spill_t = num_spills_arow(mi_t, a_space, b_space, c_space, blk_m, blk_n, blk_k);

	   if(num_spill_t < num_spill){
	     num_spill = num_spill_t;
	     blk_dimi_opti = mb;
	     blk_dimj_opti = nb;
	     blk_diml_opti = kb;

	     m_ex = blk_m*mb;
	     n_ex = blk_n*nb;
	     k_ex = blk_k*kb;

	     mi = mi_t;
	   }
	 }
      }
    }
  }
}

void OptiMacroNode::genMNSize(){
  //try brute forth algorithm
  num_spill = LONG_MAX;

  for(int mb=1; mb<=m; mb++){
    for(int nb=1; nb<=n; nb++){
      for(int kb=1; kb<=k; kb++){

	 int a = mb*kb;
	 int b = nb*kb;
	 int c = mb*nb;

	 int a_space = (a%Memory::num_bank==0)?a:(a/Memory::num_bank+1)*Memory::num_bank;
	 int b_space = (b%Memory::num_bank==0)?b:(b/Memory::num_bank+1)*Memory::num_bank;
	 int c_space = (c%Memory::num_bank==0)?c:(c/Memory::num_bank+1)*Memory::num_bank;
	 
	 int mi_t = a_space + b_space + 2*c_space;

	 if(mi_t < mem_size){
	   int blk_m = (m%mb==0)?m/mb:m/mb+1;
	   int blk_n = (n%nb==0)?n/nb:n/nb+1;
	   int blk_k = (k%kb==0)?k/kb:k/kb+1;

	   long num_spill_t = num_spills_arow(mi_t, a_space, b_space, c_space, blk_m, blk_n, blk_k);
	   if(num_spill_t < num_spill){
	     num_spill = num_spill_t;
	     blk_dimi_opti = mb;
	     blk_dimj_opti = nb;
	     blk_diml_opti = kb;

	     m_ex = blk_m*mb;
	     n_ex = blk_n*nb;
	     k_ex = blk_k*kb;

	     mi = mi_t;
	   }
	 }
      }
    }
  }
}


void OptiMacroNode::SweepMemSize(){
  ofstream f;
  f.open("../mn_size/mn_vs_mem.txt");

  for(mem_size=10; mem_size<65536; mem_size+=100){
    cout << endl << "mem size " << mem_size << endl;
    genMNSize();
  }
  f.close();
}
