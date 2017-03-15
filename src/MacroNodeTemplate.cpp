#include<iostream>
#include<vector>

#include "MacroNodeTemplate.hpp"
#include "Scheduling.hpp"
#include "ComputationGraph.hpp"

using namespace std;

MacroNodeTemplate::MacroNodeTemplate(){
  
}





MN_mtxmul::MN_mtxmul(int in_m, int in_n, int in_k, bool sche){
  cout << endl <<  "Macro Node Template for mtxmul " << endl;
  name = "mm";

  m = in_m;
  n = in_n;
  k = in_k;

  map<string, shared_ptr<DataBlock>> dblks;
  dblks["MM_A"] = shared_ptr<DataBlock>(new DataBlock("MM_A","MM_A",m,k));
  dblks["MM_B"] = shared_ptr<DataBlock>(new DataBlock("MM_B","MM_B",k,n));
  dblks["MM_C"] = shared_ptr<DataBlock>(new DataBlock("MM_C","MM_C",m,n));
  for(auto &i: dblks){
    DblkAddr blk_sp_addr = {i.second->region_name, 0};
    i.second->setSPAddr(blk_sp_addr);
  }

  ComputationGraph cg(dblks,name);
  cg.CP_Load(m, k, mtx["A"]);
  cg.CP_Load(k, n, mtx["B"]);
  cg.CP_Load(m, n, mtx["Cin"]);
  cg.CP_SuborAddMtxMul(true, mtx["A"], mtx["B"], mtx["Cin"], mtx["Cout"]);
  cg.CP_Store(mtx["Cout"]);	

  cg.PrintOps();

  cg.SetOpOut();
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if(((*p)->name != "load") && ((*p)->name != "store")){
      (*p)->TileGen(cg.tiles, cg.ops);
    }
  }
  cg.BuildTileDep();

  cg.PrintTiles();

  p_cg = &cg;

  mem = shared_ptr<MemoryTrack>(new MemoryTrack());

  if(sche){
    TileScheduling mn_sche(this, MN);
    mn_sche.Scheduling_pipe_debug();
  }
}


MN_LU::MN_LU(int in_n, bool sche){
  name = "lu";
  cout << "mn temp " << name << endl;

  n = in_n;
  
  map<string, shared_ptr<DataBlock>> dblks;
  dblks["LU_L_Tr"] = shared_ptr<DataBlock>(new DataBlock("LU_L_Tr","LU_L",n,n));
  dblks["LU_U_Tr"] = shared_ptr<DataBlock>(new DataBlock("LU_U_Tr","LU_U",n,n));
  dblks["LU_A"] = shared_ptr<DataBlock>(new DataBlock("LU_A","LU_A",n,n));
  for(auto &i: dblks){
    DblkAddr blk_sp_addr = {i.second->region_name, 0};
    i.second->setSPAddr(blk_sp_addr);
  }

  ComputationGraph cg(dblks,name);
  cg.CP_Load(n, n, mtx["Ain"]);
  cg.CP_LU(mtx["Ain"], mtx["Lout"], mtx["Uout"]); 
  cg.CP_Store(mtx["Lout"]);
  cg.CP_Store(mtx["Uout"]);

  cg.PrintOps();

  cg.SetOpOut();
  //cout << "finish set op out" << endl;
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if(((*p)->name != "load") && ((*p)->name != "store")){
      (*p)->TileGen(cg.tiles, cg.ops);
    }
  }
  cg.BuildTileDep();

  cg.PrintTiles();

  //ops = cg.ops;
  //tiles = cg.tiles;
  p_cg = &cg;

  mem = shared_ptr<MemoryTrack>(new MemoryTrack());

  if(sche){
    TileScheduling mn_sche(this, MN);
    mn_sche.Scheduling_pipe_debug();
  }
}

MN_LUCPL::MN_LUCPL(int in_n, bool sche){
  name = "lucpl";
  cout << "mn temp " << name << endl;

  n = in_n;

  map<string, shared_ptr<DataBlock>> dblks;
  dblks["LUCPL_A"] = shared_ptr<DataBlock>(new DataBlock("LUCPL_A","LU_A",n,n));
  dblks["LUCPL_U_Tr"] = shared_ptr<DataBlock>(new DataBlock("LUCPL_U_Tr","LU_U",n,n));
  dblks["LUCPL_L"] = shared_ptr<DataBlock>(new DataBlock("LUCPL_L","LU_L",n,n));
  for(auto &i: dblks){
    DblkAddr blk_sp_addr = {i.second->region_name, 0};
    i.second->setSPAddr(blk_sp_addr);
  }

  ComputationGraph cg(dblks, name);
  cg.CP_Load(n,n,mtx["Ain"]);
  cg.CP_Load(n,n,mtx["Uin"]);
  cg.CP_LUCPL(mtx["Ain"],mtx["Uin"],mtx["Lout"]);
  cg.CP_Store(mtx["Lout"]);

  cg.PrintOps();
  
  cg.SetOpOut();
  //cout << "finish set op out" << endl;
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if(((*p)->name != "load") && ((*p)->name != "store")){
      (*p)->TileGen(cg.tiles, cg.ops);
    }
  }
  cg.BuildTileDep();

  cg.PrintTiles();

  //ops = cg.ops;
  //tiles = cg.tiles;
  p_cg = &cg;

  mem = shared_ptr<MemoryTrack>(new MemoryTrack());

  if(sche){
    TileScheduling mn_sche(this, MN);
    mn_sche.Scheduling_pipe_debug();
  }
}

MN_TRS::MN_TRS(int in_n, bool sche){
  name = "trs";
  cout << "mn temp " << name << endl;

  n = in_n;

  map<string, shared_ptr<DataBlock>> dblks;
  dblks["TRS_A_Tr"] = shared_ptr<DataBlock>(new DataBlock("TRS_A_Tr","LU_L",n,n));
  dblks["TRS_B"] = shared_ptr<DataBlock>(new DataBlock("TRS_B","LU_A",n,n));
  dblks["TRS_X"] = shared_ptr<DataBlock>(new DataBlock("TRS_X","LU_U",n,n));
  dblks["TRS_Brow"] = shared_ptr<DataBlock>(new DataBlock("TRS_Brow","LU_U",1,n));
  for(auto &i: dblks){
    DblkAddr blk_sp_addr = {i.second->region_name, 0};
    i.second->setSPAddr(blk_sp_addr);
  }

  ComputationGraph cg(dblks, name);
  cg.CP_Load(n,n,mtx["Ain"]);
  cg.CP_Load(n,n,mtx["Bin"]);
  cg.CP_TRS(mtx["Ain"],mtx["Bin"],mtx["Xout"]);
  cg.CP_Store(mtx["Xout"]);

  cg.PrintOps();
  
  cg.SetOpOut();
  //cout << "finish set op out" << endl;
  for(vector<Pattern*>::iterator p=cg.patterns.begin(); p!=cg.patterns.end(); p++){
    if(((*p)->name != "load") && ((*p)->name != "store")){
      (*p)->TileGen(cg.tiles, cg.ops);
    }
  }
  cg.BuildTileDep();

  cg.PrintTiles();
  
  p_cg = &cg;

  mem = shared_ptr<MemoryTrack>(new MemoryTrack());

  if(sche){
    TileScheduling mn_sche(this, MN);
    mn_sche.Scheduling_pipe_debug();
  }
}


