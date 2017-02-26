#ifndef OPTIMACRONODE
#define OPTIMACRONODE

#include<iostream>

using namespace std;


class Parameters{
  public:
  Parameters();
  ~Parameters(){};


  int blk_dimi;
  int blk_dimj;
  int blk_diml;

  int subblk_dimi;
  int subblk_dimj;
  int subblk_diml;

  int k_stage;
  int num_cb;

  int num_bank_a;
  int num_bank_b;
  int num_bank_c;

  int m_ex;
  int n_ex;
  int k_ex;
};


class OptiMacroNode{
  public:
	OptiMacroNode(int in_m, int in_n, int in_k, Parameters& in_opti_para);

	~OptiMacroNode(){};

	unsigned long long num_spills_arow();
	unsigned long long num_spills_bcol();
	unsigned long long num_spills_c();

	void SweepMemSize();
	void genMNSize_tile();
	void genMNSize();
	int MinMem(int blk_dimi, int blk_dimj, int blk_diml);

	bool ConstraintBW(int k_subblk, int m_subblk, int n_subblk);
	unsigned long long spill_a();
	unsigned long long spill_b();
	unsigned long long getPerf();
	void optiPara();


	bool ConstraintBW_buffer(int k_subblk, int m_subblk, int n_subblk);
	unsigned long long spill_a_buffer();
	unsigned long long spill_b_buffer();
	void optiPara_buffer();

	int mem_size;

	//original input size
	int m;
	int n;
	int k;

	//num of banks for A
	int num_bank_a;
	int num_bank_b;
	int num_bank_c;
	
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

	//address regions taken by a,b,c blocks
	int a_region;
	int b_region;
	int c_region;
	
	//extended size to realize completely divided
	int m_ex;
	int n_ex;
	int k_ex;
	
	//optimal tile size in one block
	unsigned long long num_spill;
	unsigned long long perf;
	
	//optimization result
	int blk_dimi_opti;
	int blk_dimj_opti;
	int blk_diml_opti;

	int subblk_dimi_opti;
	int subblk_dimj_opti;
	int subblk_diml_opti;

	int k_stage;
	int num_cb;

	int num_bank_a_opti;
	int num_bank_b_opti;
	int num_bank_c_opti;
	Parameters& opti_para;

	
	//mem size occupied by a,b,c block
	int a_space;
	int b_space;
	int c_space;

	//minimum mem for computation
	int mi;

	unsigned long long vol;

	int blk_dimi_tile_opti;
	int blk_dimj_tile_opti;
	int blk_diml_tile_opti;

	unsigned long long vol_opti;

	int mi_opti;
	
	//tile dimension
	int tile_dimi;
	int tile_dimj;
	int tile_diml;

	//original #tiles
	int tile_m;
	int tile_n;
	int tile_k;
	
	int m_ex_opti;
	int n_ex_opti;
	int k_ex_opti;
};

#endif
