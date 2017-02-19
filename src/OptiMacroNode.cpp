#include<iostream>
#include<fstream>
#include<climits>

#include "OptiMacroNode.hpp"
#include "Setting.hpp"

using namespace std;

Parameters::Parameters(){
  blk_dimi = 0;
  blk_dimj = 0;
  blk_diml = 0;

  subblk_dimi = 0;
  subblk_dimj = 0;
  subblk_diml = 0;

  k_stage = 0;
  num_cb = 0;

  num_bank_a = 0;
  num_bank_b = 0;
  num_bank_c = 0;

  m_ex = 0;
  n_ex = 0;
  k_ex = 0;

}

OptiMacroNode::OptiMacroNode(int in_m, int in_n, int in_k, Parameters& in_opti_para):opti_para(in_opti_para){
  m = in_m;
  n = in_n;
  k = in_k;

}

unsigned long long OptiMacroNode::num_spills_arow(){
  unsigned long long spill;

  unsigned long long spill_a;
  unsigned long long spill_b;

  int num_ablk = (mem_size-mi)/a_space;
  if(num_ablk >= (blk_k-1)){
    spill_a = 0;
    int num_bblk = (mem_size-mi-a_space*(blk_k-1))/b_space;
    if(num_bblk >= (blk_n*blk_k-1)){
      spill_b = 0;
    }
    else{
      spill_b = b_space*(blk_n*blk_k-num_bblk)*(blk_m-1);
    }
  }
  else{
    spill_a = a_space*(blk_k-num_ablk)*(blk_n-1)*blk_m;
    spill_b = b_space*blk_n*blk_k*(blk_m-1);
  }

  spill = spill_a + spill_b;
  return spill;
}


unsigned long long OptiMacroNode::num_spills_bcol(){
    unsigned long long spill;

    unsigned long long spill_a;
    unsigned long long spill_b;

    int num_bblk = (mem_size-mi)/b_space;
    if(num_bblk >= (blk_k-1)){
        spill_b = 0;
        int num_ablk = (mem_size-mi-b_space*(blk_k-1))/a_space;
        if(num_ablk >= (blk_m*blk_k-1)){
            spill_a = 0;
        }
        else{
            spill_a = a_space*(blk_m*blk_k-num_ablk)*(blk_n-1);
        }
    }
    else{
        spill_b = b_space*(blk_k-num_bblk)*(blk_m-1)*blk_n;
        spill_a = a_space*blk_m*blk_k*(blk_n-1);
    }

    spill = spill_a + spill_b;
    return spill;
}

unsigned long long OptiMacroNode::num_spills_c(){
    unsigned long long spill;

    unsigned long long spill_b;
    unsigned long long spill_c;

    int num_bblk = (mem_size-mi)/b_space;
    if(num_bblk >= (blk_n-1)){
        spill_b = 0;
        int num_cblk = (mem_size-mi-b_space*(blk_n-1))/c_space;
        if(num_cblk >= (blk_m*blk_n-1)){
            spill_c = 0;
        }
        else{
            spill_c = c_space*(blk_m*blk_n-num_cblk)*(blk_k-1);
        }
    }
    else{
        spill_b = b_space*(blk_n-num_bblk)*(blk_m-1)*blk_k;
        spill_c = c_space*blk_m*blk_n*(blk_k-1);
    }

    spill = spill_b + spill_c;
    return spill;
}


void OptiMacroNode::genMNSize_tile(){
  //try brute forth algorithm
  num_spill = LLONG_MAX;
  vol_opti = LLONG_MAX;

  for(int mb_t=1; mb_t<=tile_m; mb_t++){
      blk_dimi = mb_t*tile_dimi;
      if(MinMem(blk_dimi, 1, 1) > mem_size){
          break;
      }
    for(int nb_t=1; nb_t<=tile_n; nb_t++){
        blk_dimj = nb_t*tile_dimj;
        if(MinMem(blk_dimi, blk_dimj, 1) > mem_size){
            break;
        }
      for(int kb_t=1; kb_t<=tile_k; kb_t++){
          blk_diml = kb_t*tile_diml;
          if(MinMem(blk_dimi, blk_dimj, blk_diml) > mem_size){
              break;
          }

         a = blk_dimi*blk_diml;
         b = blk_dimj*blk_diml;
         c = blk_dimi*blk_dimj;

         a_space = (a%Memory::num_bank==0)?a:(a/Memory::num_bank+1)*Memory::num_bank;
         b_space = (b%Memory::num_bank==0)?b:(b/Memory::num_bank+1)*Memory::num_bank;
         c_space = (c%Memory::num_bank==0)?c:(c/Memory::num_bank+1)*Memory::num_bank;

         mi = a_space+b_space+2*c_space;
         if(mi < mem_size){
             //cout << endl;
             //cout << "blk_dimi " << blk_dimi << endl;
             //cout << "blk_dimj " << blk_dimj << endl;
             //cout << "blk_diml " << blk_diml << endl;

           blk_m = (m%blk_dimi==0)?m/blk_dimi:m/blk_dimi+1;
           blk_n = (n%blk_dimj==0)?n/blk_dimj:n/blk_dimj+1;
           blk_k = (k%blk_diml==0)?k/blk_diml:k/blk_diml+1;

	   m_ex = blk_m*blk_dimi;
	   n_ex = blk_n*blk_dimj;
	   k_ex = blk_k*blk_diml;
	   vol = m_ex*n_ex*k_ex;

           unsigned long long num_spill_t = num_spills_arow();
             //cout << "num spill " << num_spill_t << endl;

           if((num_spill_t < num_spill) || ((num_spill_t == num_spill) && (vol <= vol_opti))){
             num_spill = num_spill_t;
             blk_dimi_opti = blk_dimi;
             blk_dimj_opti = blk_dimj;
             blk_diml_opti = blk_diml;

             m_ex_opti = m_ex;
             n_ex_opti = n_ex;
             k_ex_opti = k_ex;
	     vol_opti = vol;

             mi_opti = mi;
           }
         }
      }
    }
  }
}

