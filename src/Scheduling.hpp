#ifndef SCHEDULING
#define SCHEDULING

#include <vector>
#include <set>
#include <array>
#include <queue>

#include "./Tile.hpp"
#include "./Setting.hpp"
#include "./MacroNode.hpp"

#define SPILL 0
#define BANKEX 1

#define CG 0
#define MN 1

#define LOAD_LATENCY 100
#define STORE_LATENCY 100

#define BANKEX_LATENCY 2

using namespace std;

struct lr_size;

class TileScheduling;

class Liverange{
  public:
    Liverange(TileScheduling* in_sche){sche = in_sche;};
    ~Liverange(){};

    void GetLiverange(int in_op, int cycle);
    void UpdateLiverange(int old_read_cycle, int new_read_cycle);
    void UpdateLR(int tile, int new_read_cycle);
    //get the liverange where op located in at cycle
    void RemoveLRfromMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live);
    void RemoveLRfromMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live, vector<int> &num_port_left);
    void AddLRtoMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live);
    void AddLRtoMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live, vector<int> &num_port_left);
    void CanAddLRtoMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live);
    void PrintLR();

    TileScheduling* sche;
    int op;
    int mem_bank;
    int mem_loc;
    bool allocated;
    int write_cycle; //start at write_cycle+1
    int last_cycle;
    set<int> read_cycles;

    bool can_add;
    string cause;//cause of failing to add lr
};

class TilePriority{
  public:
    TilePriority(int tile, TileScheduling* sche);
    ~TilePriority(){};

    void UpdatePriority();

    int tile;
    TileScheduling* sche;

    int priority;
};

class TileScheduling{
  friend class SAOptimizer;
  public:
  TileScheduling(MacroNodeTemplate &in_macro, int in_mode);
  ~TileScheduling(); 

  //starting scheduling
  void InitScheduling_best();

  //Schedule tiles in given memory space (allocate all live ranges with the help of spilling and bankex)
  void Scheduling();

  bool CanAddLRstoMem_bf(set<Liverange*> &lrs, vector<vector<int>> &num_port, vector<vector<vector<int>>> &num_live);
  bool CanAddLRstoMem(set<Liverange*> &lrs, vector<vector<int>> &num_port, vector<vector<vector<int>>> &num_live);

  void ExtendMemData(int max_cycle);
  void ExtendMemData(int max_cycle, vector<vector<int>>& new_num_port, vector<vector<vector<int>>>& new_num_live);
  void ExtendNumCB(int max_cycle);
  void ExtendOpinBank(int max_cycle);

  bool AllocateLiveout(int cycle, int tile);
  bool RemoveLiveinLR(int cycle, int tile);
  bool CanAddwithMaxSpilling(int cycle, int tile);
  void RemoveSpilling(int tile);

  bool CheckCycle(int cycle, int tile);
  bool CheckCycle_MN(int cycle, int tile);
  void AllocateTile(int cycle, int tile);
  bool HasReadWrite(int cycle, int tile);
  void ScheduleTile(int tile);
 
  int FindCycle_dblks(int tile);
  void AllocateTile_dblks(int tile, int cycle);
  void Scheduling_dblks();

  void PrintNumPort(int mem_bank);
  void PrintNumLive(int mem_bank);

  bool PassTileDependency();
  bool PassValidSpill();
  bool PassCBBound();
  bool PassPortBound();
  bool PassMemoryBound();
  bool Testing();

  void PrintScheduling();

  void GetNumCBMax();
  void PrintNumCBMax();

  void PrintPerf();

  //private:
  int max_cycle;
  //int store_latency;
  //int load_latency;

  int mode; 

  map<int, vector<int>> op_spills;
  //key: op idx, vector: spill idx
  MacroNodeTemplate& macro;

  //int num_type; 
  //num of compute tile type in the current scheduling
  int last_cycle;
  //num of cycles in the current scheduling
  int sum_cycle;


  //hw allocation
  map<string,vector<int>> num_cb;
  //key:hw type, set:cycles it being occupied

  map<string, int> num_cb_max;

  vector<int> num_port_left;

  //option for condition updates
  map<int,vector<tile_sche>> tile_slack; 
  //key:tile, set:cycles it can start with
  map<int,vector<spill_sche>> spill0_slack; 
  //key:spill idx, set:cycles it can be activated with
  map<int,vector<spill_sche>> spill1_slack;
  //key1:spill idx, key2: 0 start 1 end, set:cycles it can be placed

  //data structure for checking tile allocation
  vector<vector<int>> new_num_port;
  vector<vector<vector<int>>> new_num_live;

  map<int, int> spillidx;
  map<int, Liverange*> tileout_lrs;
  map<int, Liverange*> tilein_lrs;
  //map<int, array<int,3>> bankex_to_alloc;

  map<int, Liverange*> original_inop_lrs;
  map<int, Liverange*> updated_inop_lrs;
  map<int, Liverange*> spill_lrs;
  map<int, Liverange*> bankex_lrs;

  int total_port;

  int store_latency;
  int load_latency;
  int memory_size;
  int read_bound;
  int write_bound;

  vector<int> cycle_delta;

};

#endif
