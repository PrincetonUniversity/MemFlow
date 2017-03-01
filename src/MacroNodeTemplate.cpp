#include<iostream>
#include<vector>

#include "MacroNodeTemplate.hpp"
#include "Scheduling.hpp"
#include "ComputationGraph.hpp"

using namespace std;

MacroNodeTemplate::MacroNodeTemplate(string in_name){
  name = in_name;

  a_blk = {"A", 0};
  b_blk = {"B", 0};
  c_blk = {"C", 0};
}

MacroNodeTemplate::MacroNodeTemplate(vector<Operation> &in_ops, vector<Tile> &in_tiles, string in_name){
  ops = in_ops;
  tiles = in_tiles;
  name = in_name;

  a_blk = {"A", 0};
  b_blk = {"B", 0};
  c_blk = {"C", 0};
}


void MacroNodeTemplate::MN_mtxmul(int in_m, int in_n, int in_k, bool sche){
  cout << endl <<  "Macro Node Template for mtxmul " << endl;
  m = in_m;
  n = in_n;
  k = in_k;

  tile_m = in_m;
  tile_n = in_n;
  tile_k = in_k/ComputeBlockLib::cbs["mul_acc"]->max_depth;

  ComputationGraph cg;
  cg.CP_Load(m, k, Ain);
  cg.CP_Load(k, n, Bin);
  cg.CP_Load(m, n, Cin);
  cg.CP_MtxMulAdd(Ain, Bin, Cin, Cout);
  cg.CP_Store(Cout);	

  cg.SetOpOut();
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if((*p)->name == "matrix_multiplication_addition"){
      (*p)->TileGen(cg.tiles, cg.ops);
    }
  }
  cg.BuildTileDep();

  cg.PrintTiles();

  ops = cg.ops;
  tiles = cg.tiles;

  vector<int> ops;
  for(vector<Tile>::iterator t=cg.tiles.begin(); t!=cg.tiles.end(); t++){
    for(map<int,int>::iterator in_op=t->livein_ops.begin(); in_op!=t->livein_ops.end(); in_op++){
      ops.push_back(in_op->first);
    }
    for(vector<int>::iterator out_op=t->liveout_ops.begin(); out_op!=t->liveout_ops.end(); out_op++){
      ops.push_back(*out_op);
    }
  }
  mem = shared_ptr<MemoryTrack>(new MemoryTrack());
 
  cout << "input data bank allocation " << endl;
  //determine io op bank
  cout << endl << "A" << endl;
  for(int i=0; i<Ain.size(); i++){
    for(int j=0; j<Ain[i].size(); j++){
      array<int,2> a_addr = global_sp->getAddr_a_ele(a_blk, Ain.size(), Ain[0].size(), i, j);
      cout << "A " << i << " " << j << ": " << a_addr[0] << " " << a_addr[1] << endl;
      ioop_addr[Ain[i][j]] = a_addr;
    }
  }
  cout << endl << "B" << endl;
  for(int i=0; i<Bin.size(); i++){
    for(int j=0; j<Bin[i].size(); j++){
      array<int,2> b_addr = global_sp->getAddr_b_ele(b_blk, Bin.size(), Bin[0].size(), i, j);
      cout << "B " << i << " " << j << ": " << b_addr[0] << " " << b_addr[1] << endl;
      ioop_addr[Bin[i][j]] = b_addr;
    }
  }
  cout << endl << "C" << endl;
  for(int i=0; i<Cin.size(); i++){
    for(int j=0; j<Cin[i].size(); j++){
      array<int,2> c_addr = global_sp->getAddr_c_ele(c_blk, Cin.size(), Cin[0].size(), i, j, 5);
      cout << "C " << i << " " << j << ": " << c_addr[0] << " " << c_addr[1] << endl;
      ioop_addr[Cin[i][j]] = c_addr;
    }
  }
  for(int i=0; i<Cout.size(); i++){
    for(int j=0; j<Cout[i].size(); j++){
      ioop_addr[Cout[i][j]] = ioop_addr[Cin[i][j]];
    }
  }
  cout << "finish assign addr" << endl;

  if(sche){
    TileScheduling mn_sche(*this, MN);
    mn_sche.Scheduling_pipe();
  }
}