int OptiMacroNode::MinMem(int blk_dimi, int blk_dimj, int blk_diml){
    return (blk_dimi*blk_diml+blk_dimj*blk_diml+2*blk_dimi*blk_dimj);
}


void OptiMacroNode::genMNSize(){
  //try brute forth algorithm
  num_spill = LLONG_MAX;
  vol_opti = LLONG_MAX;

  for(blk_dimi=1; blk_dimi<=m; blk_dimi++){
     if(MinMem(blk_dimi, 1, 1) > mem_size){
       break;
     }
    for(blk_dimj=1; blk_dimj<=n; blk_dimj++){
        if(MinMem(blk_dimi, blk_dimj, 1) > mem_size){
          break;
        }
      for(blk_diml=1; blk_diml<=k; blk_diml++){
         if(MinMem(blk_dimi, blk_dimj, blk_diml) > mem_size){
           break;
         }

          //cout << "blk_dimi " << blk_dimi << endl;
          //cout << "blk_dimj " << blk_dimj << endl;
          //cout << "blk_diml " << blk_diml << endl;

         a = blk_dimi*blk_diml;
         b = blk_dimj*blk_diml;
         c = blk_dimi*blk_dimj;

         a_space = a;
         b_space = b;
         c_space = c;

         mi = a_space + b_space + 2*c_space;

         if(mi < mem_size){
           blk_m = (m%blk_dimi==0)?m/blk_dimi:m/blk_dimi+1;
           blk_n = (n%blk_dimj==0)?n/blk_dimj:n/blk_dimj+1;
           blk_k = (k%blk_diml==0)?k/blk_diml:k/blk_diml+1;
	   
	   m_ex = blk_m*blk_dimi;
	   n_ex = blk_n*blk_dimj;
	   k_ex = blk_k*blk_diml;
	   vol = m_ex*n_ex*k_ex;

           unsigned long long num_spill_t = num_spills_arow();

           if((num_spill_t < num_spill) || ((num_spill_t == num_spill) && (vol <= vol_opti))){
             num_spill = num_spill_t;

             blk_dimi_opti = blk_dimi;
             blk_dimj_opti = blk_dimj;
             blk_diml_opti = blk_diml;

             m_ex_opti = m_ex;
             n_ex_opti = n_ex;
             k_ex_opti = k_ex;
	     vol_opti = vol;

             mi_opti = mi;
           }
         }
      }
    }
  }
}


void OptiMacroNode::SweepMemSize(){
  ofstream f;
  f.open("./mn_size/mn_vs_mem.txt");

  for(mem_size=10; mem_size<16000; mem_size+=100){
    genMNSize();
      f << mem_size << " " << blk_dimi_opti << " " << blk_dimj_opti << " " << blk_diml_opti << " " << num_spill << endl;
  }
  f.close();
}


bool OptiMacroNode::ConstraintBW(int k_subblk, int m_subblk, int n_subblk){
  int port_per_bank = Memory::membanks[0].num_port;

  int num_port_a = k_subblk*m_subblk;
  num_bank_a = (num_port_a%port_per_bank==0)?num_port_a/port_per_bank:num_port_a/port_per_bank+1;

  int num_port_b = k_subblk*n_subblk;
  num_bank_b = (num_port_b%port_per_bank==0)?num_port_b/port_per_bank:num_port_b/port_per_bank+1;

  int num_port_c = 2*m_subblk*n_subblk;
  num_bank_c = (num_port_c%port_per_bank==0)?num_port_c/port_per_bank:num_port_c/port_per_bank+1;

  if(num_bank_a+num_bank_b+num_bank_c <= Memory::num_bank){
    return true;
  }
  else{
    return false;
  }
}

