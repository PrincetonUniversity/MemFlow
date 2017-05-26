#ifndef PATTERN
#define PATTERN

#include<vector>
#include<array>
#include<string>
#include<set>
#include<map>
#include<list>
#include<memory>

//#include "Hardware.hpp"
//#include "Util.hpp"
#include "Tile.hpp"

using namespace std;

// a set of ops repeated in pattern
struct PatternUnit{
  vector<int> ops;
};

// define pattern in the computation graph
// each pattern has a scaling rule
class Pattern{
  friend class ComputationGraph;
  public:


  Pattern() {};
  virtual ~Pattern(){};
  void PrintNumOps();
  void PrintOps(vector<Operation> &ops);
  virtual void TileGen(vector<Tile> &tiles, vector<Operation> &ops){};
  //virtual void MacroNodeGen(vector<MacroNode*> &mns, int mn_dimi, int mn_dimj){};
  //virtual void MacroNodeGen(int mn_dimi, int mn_dimj, int mn_diml){};

  void SetMNRowMajor(){row_major_mn = true;};

  vector<int> tiles_idx;	
  vector<PatternUnit> pus;

  bool row_major_mn = false;
  ComputeBlock* cb;
  string name;
  string description;
  string mn_name;

  int m;
  int n;
  int k;

  int m_ex;
  int n_ex;
  int k_ex;

  //number of tiles
  int tile_m;
  int tile_n;
  int tile_k;

  //number of mns
  int mn_m;
  int mn_n;
  int mn_k;
};

class Load_vec: public Pattern{
  public:
    Load_vec(vector<Operation> &ops, string &in_mn_name, int n, vector<int> &out_vec);
};

class Load_mtx: public Pattern{
  public:
    Load_mtx(vector<Operation> &ops, string &in_mn_name, int in_m, int in_n, vector<vector<int>> &out_mtx);
    ~Load_mtx();
    void TileGen(vector<Tile> &tiles, vector<Operation> &ops);
    //void MacroNodeGen(vector<MacroNode*> &mns, int mn_dimi, int mn_dimj);
    //void MacroNodeGen_idxorder(vector<MacroNode*> &mns);
    //void MacroNodeGen_tile(int mn_dimi, int mn_dimj, int mn_diml);
    //void MacroNodeGen(int mn_dimi, int mn_dimj, int mn_diml);

};

class Store_vec: public Pattern{
  public:
    Store_vec(vector<Operation> &ops, string &in_mn_name, const vector<int> &in_vec);
};

class Store_mtx: public Pattern{
  public:
    Store_mtx(vector<Operation> &ops, string &in_mn_name, const vector<vector<int>> &in_mtx);
    ~Store_mtx();
    void TileGen(vector<Tile> &tiles, vector<Operation> &ops);
    //void MacroNodeGen(vector<MacroNode*> &mns, int mn_dimi, int mn_dimj);
    //void MacroNodeGen(int mn_dimi, int mn_dimj, int mn_diml);
    //void MacroNodeGen_tile(int mn_dimi, int mn_dimj, int mn_diml);
    //void MacroNodeGen_idxorder(vector<MacroNode*> &mns);
};

class Copy_vec: public Pattern{
  public:
    Copy_vec(vector<Operation> &ops, string &in_mn_name, const vector<int> &in_vec, vector<int> &out_vec);
    ~Copy_vec(){};
    void TileGen(vector<Tile> &tiles, vector<Operation> &ops);
};

class Copy_mtx: public Pattern{
  public:
    Copy_mtx(vector<Operation> &ops, string &in_mn_name, const vector<vector<int>> &in_mtx, vector<vector<int>> &out_mtx);
    ~Copy_mtx(){};
    void TileGen(vector<Tile> &tiles, vector<Operation> &ops);
};

class ColSum: public Pattern{
  public:
    ColSum(vector<Operation> &ops, string &in_mn_name, const vector<vector<int>> &in_mtx, vector<int> &out_vec);
};

