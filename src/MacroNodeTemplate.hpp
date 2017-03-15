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

//struct rw_info{
//  int cycle;
//  array<int,2> addr;
//};

struct tile_sche{
  int start;
  int end;
  int tile_idx;
  int cb_idx;
  map<int,rw_info> inop;
  map<int,rw_info> outop;
};

struct spill_sche{
  int op;
  int latency;
  bool only_read;
  int cond;
  int start;
  int end;
  int start_membank;
  int end_membank;
};

struct bankex_sche{
  int op;
  int latency=2;
  int start;
  int end;
  int start_membank;
  int end_membank;
};

class MacroNodeTemplate{
  public:
    MacroNodeTemplate(){};
    ~MacroNodeTemplate(){};

    //ops, tils inside mn
    //vector<Operation> ops;
    //vector<Tile> tiles;
	
    map<string, shared_ptr<DatBlock>> dblks;
    map<string, vector<vector<int>>> mtx;

    ComputationGraph* p_cg;

    shared_ptr<MemoryTrack> mem;
    
    string name;
    int cycle_length;
};

class MN_mtxmul: public MacroNodeTemplate{
  public:
  MN_mtxmul(int in_m, int in_n, int in_k, bool sche);
  ~MN_mtxmul(){};

  int m;
  int n;
  int k;
};

class MN_LU: public MacroNodeTemplate{
  public:
    MN_LU(int in_n, bool sche);
    ~MN_LU(){};

    int n;
};

//LU complement
class MN_LUCPL: public MacroNodeTemplate{
  public:
    MN_LUCPL(int in_n, bool sche);
    ~MN_LUCPL(){};

    int n;
};

class MN_TRS: public MacroNodeTemplate{
  public:
    MN_TRS(int in_n, bool sche);
    ~MN_TRS(){};

    int n;
};

#endif
