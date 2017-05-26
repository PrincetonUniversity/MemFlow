#include<iostream>
#include<vector>

#include "MacroNodeTemplate.hpp"
#include "Scheduling.hpp"
#include "ComputationGraph.hpp"

using namespace std;

void MacroNodeTemplate::buildDblks(){
  for(auto &blk: dblks_info){
    dblks[blk.first] = shared_ptr<DataBlock>(new DataBlock(blk.second.matrix_name, blk.second.region_name, blk.second.dimi, blk.second.dimj));
    DblkAddr blk_sp_addr = {blk.second.region_name, 0};
    dblks[blk.first]->setSPAddr(blk_sp_addr);
    dblks[blk.first]->setElementSPAddr(blk.second.elm_spaddr_mode, blk.second.subblk_dimi, blk.second.subblk_dimj);
  }

}

void MacroNodeTemplate::genScheduling(){
  p_cg->SetOpOut();
  for(vector<Pattern*>::iterator p=p_cg->patterns.begin(); p!=p_cg->patterns.end(); p++){
    if(((*p)->name != "load") && ((*p)->name != "store")){
      (*p)->TileGen(p_cg->tiles, p_cg->ops);
    }
  }
  p_cg->BuildTileDep();

  //p_cg->PrintTiles();

  mem = shared_ptr<MemoryTrack>(new MemoryTrack());

  TileScheduling mn_sche(this, MN);
  mn_sche.Scheduling_pipe_debug();
}

MN_mtxmul::MN_mtxmul(bool is_add, int in_m, int in_n, int in_k, map<string, dblk_info>& in_dblks_info){
  name = "mm";

  m = in_m;
  n = in_n;
  k = in_k;

  /*
  dblks_info = in_dblks_info;
  buildDblks();

  ComputationGraph cg(dblks,name);
  cg.CP_Load(m, k, mtx["A"]);
  cg.CP_Load(k, n, mtx["B"]);
  cg.CP_Load(m, n, mtx["Cin"]);
  cg.CP_SuborAddMtxMul(is_add, mtx["A"], mtx["B"], mtx["Cin"], mtx["Cout"]);
  cg.CP_Store(mtx["Cout"]);	
  
  //cg.PrintOps();

  p_cg = &cg;

  genScheduling();
  */

  //model to represent real pipeling
  cycle_length = opti_para.blk_cycles["mm"];
  total_use = opti_para.mntemps["mm"].total_use;
  use_sram = opti_para.mntemps["mm"].use_sram;
  use_pipe = opti_para.mntemps["mm"].use_pipe;
}


MN_LU::MN_LU(int in_n, map<string, dblk_info>& in_dblks_info){
  name = "lu";

  n = in_n;
  
  /*
  dblks_info = in_dblks_info;
  buildDblks();

  ComputationGraph cg(dblks,name);
  cg.CP_Load(n, n, mtx["Ain"]);
  cg.CP_LU(mtx["Ain"], mtx["Lout"], mtx["Uout"]); 
  cg.CP_Store(mtx["Lout"]);
  cg.CP_Store(mtx["Uout"]);

  cg.PrintOps();
 
  p_cg = &cg;

  genScheduling();
  */
  cycle_length = opti_para.blk_cycles["lu"];
  total_use = opti_para.mntemps["lu"].total_use;
  use_sram = opti_para.mntemps["lu"].use_sram;
  use_pipe = opti_para.mntemps["lu"].use_pipe;
}

MN_LUCPL::MN_LUCPL(int in_n, map<string, dblk_info>& in_dblks_info){
  name = "lucpl";

  n = in_n;

  /*
  dblks_info = in_dblks_info;
  buildDblks();

  ComputationGraph cg(dblks, name);
  cg.CP_Load(n,n,mtx["Ain"]);
  cg.CP_Load(n,n,mtx["Uin"]);
  cg.CP_LUCPL(mtx["Ain"],mtx["Uin"],mtx["Lout"]);
  cg.CP_Store(mtx["Lout"]);

  p_cg = &cg;
  
  genScheduling();
  */
  cycle_length = opti_para.blk_cycles["lucpl"];
  total_use = opti_para.mntemps["lucpl"].total_use;
  use_sram = opti_para.mntemps["lucpl"].use_sram;
  use_pipe = opti_para.mntemps["lucpl"].use_pipe;
}

MN_TRS::MN_TRS(int in_n, map<string, dblk_info>& in_dblks_info){
  name = "trs";

  n = in_n;

  /*
  dblks_info = in_dblks_info;
  buildDblks();

  ComputationGraph cg(dblks, name);
  cg.CP_Load(n,n,mtx["Ain"]);
  cg.CP_Load(n,n,mtx["Bin"]);
  cg.CP_TRS(mtx["Ain"],mtx["Bin"],mtx["Xout"]);
  cg.CP_Store(mtx["Xout"]);

  p_cg = &cg;

  genScheduling();
  */
  cycle_length = opti_para.blk_cycles["trs"];
  total_use = opti_para.mntemps["trs"].total_use;
  use_sram = opti_para.mntemps["trs"].use_sram;
  use_pipe = opti_para.mntemps["trs"].use_pipe;
}

MN_SUBMM::MN_SUBMM(int in_m, int in_n, int in_k, map<string, dblk_info>& in_dblks_info){
  
  //model
  cycle_length = opti_para.blk_cycles["submm"];
  total_use = opti_para.mntemps["submm"].total_use;
  use_sram = opti_para.mntemps["submm"].use_sram;
  use_pipe = opti_para.mntemps["submm"].use_pipe;
}


