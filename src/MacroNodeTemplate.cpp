#include<iostream>
#include<vector>

#include "MacroNodeTemplate.hpp"
#include "Scheduling.hpp"
#include "ComputationGraph.hpp"

using namespace std;

MacroNodeTemplate::MacroNodeTemplate(string in_name){
  name = in_name;

  A_base = 0;
  B_base = 0;
  Cin_base = 0;
  Cout_base = 0;
}

MacroNodeTemplate::MacroNodeTemplate(vector<Operation> &in_ops, vector<Tile> &in_tiles, string in_name){
  ops = in_ops;
  tiles = in_tiles;
  name = in_name;

  A_base = 0;
  B_base = 0;
  Cin_base = 0;
  Cout_base = 0;
}

void MacroNodeTemplate::GetScheduling(){
  TileScheduling sche(*this, MN);
  sche.Scheduling();
  sche.Testing();
  sche.PrintScheduling();
  sche.PrintPerf();
}

void MacroNodeTemplate::MN_load(int width, bool sche){

  vector<vector<int>> in_mtx;

  ComputationGraph cg;
  cg.CP_Load(1, width, in_mtx);
  cg.CP_Store(in_mtx);

  cg.SetOpOut();
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if((*p)->name == "load_matrix"){
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
  mem = shared_ptr<MemoryTrack>(new MemoryTrack(ops, true));

  if(sche){
    cout << "start mn template scheduling" << endl;
    TileScheduling mn_sche(*this, MN);
    mn_sche.Scheduling();
    mn_sche.PrintPerf();
    cycle_length = mn_sche.last_cycle+1;
    mem->getMaxNumLive();
  }
  cout << "finish mn template load" << endl;
}

void MacroNodeTemplate::MN_mtxmul(int m, int n, int k, bool sche){
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
  mem = shared_ptr<MemoryTrack>(new MemoryTrack(ops, true));
 
  //determine io op bank
  for(int i=0; i<Ain.size(); i++){
    for(int j=0; j<Ain[i].size(); j++){
      int idx = i*Ain[0].size()+j;
      ioop_addr[Ain[i][j]][0] = idx%mem->num_bank;
      ioop_addr[Ain[i][j]][1] = A_base+idx/mem->num_bank;
    }
  }
  for(int i=0; i<Bin.size(); i++){
    for(int j=0; j<Bin[i].size(); j++){
      int idx = i*Bin[0].size()+j;
      ioop_addr[Bin[i][j]][0] = idx%mem->num_bank;
      ioop_addr[Bin[i][j]][1] = B_base+idx/mem->num_bank;
    }
  }
  for(int i=0; i<Cin.size(); i++){
    for(int j=0; j<Cin[i].size(); j++){
      int idx = i*Cin[0].size()+j;
      ioop_addr[Cin[i][j]][0] = idx%mem->num_bank;
      ioop_addr[Cin[i][j]][1] = Cin_base+idx/mem->num_bank;
    }
  }
  for(int i=0; i<Cout.size(); i++){
    for(int j=0; j<Cout[i].size(); j++){
      int idx = i*Cout[0].size()+j;
      ioop_addr[Cout[i][j]][0] = idx%mem->num_bank;
      ioop_addr[Cout[i][j]][1] = Cout_base+idx/mem->num_bank;
    }
  }

  if(sche){
    TileScheduling mn_sche(*this, MN);
    mn_sche.Scheduling_dblks();
    //mn_sche.PrintPerf();
    //cycle_length = mn_sche.last_cycle+1;
    //mem->getMaxNumLive();
  }

  /*
  for(int i=0; i<Ain.size(); i++){
    for(int j=0; j<Ain[i].size(); j++){
      //op: Ain[i][j]
      int op = Ain[i][j];
      int first_cycle = inop_firstread[op];
      Ain_loc[op] = mem->op_in_bank[op][first_cycle];
      int last_cycle = op_in_cycle[op];
      Aout_loc[op] = mem->op_in_bank[op][last_cycle];
    }
  }
  for(int i=0; i<Bin.size(); i++){
    for(int j=0; j<Bin[i].size(); j++){
      int op = Bin[i][j];
      int first_cycle = inop_firstread[op];
      Bin_loc[op] = mem->op_in_bank[op][first_cycle];
      int last_cycle = op_in_cycle[op];
      Bout_loc[op] = mem->op_in_bank[op][last_cycle];
    }
  }
  for(int i=0; i<Cin.size(); i++){
    for(int j=0; j<Cin[i].size(); j++){
      int op = Cin[i][j];
      int first_cycle = inop_firstread[op];
      Cin_loc[op] = mem->op_in_bank[op][first_cycle];
    }
  }
  for(int i=0; i<Cout.size(); i++){
    for(int j=0; j<Cout[i].size(); j++){
      int op = Cout[i][j];
      int last_cycle = op_in_cycle[op];
      Cout_loc[op] = mem->op_in_bank[op][last_cycle];
    }
  }
  cout << "Ain_loc" << endl;
  for(auto i=Ain_loc.begin(); i!=Ain_loc.end(); i++){
    cout << i->first << ": " << i->second[0] << ", " << i->second[1] << endl;
  }
  cout << "Bin_loc" << endl;
  for(auto i=Bin_loc.begin(); i!=Bin_loc.end(); i++){
    cout << i->first << ": " << i->second[0] << ", " << i->second[1] << endl;
  }
  cout << "Cin_loc" << endl;
  for(auto i=Cin_loc.begin(); i!=Cin_loc.end(); i++){
    cout << i->first << ": " << i->second[0] << ", " << i->second[1] << endl;
  }
  cout << "Aout_loc" << endl;
  for(auto i=Aout_loc.begin(); i!=Aout_loc.end(); i++){
    cout << i->first << ": " << i->second[0] << ", " << i->second[1] << endl;
  }
  cout << "Bout_loc" << endl;
  for(auto i=Bout_loc.begin(); i!=Bout_loc.end(); i++){
    cout << i->first << ": " << i->second[0] << ", " << i->second[1] << endl;
  }
  cout << "Cout_loc" << endl;
  for(auto i=Cout_loc.begin(); i!=Cout_loc.end(); i++){
    cout << i->first << ": " << i->second[0] << ", " << i->second[1] << endl;
  }
*/

}

void MacroNodeTemplate::MN_store(int width, bool sche){
  vector<vector<int>> mtx;

  ComputationGraph cg;
  cg.CP_Load(1, width, mtx);
  cg.CP_Store(mtx);

  cg.SetOpOut();
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if((*p)->name == "store_matrix"){
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
  mem = shared_ptr<MemoryTrack>(new MemoryTrack(ops, true));

  if(sche){
    TileScheduling mn_sche(*this, MN);
    mn_sche.Scheduling();
    mn_sche.PrintPerf();
    cycle_length = mn_sche.last_cycle+1;
    mem->getMaxNumLive();
  }
}


void MacroNodeTemplate::MN_2load_mtxmul(int m, int n, int k, bool sche){
  vector<vector<int>> in_mtx1;
  vector<vector<int>> in_mtx2;
  vector<vector<int>> in_mtx3;
  vector<vector<int>> out_mtx;

  ComputationGraph cg;
  cg.CP_Load(m, k, in_mtx1);
  cg.CP_Load(k, n, in_mtx2);
  cg.CP_Load(m, n, in_mtx3);
  cg.CP_MtxMulAdd(in_mtx1, in_mtx2, in_mtx3, out_mtx);
  cg.CP_Store(out_mtx);	

  cg.SetOpOut();
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if((p-cg.patterns.begin()==0) || (p-cg.patterns.begin()==1) || (p-cg.patterns.begin()==3)){
      (*p)->TileGen(cg.tiles, cg.ops);
    }
  }
  cg.BuildTileDep();

  cout << endl << "MacroNode 2 load + matrix multiplication: " << endl;
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
  mem = shared_ptr<MemoryTrack>(new MemoryTrack(ops, true));
  
  if(sche){
    TileScheduling mn_sche(*this, MN);
    mn_sche.Scheduling();
    mn_sche.PrintPerf();
    cycle_length = mn_sche.last_cycle+1;
    mem->getMaxNumLive();
  }

}

void MacroNodeTemplate::MN_load1_mtxmul(int m, int n, int k, bool sche){
  vector<vector<int>> in_mtx1;
  vector<vector<int>> in_mtx2;
  vector<vector<int>> in_mtx3;
  vector<vector<int>> out_mtx;

  ComputationGraph cg;
  cg.CP_Load(m, k, in_mtx1);
  cg.CP_Load(k, n, in_mtx2);
  cg.CP_Load(m, n, in_mtx3);
  cg.CP_MtxMulAdd(in_mtx1, in_mtx2, in_mtx3, out_mtx);
  cg.CP_Store(out_mtx);	

  cg.SetOpOut();
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if((p-cg.patterns.begin()==0) || (p-cg.patterns.begin()==3)){
      (*p)->TileGen(cg.tiles, cg.ops);
    }
  }
  cg.BuildTileDep();

  cout << endl << "MacroNode load + matrix multiplication: " << endl;
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
  mem = shared_ptr<MemoryTrack>(new MemoryTrack(ops, true));
  
  if(sche){
    TileScheduling mn_sche(*this, MN);
    mn_sche.Scheduling();
    mn_sche.PrintPerf();
    cycle_length = mn_sche.last_cycle+1;
    mem->getMaxNumLive();
  }
}

void MacroNodeTemplate::MN_load2_mtxmul(int m, int n, int k, bool sche){
  vector<vector<int>> in_mtx1;
  vector<vector<int>> in_mtx2;
  vector<vector<int>> in_mtx3;
  vector<vector<int>> out_mtx;

  ComputationGraph cg;
  cg.CP_Load(m, k, in_mtx1);
  cg.CP_Load(k, n, in_mtx2);
  cg.CP_Load(m, n, in_mtx3);
  cg.CP_MtxMulAdd(in_mtx1, in_mtx2, in_mtx3, out_mtx);
  cg.CP_Store(out_mtx);	

  cg.SetOpOut();
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if((p-cg.patterns.begin()==1) || (p-cg.patterns.begin()==3)){
      (*p)->TileGen(cg.tiles, cg.ops);
    }
  }
  cg.BuildTileDep();

  cout << endl << "MacroNode load + matrix multiplication: " << endl;
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
  mem = shared_ptr<MemoryTrack>(new MemoryTrack(ops, true));
  
  if(sche){
    TileScheduling mn_sche(*this, MN);
    mn_sche.Scheduling();
    mn_sche.PrintPerf();
    cycle_length = mn_sche.last_cycle+1;
    mem->getMaxNumLive();
  }
}
