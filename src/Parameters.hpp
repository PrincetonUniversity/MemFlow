#ifndef PARAMETERS
#define PARAMETERS

#include<map>
#include<string>
#include<iostream>
#include<fstream>
#include "Util.hpp"

using namespace std;

extern string app;

class LoopOrder{
  public:
  LoopOrder(){};
  LoopOrder(string loop1, string loop2, string loop3);
  ~LoopOrder(){};
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
  bool set = false;
};

struct cb_info{
  int num_cb;
  int k_stage = 1;
  int width = 1;
  int num_fus;
  int num_regs;
};

struct blk_info{
  int size;
  int num_bank;
  int num_port;
  int interval;
  int num_blk_mem;
};

struct axis_info{
  int input_size;
  int ex_input_size;
  int blk_dim;
  int subblk_dim;
  int num_blk;
  int num_subblk;
  int num_subblk_inblk;
};

struct mntemp_info{
  int cycles;
  int total_use;
  int use_pipe;
  int use_sram;
};

class Parameters{
  public:
  Parameters();
  ~Parameters(){};

  void PrintInfo();

  map<string, axis_info> axes;
  map<string, map<string,cb_info>> cbs;
  map<string, blk_info> dblks;
  map<string, mntemp_info> mntemps;

  int total_num_fus;
  int total_num_regs;

  unsigned long long num_spill;
  unsigned long long num_spill1;
  unsigned long long num_spill2;

  map<string, int> blk_cycles;
  unsigned long long total_compute_cycles;

  LoopOrder loop_order;
};

#endif
