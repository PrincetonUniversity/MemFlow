#ifndef OPTIMACRONODE
#define OPTIMACRONODE

#include<map>
#include<string>
#include<iostream>
#include "Util.hpp"

using namespace std;

class OptiMacroNode;

class LoopOrder{
  public:
  LoopOrder(){};
  LoopOrder(string loop1, string loop2, string loop3);
  ~LoopOrder(){};
  void setupLoopOrder(OptiMacroNode* opti_mn);
  void setDblkSPAddrIdx();

  //type 1 spill
  string matrix_spilltype1;
  int num_dblk1_mem;
  int dblk1_size;
  int num_dblk1_need;
  int num_reuse1;
  int num_iterate;
  DblkSPAddrIdx idx_mode1;

  //type 2 spill
  string matrix_spilltype2;
  int num_dblk2_mem;
  int dblk2_size;
  int num_dblk2_need;
  int num_reuse2;
  DblkSPAddrIdx idx_mode2;

  string matrix_nospill;
  array<string,3> loop_idc;
};

class Parameters{
  public:
  Parameters();
  ~Parameters(){};

  void PrintInfo();

  //blk dim
  map<string, int> blk_dim;

  //subblk dim
  map<string, int> subblk_dim;

  //num of blks in each direction
  map<string, int> num_blk;

  //size of block
  map<string, int> blk_size;

  //key1: mn temp name
  //key2: cb name
  map<string, map<string,int>> k_stage;
  map<string, map<string,int>> num_cb;
  
  //number of banks
  map<string, int> num_bank;

  map<string, int> num_port;

  map<string, int> ex_input_size;

  unsigned long long num_spill;
  unsigned long long blk_compute_cycles;
  unsigned long long total_compute_cycles;

  LoopOrder loop_order;
};


class OptiMacroNode{
  public:
	OptiMacroNode(int in_m, int in_n, int in_k, Parameters& in_opti_para);
	~OptiMacroNode(){};

	int MinMem(int blk_dimi, int blk_dimj, int blk_diml);

	void genSubblkSet();

	unsigned long long spill(LoopOrder &loop_order, unsigned long long& spill1, unsigned long long& spill2);
	unsigned long long spill_type1(int blk_size, int num_dblk_mem, int num_dblk_need, int num_reuse, int iterate);
	unsigned long long spill_type2(int blk_size, int num_dblk_mem, int num_dblk_need, int num_reuse);
	
	void getPerf(unsigned long long& perf, unsigned long long& blk_perf);
	int MinPort(int sb_dimi, int sb_dimj, int sb_diml);
	int MinBank(int k_subblk, int m_subblk, int n_subblk);

	
	void optiPara();
	void setPara(); //used for debug

	bool ConstraintBW_buffer(int k_subblk, int m_subblk, int n_subblk);
	unsigned long long spill_a_buffer();
	unsigned long long spill_b_buffer();
	void optiPara_buffer();

	LoopOrder loop_order;

	int mem_size;

	int total_mem_port;

	int num_port_used;
	vector<array<int,3>> sb_dim_set;

	//original input size
	int m;
	int n;
	int k;

	//num of banks for A
	int num_bank_a;
	int num_bank_b;
	int num_bank_c;
	
	int num_ablk_mem;
	int num_bblk_mem;
	int num_cblk_mem;

	//subblk dimension
	int subblk_dimi;
	int subblk_dimj;
	int subblk_diml;

	//blk dimension
	int blk_dimi;
	int blk_dimj;
	int blk_diml;

	//blk dimension in terms of subblk
	int blk_dimi_sb;
	int blk_dimj_sb;
	int blk_diml_sb;

	//#blks
	int blk_m;
	int blk_n;
	int blk_k;

	//block size of a, b, c
	int a;
	int b;
	int c;

	//address interval taken by a,b,c blocks
	int a_interval;
	int b_interval;
	int c_interval;

	
	//extended size to realize completely divided
	int m_ex;
	int n_ex;
	int k_ex;
	
	//optimal tile size in one block
	unsigned long long num_spill;
	unsigned long long perf;
	
	Parameters& opti_para;



};

#endif