MN_QR::MN_QR(int in_n, map<string,dblk_info>& in_dblks_info){
  name = "qr";

  n = in_n;

  /*
  dblks_info = in_dblks_info;
  buildDblks();  

  ComputationGraph cg(dblks, name);
  cg.CP_Load(n,n,mtx["Ain"]);
  cg.CP_QR(mtx["Ain"],mtx["Qout"],mtx["Rout"]);
  cg.CP_Store(mtx["Qout"]);
  cg.CP_Store(mtx["Rout"]);

  p_cg = &cg;

  genScheduling();
  */
  cycle_length = opti_para.blk_cycles["qr"];
  total_use = opti_para.mntemps["qr"].total_use;
  use_sram = opti_para.mntemps["qr"].use_sram;
  use_pipe = opti_para.mntemps["qr"].use_pipe;
}

MN_QRCPL::MN_QRCPL(int in_n, map<string,dblk_info>& in_dblks_info){
  name = "qrcpl";

  n = in_n;

  /*
  dblks_info = in_dblks_info;
  buildDblks();

  ComputationGraph cg(dblks, name);
  cg.CP_Load(n,n,mtx["Ain"]);
  cg.CP_Load(n,n,mtx["A0in"]);
  cg.CP_QRCPL(mtx["A0in"],mtx["Ain"],mtx["Tout"],mtx["Qout"],mtx["Rout"]);
  cout << "Tout size " << mtx["Tout"].size() << "," << mtx["Tout"][0].size() << endl;
  for(int i=0; i<mtx["Tout"].size(); i++){
    for(int j=0; j<mtx["Tout"][i].size(); j++){
      cout << mtx["Tout"][i][j] << " ";
    }
    cout << endl;
  }
  cout << "Qout size " << mtx["Qout"].size() << "," << mtx["Qout"][0].size() << endl;
  for(int i=0; i<mtx["Qout"].size(); i++){
    for(int j=0; j<mtx["Qout"][i].size(); j++){
      cout << mtx["Qout"][i][j] << " ";
    }
    cout << endl;
  }
  cout << "Rout size " << mtx["Rout"].size() << "," << mtx["Rout"][0].size() << endl;
  for(int i=0; i<mtx["Rout"].size(); i++){
    for(int j=0; j<mtx["Rout"][i].size(); j++){
      cout << mtx["Rout"][i][j] << " ";
    }
    cout << endl;
  }
  cg.CP_Store(mtx["Tout"]);
  cg.CP_Store(mtx["Qout"]);
  cg.CP_Store(mtx["Rout"]);

  p_cg = &cg;

  genScheduling();
  */

  cycle_length = opti_para.blk_cycles["qrcpl"];
  total_use = opti_para.mntemps["qrcpl"].total_use;
  use_sram = opti_para.mntemps["qrcpl"].use_sram;
  use_pipe = opti_para.mntemps["qrcpl"].use_pipe;
}

MN_QRUpdateDiag::MN_QRUpdateDiag(int in_m, int in_n, map<string,dblk_info>& in_dblks_info){
  name = "qrupdatediag";

  m = in_m;
  n = in_n;

  dblks_info = in_dblks_info;
  buildDblks();

  ComputationGraph cg(dblks, name);
  cg.CP_Load(m,n,mtx["Tin"]);
  cg.CP_Load(m,m,mtx["Ain"]);
  cg.CP_QRUpdateDiag(mtx["Tin"],mtx["Ain"],mtx["Aout"]);
  cg.CP_Store(mtx["Aout"]);

  p_cg = &cg;

  genScheduling();
}

MN_QRUpdateTr::MN_QRUpdateTr(int in_n, map<string,dblk_info>& in_dblks_info){
  name = "qrupdatetr";

  n = in_n;

  /*
  dblks_info = in_dblks_info;
  buildDblks();

  ComputationGraph cg(dblks,name);
  cg.CP_Load(n,n,mtx["Qin"]);
  cg.CP_Load(n,n,mtx["Ain"]);
  cg.CP_QRUpdateTr(mtx["Qin"],mtx["Ain"],mtx["Rout"]);
  cg.CP_Store(mtx["Rout"]);

  p_cg = &cg;

  genScheduling();
  */
  cycle_length = opti_para.blk_cycles["qrupdatetr"];
  total_use = opti_para.mntemps["qrupdatetr"].total_use;
  use_sram = opti_para.mntemps["qrupdatetr"].use_sram;
  use_pipe = opti_para.mntemps["qrupdatetr"].use_pipe;
}

MN_QRUpdate::MN_QRUpdate(int in_n, map<string,dblk_info>& in_dblks_info){
  name = "qrupdate";

  n = in_n;

  /*
  dblks_info = in_dblks_info;
  buildDblks();

  ComputationGraph cg(dblks,name);
  cg.CP_Load(n,n,mtx["Qin"]);
  cg.CP_Load(n,n,mtx["Ain"]);
  cg.CP_QRUpdate(mtx["Qin"],mtx["Ain"],mtx["Aout"]);
  cg.CP_Store(mtx["Aout"]);

  p_cg = &cg;

  genScheduling();
  */

  cycle_length = opti_para.blk_cycles["qrupdate"];
  total_use = opti_para.mntemps["qrupdate"].total_use;
  use_sram = opti_para.mntemps["qrupdate"].use_sram;
  use_pipe = opti_para.mntemps["qrupdate"].use_pipe;
}