unsigned long long OptiMacroNode::spill_a(){
  unsigned long long spill;
  int num_a_region = Memory::membanks[0].size/a_region;

  if(num_a_region >= blk_k){
    spill = 0;
  }
  else{
    spill = a*(blk_k+1-num_a_region)*(blk_n-1)*blk_m;
  }
  return spill;
}

unsigned long long OptiMacroNode::spill_b(){
  unsigned long long spill;
  int num_b_region = Memory::membanks[0].size/b_region;

  if(num_b_region >= blk_n*blk_k){
    spill = 0;
  }
  else{
    spill = b*(blk_n*blk_k+1-num_b_region)*(blk_m-1);
  }
  return spill;
}

unsigned long long OptiMacroNode::getPerf(){
  unsigned long long perf;

  int num_subblk_level = blk_dimi_sb*blk_dimj_sb;
  int latency_cb = (subblk_diml-1)+3+1+2;

  int perf_blk = max(num_subblk_level, latency_cb)*blk_diml_sb+min(num_subblk_level,latency_cb)-1;
  
  perf = perf_blk*blk_m*blk_n*blk_k;
  
  return perf;
}

void OptiMacroNode::optiPara(){
  cout << "Memory banks: " << Memory::num_bank << endl;
  cout << "bank 0 size: " << Memory::membanks[0].size << endl;


  num_spill = LLONG_MAX;
  perf = LLONG_MAX;
  for(subblk_dimi=1; subblk_dimi<=m; subblk_dimi++){
    for(subblk_dimj=1; subblk_dimj<=n; subblk_dimj++){
      for(subblk_diml=1; subblk_diml<=k; subblk_diml++){
 	if(ConstraintBW(subblk_diml, subblk_dimi, subblk_dimj)){
	  cout << endl;
	  cout << "subblk dimi: " << subblk_dimi << endl;
	  cout << "subblk dimj: " << subblk_dimj << endl;
	  cout << "subblk diml: " << subblk_diml << endl;

	  int subblk_m = (m%subblk_dimi==0)?m/subblk_dimi:m/subblk_dimi+1;
	  int subblk_n = (n%subblk_dimj==0)?n/subblk_dimj:n/subblk_dimj+1;
	  int subblk_k = (k%subblk_diml==0)?k/subblk_diml:k/subblk_diml+1;
	
	  for(blk_dimi_sb=1; blk_dimi_sb<=subblk_m; blk_dimi_sb++){
	    blk_dimi = blk_dimi_sb*subblk_dimi;
	    for(blk_dimj_sb=1; blk_dimj_sb<=subblk_n; blk_dimj_sb++){
	      blk_dimj = blk_dimj_sb*subblk_dimj;
	      for(blk_diml_sb=1; blk_diml_sb<=subblk_k; blk_diml_sb++){
		blk_diml = blk_diml_sb*subblk_diml;

	        blk_m = (m%blk_dimi==0)?m/blk_dimi: m/blk_dimi+1;
	        blk_n = (n%blk_dimj==0)?n/blk_dimj: n/blk_dimj+1;
	        blk_k = (k%blk_diml==0)?k/blk_diml: k/blk_diml+1;

		a = blk_dimi*blk_diml;
		b = blk_dimj*blk_diml;
		c = blk_dimi*blk_dimj;
                
		a_region = (a%num_bank_a==0)?a/num_bank_a:a/num_bank_a+1;
		b_region = (b%num_bank_b==0)?b/num_bank_b:b/num_bank_b+1;
		c_region = (a%num_bank_c==0)?c/num_bank_c:c/num_bank_c+1;

		if((a_region <= Memory::membanks[0].size)
		    && (b_region <= Memory::membanks[0].size)
		    && (2*c_region <= Memory::membanks[0].size)){
	 
		  unsigned long long cur_spill = spill_a()+spill_b();
		  unsigned long long cur_perf = getPerf();

		  if((cur_spill < num_spill)
		      || ((cur_spill == num_spill) && (cur_perf < perf))){
		    num_spill = cur_spill;
		    perf = cur_perf;

		    opti_para.subblk_dimi = subblk_dimi;
		    opti_para.subblk_dimj = subblk_dimj;
		    opti_para.subblk_diml = subblk_diml;
		    opti_para.blk_dimi = blk_dimi;
		    opti_para.blk_dimj = blk_dimj;
		    opti_para.blk_diml = blk_diml;

		    opti_para.m_ex = blk_m*blk_dimi;
		    opti_para.n_ex = blk_n*blk_dimj;
		    opti_para.k_ex = blk_k*blk_diml;

		    opti_para.num_bank_a = num_bank_a;
		    opti_para.num_bank_b = num_bank_b;
		    opti_para.num_bank_c = num_bank_c;
		  }

		}
	      }
	    }
	  }
	  cout << "num spill " << num_spill << endl;
	}
      }
    }
  }

  opti_para.k_stage = opti_para.subblk_diml;
  opti_para.num_cb = opti_para.subblk_dimi*opti_para.subblk_dimj;

  cout << "num spill " << num_spill << endl;
  cout << "perf " << perf << endl;
}
