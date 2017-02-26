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

class StageEvents{
  public: 
    StageEvents(){produced_out = false;};
    ~StageEvents(){};

    void Enqueue(set<int> &ops);
    void Dequeue();

    set<int> ops;
    bool produced_out;
    bool stall;
    queue<set<int>> q_ops;
};

class CBEvents{
  public:
    CBEvents(int in_num_stage);
    CBEvents(){};
    ~CBEvents(){};
    
    void setStages(int in_num_stage);

    int num_stage;
    vector<StageEvents> stage_events;
};


class TileScheduling{
  friend class SAOptimizer;
  public:
  TileScheduling(MacroNodeTemplate &in_macro, int in_mode);
  ~TileScheduling(){}; 

  int Executing_cbs();
  void Scheduling_pipe();

  int mode; 

  MacroNodeTemplate& macro;
  map<int, set<int>> op_read_cycles;

  int num_cbs;
  int subblk_dimi_tile;
  int subblk_dimj_tile;
  int subblk_diml_tile;

  int subblk_m;
  int subblk_n;
  int subblk_k;

  vector<CBEvents> cb_events;


  int last_cycle;

};

#endif