class Div_vec: public Pattern{
  public:
    Div_vec(vector<Operation> &ops, string &in_mn_name, const vector<int> &in_vec1, const vector<int> &in_vec2, vector<int> &out_vec);
    ~Div_vec(){};
    void TileGen(vector<Tile> &tiles, vector<Operation> &ops);
};

class Mul_vec: public Pattern{
  public:
    Mul_vec(vector<Operation> &ops, string &in_mn_name, const vector<int> &in_vec1, const vector<int> &in_vec2, vector<int> &out_vec);
    ~Mul_vec(){};
    void TileGen(vector<Tile> &tiles, vector<Operation> &ops);
};

class Div_mtx: public Pattern{
  public:
    Div_mtx(vector<Operation> &ops, string &in_mn_name, const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, vector<vector<int>> &out_mtx);
};

class QRHouseholderPara: public Pattern{
  public:
    QRHouseholderPara(vector<Operation> &ops, string &in_mn_name, const int& norm2, const int& x1, int& diag, int& divider);
    ~QRHouseholderPara(){};
    void TileGen(vector<Tile> &tiles, vector<Operation>& ops);
};

class QRHouseholderMtxMul: public Pattern{
  public:
    QRHouseholderMtxMul(vector<Operation>&ops, string &in_mn_name, const vector<int>&w, const vector<vector<int>>&in_mtx1, vector<vector<int>>& out_mtx);
    ~QRHouseholderMtxMul(){};
    void TileGen(vector<Tile> &tiles, vector<Operation>& ops);
};

class Sub_mtx: public Pattern{
  public:
    Sub_mtx(vector<Operation> &ops, string &in_mn_name, const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, vector<vector<int>> &out_mtx);
};

class SquareAcc_vec: public Pattern{
  public:
    SquareAcc_vec(vector<Operation> &ops, string &in_mn_name, const vector<int> &in_vec, int &out_s);
    ~SquareAcc_vec(){};
    void TileGen(vector<Tile> &tiles, vector<Operation>& ops);
};

class SquareAcc_mtx: public Pattern{
  public:
    SquareAcc_mtx(vector<Operation> &ops, string &in_mn_name, const vector<vector<int>> &in_mtx, vector<int> &out_vec);
};

class DivRoot: public Pattern{
  public:
    DivRoot(vector<Operation> &ops, string &in_mn_name, const vector<int> &in_vec1, const vector<int> &in_vec2, vector<int> &out_vec);
};

class MtxMul: public Pattern{
  public:
    MtxMul(vector<Operation> &ops, string &in_mn_name, const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, vector<vector<int>> &out_mtx);
    ~MtxMul();
    void TileGen(vector<Tile> &tiles, vector<Operation> &ops);
    //void MacroNodeGen_idxorder(vector<MacroNode*> &mns);
    //void MacroNodeGen(vector<MacroNode*> &mns, int mn_dimi, int mn_dimj);
    //void MacroNodeGen(int mn_dimi, int mn_dimj, int mn_diml);
    //void MacroNodeGen_tile(int mn_dimi, int mn_dimj, int mn_diml);
};

//X*Y+Z
class SuborAddMtxMul: public Pattern{
  public:
    SuborAddMtxMul(bool is_add, vector<Operation> &ops, string &in_mn_name, const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, const vector<vector<int>> &in_mtx3, vector<vector<int>> &out_mtx);
    void TileGen(vector<Tile> &tiles, vector<Operation> &ops);
};

class Root_s: public Pattern{
  public:
    Root_s(vector<Operation> &ops, string &in_mn_name, int in_s, int &out_s);
};

class Div_s: public Pattern{
  public:
    Div_s(vector<Operation> &ops, string &in_mn_name, int in_s1, int in_s2, int &out_s);
};

class Jacobi_cs: public Pattern{
  public:
    Jacobi_cs(vector<Operation> &ops, string &in_mn_name, int app, int aqq, int apq, int &c, int &s, int &ns);
};

class Givens_cs: public Pattern{
  public:
    Givens_cs(vector<Operation> &ops, string &in_mn_name, int arp, int arq, bool zero_arp, int &c, int &s, int &ns);
};


#endif


