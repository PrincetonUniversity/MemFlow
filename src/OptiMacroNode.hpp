#ifndef OPTIMACRONODE
#define OPTIMACRONODE

#include<map>
#include<string>
#include<iostream>
#include<fstream>
#include "Util.hpp"
#include "Parameters.hpp"

using namespace std;

class Workload;
extern string app;

class OptiMacroNode{
  public:
	OptiMacroNode(Workload* in_wl, Parameters& in_opti_para);
	~OptiMacroNode(){};
	
	//MM
	int MM_MinMem(int blk_dimi, int blk_dimj, int blk_diml);
	int MM_MinPort(int sb_dimi, int sb_dimj, int sb_diml);
	int MM_MinBank(int k_subblk, int m_subblk, int n_subblk);
	void MM_genSubblkSet();
	unsigned long long MM_spill(LoopOrder &loop_order, unsigned long long& spill1, unsigned long long& spill2);
	unsigned long long MM_spill_type1(int blk_size, int num_dblk_mem, int num_dblk_need, int num_reuse, int iterate);
	unsigned long long MM_spill_type2(int blk_size, int num_dblk_mem, int num_dblk_need, int num_reuse);
	void MM_getPerf(unsigned long long& perf, unsigned long long& blk_perf);
	unsigned long long MM_getPerf_blk();
        void MM_optiPara();
	void MM_genSubblkSet_expr();
        void MM_optiPara_expr();
        
	
	//for kernels
	void MM_genSubblkSet_new();
	void MM_updatePara();
	void MM_optiPara_kernel();
	
	void LU_genSubblkSet_new();
	void LU_updatePara();
	void LU_updatePara_old();
	void LU_optiPara_kernel();

	void LUCPL_updatePara();
	void LUCPL_genSubblkSet_new();
	void LUCPL_updatePara_old();
	void LUCPL_optiPara_kernel();
	
	void TRS_genSubblkSet_new();
	void TRS_updatePara();
	void TRS_updatePara_old();
	void TRS_optiPara_kernel();
	void SUBMM_updatePara();
	void SUBMM_optiPara_kernel();
	
	void QR_genSubblkSet_new();
	void QR_updatePara();
	void QR_optiPara_kernel();
	void QRCPL_updatePara();
	void QRCPL_optiPara_kernel();
	void QRUPDATETR_updatePara();
	void QRUPDATETR_optiPara_kernel();
	void QRUPDATE_updatePara();
	void QRUPDATE_optiPara_kernel();

	//LU
	int LU_MinMem(int blk_dimi, int blk_dimj, int blk_diml);
	int LU_MinPort(int sb_dimi, int sb_dimj, int sb_diml);
	int LU_MinBank(int k_subblk, int m_subblk, int n_subblk);
	void LU_genSubblkSet();
	unsigned long long LU_spill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2);
	void LU_optiPara();
	void LU_optiPara_expr();
	
	//QR
	void QR_genSubblkSet();
	unsigned long long QR_spill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2);
	void QR_optiPara();
	void QR_optiPara_expr();

	void optiPara(Workload* wl);
	void optiPara_kernel();
	void setPara(); //used for debug

        ofstream myfile;

	LoopOrder loop_order;

	int mem_size;
	int total_mem_port;

	int num_port_used;
	vector<array<int,3>> sb_dim_set;

	//original input size
	int m;
	int n;
	int k;

	int m_ex;
	int n_ex;
	int k_ex;
	
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
	//map<string, int> blk_dim_sb;

	//#blks
	int blk_m;
	int blk_n;
	int blk_k;
	
	//num of banks for each data
	map<string, int> num_bank;

	map<string, int> blk_size;
	map<string, int> blk_interval;
	map<string, int> num_blk_mem;

	//optimal tile size in one block
	unsigned long long num_spill;
	unsigned long long perf;
	unsigned long long sram_use;
	unsigned long long mem_access;
	
	Parameters para;
	Parameters& opti_para;

};

#endif
