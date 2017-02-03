#ifndef MACRONODETEMPLATE
#define MACRONODETEMPLATE

#include<vector>
#include<string>
#include<memory>

#include "Tile.hpp"
#include "Hardware.hpp"
#include "Memory.hpp"

using namespace std;

struct tile_sche{
  int start;
  int end;
  int tile_idx;
  map<int,array<int,2>> inop_addr;
  map<int,array<int,2>> outop_addr;
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
    MacroNodeTemplate(string in_name);
    MacroNodeTemplate(vector<Operation> &in_ops, vector<Tile> &in_tiles, string in_name);
    ~MacroNodeTemplate(){};

    void MN_mtxmul(int m, int n, int k, bool sche);
    void MN_load(int tile_width, bool sche);
    void MN_store(int tile_width, bool sche);
    void MN_2load_mtxmul(int m, int n, int k, bool sche);
    void MN_load1_mtxmul(int m, int n, int k, bool sche);
    void MN_load2_mtxmul(int m, int n, int k, bool sche);
    void GetScheduling();

    //ops, tils inside mn
    vector<Operation> ops;
    vector<Tile> tiles;


    //For scheduling tiles (allocate liveranges)
    //*************************************
    //data determined by scheduling
    vector<tile_sche> tiles_sche;

    vector<spill_sche> spills_sche;
    vector<bankex_sche> bankexs_sche;
    
    shared_ptr<MemoryTrack> mem;

    map<int,int> op_in_cycle;	
    map<int,int> inop_firstread;
    //key: op idx in mn, value: cycle to read op for the first time

    //io ops
    //just need to update mem port usage in mem
    vector<vector<int>> Ain;   
    vector<vector<int>> Bin;   
    vector<vector<int>> Cin;   
    vector<vector<int>> Cout;   

    map<int,array<int,2>> ioop_addr;
    //key: io op, value: mem addr

    int A_base;
    int B_base;
    int Cin_base;
    int Cout_base;


    vector<int> max_num_live;
    //*************************************

    string name;

    int cycle_length;
};

#endif
