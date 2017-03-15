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

struct event{
  int op;
  char r_or_w;

  event(int in_op, char in_r_w):op(in_op),r_or_w(in_r_w){};
  friend bool operator<(const event& e1, const event& e2){
    return (e1.op < e2.op);
  }
  friend bool operator==(const event& e1, const event& e2){
    return (e1.op == e2.op);
  }
};

class StageEvents{
  public: 
    StageEvents(){produced_out = false;};
    ~StageEvents(){};

    void Enqueue(set<event> &ops);
    void Dequeue();

    set<event> ops; //op,'r'or'w'
    bool produced_out;
    bool stall;
    queue<set<event>> q_ops;
};

class CBEvents{
  public:
    CBEvents(int in_num_stage);
    CBEvents(){};
    ~CBEvents(){};
    
    void setStages(int in_num_stage);

    int num_stage;
    vector<StageEvents> stage_events;

    bool finished = false;
};


class TileScheduling{
  friend class SAOptimizer;
  public:
  TileScheduling(MacroNodeTemplate* in_macro, int in_mode);
  ~TileScheduling(){}; 

  int Executing_cbs();
  void Scheduling_pipe();
  void Scheduling_pipe_debug();

  int mode; 

  MacroNodeTemplate* macro;
  map<int, set<int>> op_read_cycles;

  int num_cbs;
  int subblk_dimi_tile;
  int subblk_dimj_tile;
  int subblk_diml_tile;

  int subblk_m;
  int subblk_n;
  int subblk_k;

  map<string, vector<CBEvents>> cb_events;
  int last_cycle;

  map<int, int> intermediate_ops;
};

#endif
