#ifndef MACRONODETEMPLATE
#define MACRONODETEMPLATE

#include<vector>
#include<string>
#include<memory>

#include "Tile.hpp"
#include "Hardware.hpp"
#include "Memory.hpp"
#include "DataBlock.hpp"
#include "ComputationGraph.hpp"

using namespace std;

struct dblk_info{
  string matrix_name;
  string region_name;
  string elm_spaddr_mode;
  int subblk_dimi;
  int subblk_dimj;

  int dimi;
  int dimj;
};

class MacroNodeTemplate{
  public:
    MacroNodeTemplate(){};
    ~MacroNodeTemplate(){};

    void buildDblks();
    void genScheduling();

    map<string, dblk_info> dblks_info;
    map<string, shared_ptr<DataBlock>> dblks;
    map<string, vector<vector<int>>> mtx;

    ComputationGraph* p_cg;

    shared_ptr<MemoryTrack> mem;
    
    string name;
    int cycle_length;
    int total_use;
    int use_sram;
    int use_pipe;
};

class MN_mtxmul: public MacroNodeTemplate{
  public:
  MN_mtxmul(bool is_add, int in_m, int in_n, int in_k, map<string, dblk_info>& in_dblks_info);
  ~MN_mtxmul(){};

  int m;
  int n;
  int k;
};

class MN_LU: public MacroNodeTemplate{
  public:
    MN_LU(int in_n, map<string, dblk_info>& in_dblks_info);
    ~MN_LU(){};

    int n;
};

//LU complement
class MN_LUCPL: public MacroNodeTemplate{
  public:
    MN_LUCPL(int in_n, map<string, dblk_info>& in_dblks_info);
    ~MN_LUCPL(){};

    int n;
};

class MN_TRS: public MacroNodeTemplate{
  public:
    MN_TRS(int in_n, map<string, dblk_info>& in_dblks_info);
    ~MN_TRS(){};

    int n;
};

class MN_SUBMM: public MacroNodeTemplate{
  public:
    MN_SUBMM(int in_m, int in_n, int in_k, map<string, dblk_info>& in_dblks_info);
    ~MN_SUBMM(){};

    int n;
};

class MN_QR: public MacroNodeTemplate{
  public:
    MN_QR(int in_n, map<string, dblk_info>& in_dblks_info);
    ~MN_QR(){};

    int n;
};

class MN_QRCPL: public MacroNodeTemplate{
  public:
    MN_QRCPL(int in_n, map<string, dblk_info>& in_dblks_info);
    ~MN_QRCPL(){};

    int n;
};

class MN_QRUpdateDiag: public MacroNodeTemplate{
  public:
    MN_QRUpdateDiag(int in_m, int in_n, map<string,dblk_info>& in_dblks_info);
    ~MN_QRUpdateDiag(){};

    int m;
    int n;
};

class MN_QRUpdateTr: public MacroNodeTemplate{
  public:
    MN_QRUpdateTr(int in_n, map<string,dblk_info> &in_dblks_info);
    ~MN_QRUpdateTr(){};

    int n;
};

class MN_QRUpdate: public MacroNodeTemplate{
  public:
    MN_QRUpdate(int in_n, map<string,dblk_info> &in_dblks_info);
    ~MN_QRUpdate(){};
    int n;
};

#endif
