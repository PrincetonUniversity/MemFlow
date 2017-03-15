#ifndef HARDWARE
#define HARDWARE

#include<string>
#include<vector>
#include<array>
#include<map>

#include "./Util.hpp"

using namespace std;

class ComputeBlock{
  public:
    ComputeBlock(){};
    virtual ~ComputeBlock(){};
    virtual int LiveinReadCycle(int livein_idx){return 0;};
    virtual void updateStage(int in_stage){};

    string name;
    const ComputeBlockUnit* cb_unit;
    int width;
    int stage;
    int latency;
};

class CB_Load: public ComputeBlock{
  public:
    CB_Load(string in_name, int in_width);
};

class CB_Store: public ComputeBlock{
  public:
    CB_Store(string in_name, int in_width);
};

class CB_Copy: public ComputeBlock{
  public:
    CB_Copy(string in_name, int in_width);
};

class CB_Sub: public ComputeBlock{
  public:
    CB_Sub(string in_name, int in_width);
};

class CB_Div: public ComputeBlock{
  public:
    CB_Div(string in_name, int in_width);
};

class CB_DivRoot: public ComputeBlock{
  public:
    CB_DivRoot(string in_name, int in_width);
};

class CB_MulAcc: public ComputeBlock{
  public:
    CB_MulAcc(bool in_is_add, string in_name, int in_width, int in_stage);
    void updateStage(int in_stage);
    int LiveinReadCycle(int livein_idx);

    int latency_rw_c;
    bool is_add;
};

class CB_Acc: public ComputeBlock{
  public:
    CB_Acc(string in_name, int in_width, int in_stage);
    void updateStage(int in_stage);
    int LiveinReadCycle(int livein_idx);
};

class CB_Root: public ComputeBlock{
  public:
    CB_Root(string in_name, int in_width);
};

class CB_JacobiS: public ComputeBlock{
  public:
    CB_JacobiS(string in_name);
};

class CB_GivensS: public ComputeBlock{
  public:
    CB_GivensS(string in_name);
};

/*
   namespace ComputeBlockLib{
   CB_Load LOAD("load", 1, 0);
   CB_Store STORE("store", 1, 1);
   CB_Sub SUB("sub", 1, 2);
   CB_Div DIV("div", 1, 3);
   CB_DivRoot DIV_ROOT("div_root", 1, 4);
   CB_MulAcc MUL_ACC("mul_acc", 1, 10, 5);
   CB_Acc ACC("acc", 1, 10, 6);
   CB_Root ROOT_S("root_s", 1, 7);
   CB_Div DIV_S("div_s", 1, 8);
   CB_JacobiS JACOBI_S("jacobi_s", 9);
   CB_GivensS GIVENS_S("givens_s", 10);

   map<string, int> num_cb = {
   {"load", 10},
   {"store", 10},
   {"sub", 10},
   {"div", 10},
   {"div_root", 10},
   {"mul_acc", 10},
   {"acc", 10},
   {"root_s", 1},
   {"div_s", 1},
   {"jacobi_s", 1},
   {"givens_s", 1}
   };
   }
 */

struct MemBank{
  int size;
  int num_port;
};



#endif
