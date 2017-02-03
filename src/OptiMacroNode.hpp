#include<iostream>

using namespace std;

class OptiMacroNode{
  public:
  	OptiMacroNode(int in_mem_size, int in_m, int in_n, int in_k);
	OptiMacroNode(int in_mem_size, int in_m, int in_n, int in_k, int in_tile_dimi, int in_tile_dimj, int in_tile_diml);

	~OptiMacroNode(){};

	long num_spills_arow(int mi, int a_space, int b_space, int c_space, int nb_m, int nb_n, int nb_k);
	int num_spills_bcol(int mi, int mb, int nb, int kb, int nb_m, int nb_n, int nb_k);
	int num_spills_c(int mi, int mb, int nb, int kb, int nb_m, int nb_n, int nb_k);

	void SweepMemSize();
	void genMNSize_tile();
	void genMNSize();

	int mem_size;

	int mi;
	//total block size

	//original input size
	int m;
	int n;
	int k;

	//tile dimension
	int tile_dimi;
	int tile_dimj;
	int tile_diml;

	//original #tiles
	int tile_m;
	int tile_n;
	int tile_k;

	//extended size to realize completely divided
	int m_ex;
	int n_ex;
	int k_ex;

	//optimal tile size in one block
	long num_spill;
	int blk_dimi_tile_opti;
	int blk_dimj_tile_opti;
	int blk_diml_tile_opti;

	int blk_dimi_opti;
	int blk_dimj_opti;
	int blk_diml_opti;
};
