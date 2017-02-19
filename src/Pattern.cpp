#include<iostream>
#include<vector>
#include<array>
#include<queue>
#include<string>
#include<map>
#include"math.h"
#include<memory>

#include "Pattern.hpp"
#include "Setting.hpp"

using namespace std;

//map<string, int> num_cb = {
//{"load", 10},
//{"store", 10},
//{"sub", 10},
//{"div", 10},
//{"div_root", 10}
//};

vector<MacroNodeTemplate*> mn_temps;


void Transpose_mtx(vector<vector<int>> &in_mtx, vector<vector<int>> &out_mtx);

// class Pattern
void Pattern::PrintOps(vector<Operation> &ops){

  cout << endl;
  cout << name << endl;
  cout << description << endl;
  cout << cp << endl;
  for(vector<PatternUnit>::iterator pu=pus.begin(); pu!=pus.end(); pu++){
    for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
      cout << "op " << *op << ": ";
      for(vector<int>::iterator i=ops[*op].in.begin(); i!=ops[*op].in.end(); i++){
	cout << *i << " ";
      }
      cout << ops[*op].fu.name << endl;
      cout << "   out: ";
      for(set<int>::iterator i=ops[*op].out.begin(); i!=ops[*op].out.end(); i++){
	cout << *i << " ";
      }
      cout << endl;
      cout << "   tile: " << ops[*op].tile << endl;
    }
  }

}

// use cb defined in pattern to tile up the pattern
void Pattern::TileGen(vector<Tile> &tiles, vector<Operation> &ops){

  //pattern unit wait to be added to tiles
  list<PatternUnit> wait;

  //pattern units that ready to be added to a new compute block
  queue<PatternUnit> ready;

  //initialize these two
  set<int> ops_wait;
  for(vector<PatternUnit>::iterator pu=pus.begin(); pu!=pus.end(); pu++){
    for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
      ops_wait.insert(*op);
    }
  }

  for(vector<PatternUnit>::iterator pu=pus.begin(); pu!=pus.end(); pu++){
    //examine each pattern unit
    int is_ready = 1;

    set<int> ops_inpu;
    for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
      ops_inpu.insert(*op);
    }

    //examine all inputs of all ops in the pattern unit
    for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
      for(vector<int>::iterator op_in=ops[*op].in.begin(); op_in!=ops[*op].in.end(); op_in++){
	// input op idx: *op_in
	if(ops_wait.find(*op_in)!=ops_wait.end() && ops_inpu.find(*op_in)==ops_inpu.end()){
	  is_ready = 0;
	}
      }	      
    }

    if(is_ready){
      //put this pattern units into ready queue
      ready.push(*pu);
    }
    else{
      wait.push_back(*pu);
    }
  }


  while(!ready.empty()){
    Tile tile(cb);
    int count_pu_added = 0;
    while(count_pu_added<cb->width && !ready.empty()){
      PatternUnit pu_toadd = ready.front();
      ready.pop();

      //add the whole pu into tile and update ops_wait
      for(vector<int>::iterator op=pu_toadd.ops.begin(); op!=pu_toadd.ops.end(); op++){
	tile.ops.push_back(*op);
	ops[*op].tile = tiles.size();
	ops_wait.erase(*op);

      }

      count_pu_added++;
    }

    //add other pu that depend on ready
    while(count_pu_added<cb->width*cb->max_depth && !wait.empty()){
      //check each pu in wait
      vector<list<PatternUnit>::iterator> pus_toadd;
      for(list<PatternUnit>::iterator pu=wait.begin(); pu!=wait.end(); pu++){
	int can_add = 1;

	set<int> ops_inpu;
	for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
	  ops_inpu.insert(*op);
	}

	for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
	  for(vector<int>::iterator op_in=ops[*op].in.begin(); op_in!=ops[*op].in.end(); op_in++){
	    if(ops_wait.find(*op_in)!=ops_wait.end() && ops_inpu.find(*op_in)==ops_inpu.end()){
	      can_add = 0;
	    }
	  }
	}

	if(can_add){
	  pus_toadd.push_back(pu);
	}
      }

      if(pus_toadd.empty()){
	break;
      }
      else{
	for(size_t i=0; i<pus_toadd.size(); i++){
	  for(vector<int>::iterator op=pus_toadd[i]->ops.begin(); op!=pus_toadd[i]->ops.end(); op++){
	    tile.ops.push_back(*op);
	    ops[*op].tile = tiles.size();
	    ops_wait.erase(*op);
	  }
	  wait.erase(pus_toadd[i]);
	  count_pu_added++;
	}

      }
    }

    //real depth of used cb
    if(cb->max_depth != 1){
      int cb_real_depth = count_pu_added / cb->width;
      tile.latency = cb->GetRealLatency(cb_real_depth);
    }
    else{
      tile.latency = cb->latency;
    }

    //update ready queue
    vector<list<PatternUnit>::iterator> pus_ready;
    for(list<PatternUnit>::iterator pu=wait.begin(); pu!=wait.end(); pu++){
      int is_ready = 1;

      set<int> ops_inpu;
      for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
	ops_inpu.insert(*op);
      }

      for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
	for(vector<int>::iterator op_in=ops[*op].in.begin(); op_in!=ops[*op].in.end(); op_in++){
	  if(ops_wait.find(*op_in)!=ops_wait.end() && ops_inpu.find(*op_in)==ops_inpu.end()){
	    is_ready = 0;
	  }
	}
      }

      if(is_ready){
	ready.push(*pu);
	pus_ready.push_back(pu);
      }
    }
    for(size_t i=0; i<pus_ready.size(); i++){
      wait.erase(pus_ready[i]);
    }

    tile.pattern_name = name;
    tile.cpsection_name = cp;
    tiles.push_back(tile);
    tiles_idx.push_back(tiles.size()-1);

  }

}

/*
   void Pattern::MacroNodeGen(int mn_width, int mn_depth, vector<Tile> &tiles, MacroNode &mn_template, vector<MacroNode>& mns){
   queue<int> ready;
   list<int> wait;
   set<int> added;

//initialize these two
for(vector<int>::iterator t=tiles_idx.begin(); t!=tiles_idx.end(); t++){
//tile idx: *t
bool is_ready = true;
for(vector<int>::iterator pred_t=tiles[*t].pred_tiles.begin(); pred_t!=tiles[*t].pred_tiles.end(); pred_t++){
if(*pred_t >= tiles_idx[0]){
is_ready = false;
break;
}
}
if(is_ready){
ready.push(*t);
}
else{
wait.push_back(*t);
}
}

while(!ready.empty()){
MacroNode mn(mn_template);

int count = 0;
while(count<mn_width && !ready.empty()){
int tileidx_in_mn = mn.tile_map.size();
mn.tile_map[ready.front()] = tileidx_in_mn;

map<int,int>::iterator in_op1 = tiles[ready.front()].livein_ops.begin();
for(map<int,int>::iterator in_op2=mn_template.tiles[tileidx_in_mn].livein_ops.begin(); in_op2!=mn_template.tiles[tileidx_in_mn].livein_ops.end(); in_op2++){
if(mn.op_map.find(in_op2->first) == mn.op_map.end()){
mn.op_map[in_op2->first] = in_op1->first;
}
in_op1++;
}
vector<int>::iterator out_op1 = tiles[ready.front()].liveout_ops.begin();
for(vector<int>::iterator out_op2=mn_template.tiles[tileidx_in_mn].liveout_ops.begin(); out_op2!=mn_template.tiles[tileidx_in_mn].liveout_ops.end(); out_op2++){
if(mn.op_map.find(*out_op2) == mn.op_map.end()){
mn.op_map[*out_op2] = *out_op1;
}
out_op1++;
}

added.insert(ready.front());
ready.pop();
count++;
}

while(count<mn_width*mn_depth && !wait.empty()){
vector<list<int>::iterator> tile_toadd;
for(list<int>::iterator t=wait.begin(); t!=wait.end(); t++){
//tile: *t;
bool can_add = true;
for(vector<int>::iterator pred_t=tiles[*t].pred_tiles.begin(); pred_t!=tiles[*t].pred_tiles.end(); pred_t++){
if((*pred_t >= tiles_idx[0]) && (added.find(*pred_t) == added.end())){
can_add = false;
break;
}
}	
if(can_add){
tile_toadd.push_back(t);
}
}

if(tile_toadd.empty()){
break;
}
else{
  for(int i=0; i<tile_toadd.size(); i++){  
    int tileidx_in_mn = mn.tile_map.size(); 
    mn.tile_map[*tile_toadd[i]] = tileidx_in_mn;

    map<int,int>::iterator in_op1 = tiles[ready.front()].livein_ops.begin();
    for(map<int,int>::iterator in_op2=mn_template.tiles[tileidx_in_mn].livein_ops.begin(); in_op2!=mn_template.tiles[tileidx_in_mn].livein_ops.end(); in_op2++){
      if(mn.op_map.find(in_op2->first) == mn.op_map.end()){
	mn.op_map[in_op2->first] = in_op1->first;
      }
      in_op1++;
    }
    vector<int>::iterator out_op1 = tiles[ready.front()].liveout_ops.begin();
    for(vector<int>::iterator out_op2=mn_template.tiles[tileidx_in_mn].liveout_ops.begin(); out_op2!=mn_template.tiles[tileidx_in_mn].liveout_ops.end(); out_op2++){
      if(mn.op_map.find(*out_op2) == mn.op_map.end()){
	mn.op_map[*out_op2] = *out_op1;
      }
      out_op1++;
    }

    added.insert(*tile_toadd[i]);
    wait.erase(tile_toadd[i]);
    count++;
  }
}

}

//update ready
vector<list<int>::iterator> tile_ready;
for(list<int>::iterator t=wait.begin(); t!=wait.end(); t++){
  bool is_ready = true;
  for(vector<int>::iterator pred_t=tiles[*t].pred_tiles.begin(); pred_t!=tiles[*t].pred_tiles.end(); pred_t++){
    if((*pred_t >= tiles_idx[0]) && (added.find(*pred_t) == added.end())){
      is_ready = false;
      break;
    }
  }
  if(is_ready){
    tile_ready.push_back(t);
    ready.push(*t);
  }
}
for(int i=0; i<tile_ready.size(); i++){
  wait.erase(tile_ready[i]);
}

mns.push_back(mn);
}
}
*/


//-----------------------------------------------------------------------------------------

// Load vector with size n
Load_vec::Load_vec(vector<Operation> &ops, string &in_cp, int n, vector<int> &out_vec){

  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(int i=1; i<=n; i++){
    PatternUnit pu;

    pre_idx++;
    Operation op;
    op.idx = pre_idx;
    op.fu = FunctionUnitLib::LOAD;
    ops.push_back(op);
    pu.ops.push_back(op.idx);
    out_vec.push_back(op.idx);

    pus.push_back(pu);
  }

  name = "load_vector";

  cb = ComputeBlockLib::cbs["load"];
}

// Load matrix with size mxn
Load_mtx::Load_mtx(vector<Operation> &ops, string &in_cp, int in_m, int in_n, vector<vector<int>> &out_mtx){
  m = in_m;
  n = in_n;

  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(int i=1; i<=m; i++){
    vector<int> row;
    for(int j=1; j<=n; j++){
      PatternUnit pu;

      pre_idx++;
      Operation op;
      op.idx = pre_idx;
      op.fu = FunctionUnitLib::LOAD;
      ops.push_back(op);
      pu.ops.push_back(op.idx);
      row.push_back(op.idx);

      pus.push_back(pu);
    }
    out_mtx.push_back(row);
  }
  name = "load_matrix";

  cb = ComputeBlockLib::cbs["load"];
}


Load_mtx::~Load_mtx(){
  //for(auto it=mns.begin(); it!=mns.end(); it++){
  //  delete *it;
  //}
}

void Load_mtx::TileGen(vector<Tile> &tiles, vector<Operation> &ops){
  int num_tile = (pus.size()%cb->width==0)?pus.size()/cb->width:pus.size()/cb->width+1;

  int tile_idx_start = tiles.size();
  for(int i=0; i<num_tile; i++){
    Tile tile(cb);
    tile.latency = cb->latency;
    tile.pattern_name = name;
    tile.cpsection_name = cp;
    tiles.push_back(tile);
    tiles_idx.push_back(tiles.size()-1);
  }

  for(vector<PatternUnit>::iterator pu=pus.begin(); pu!=pus.end(); pu++){
    int idx = pu-pus.begin();
    int tile_idx = tile_idx_start + int(idx/cb->width);
    //add pu to the tiles[tile_idx];
    for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
      tiles[tile_idx].ops.push_back(*op);
      ops[*op].tile = tile_idx;
    }
  }

  tile_m = m;
  tile_n = n;

  cout << "pattern dimension (pu): 1 x " << pus.size() << endl;
  cout << "pattern dimension (tile): 1 x " << num_tile << endl;
}


void Load_mtx::MacroNodeGen(int mn_dimi, int mn_dimj, int mn_diml){

  mn_m = (m%mn_dimi == 0)?m/mn_dimi: m/mn_dimi+1;
  mn_n = (n%mn_dimj == 0)?n/mn_dimj: n/mn_dimj+1;

  cout << "m " << m << endl;
  cout << "n " << n << endl;
  cout << "mn_dimi " << mn_dimi << endl;
  cout << "mn_dimj " << mn_dimj << endl;
  cout << "mn_m " << mn_m << endl;
  cout << "mn_n " << mn_n << endl;

  for(int mn_i=0; mn_i<mn_m; mn_i++){
    for(int mn_j=0; mn_j<mn_n; mn_j++){
      cout << endl << "dblk " << dblks.size() << endl;
      shared_ptr<DataBlock> dblk(new DataBlock(dblks.size()));
      dblks.push_back(dblk);

      //add ops
      for(int i=mn_i*mn_dimi; i<(mn_i+1)*mn_dimi; i++){
        for(int j=mn_j*mn_dimj; j<(mn_j+1)*mn_dimj; j++){
	  //pu idx
	  int pu_idx = i*n+j;
	  dblk->ops.push_back(pus[pu_idx].ops[0]);
	  cout << pus[pu_idx].ops[0] << endl;
	}
      }
    }	
  }

}

//void Load_mtx::MacroNodeGen(vector<MacroNode*> &mns, int mn_dimi, int mn_dimj){
void Load_mtx::MacroNodeGen_tile(int mn_dimi, int mn_dimj, int mn_diml){

  cout << "in load mtx mn generator " << endl;

  bool main_blk = (m/mn_dimi != 0) && (n/mn_dimj != 0);
  bool rightcol_blk = (m/mn_dimi != 0) && (n%mn_dimj != 0);
  bool bottomrow_blk = (n/mn_dimj != 0) && (m%mn_dimi != 0);
  bool cornor_blk = (m%mn_dimi != 0) && (n%mn_dimj != 0);

  MacroNodeTemplate* mn_temp1;
  MacroNodeTemplate* mn_temp2;
  MacroNodeTemplate* mn_temp3;
  MacroNodeTemplate* mn_temp4;
  
  if(main_blk){
    cout << "mn temp for main blk" << endl;
    mn_temp1 = new MacroNodeTemplate("load mtx");
    mn_temp1->MN_load(mn_dimi*mn_dimj, true);
    mn_temps.push_back(mn_temp1);
  }
  if(rightcol_blk){
    cout << "mn temp for rightcol blk" << endl;
    mn_temp2 = new MacroNodeTemplate("load mtx");
    mn_temp2->MN_load(mn_dimi*(n%mn_dimj), true);
    mn_temps.push_back(mn_temp2);
  }
  if(bottomrow_blk){
    cout << "mn temp for bottomrow blk" << endl;
    mn_temp3 = new MacroNodeTemplate("load mtx");
    mn_temp3->MN_load((m%mn_dimi)*mn_dimj, true);
    mn_temps.push_back(mn_temp3);
  }
  if(cornor_blk){
    cout << "mn temp for cornor blk" << endl;
    mn_temp4 = new MacroNodeTemplate("load mtx");
    mn_temp4->MN_load((m%mn_dimi)*(n%mn_dimj), true);
    mn_temps.push_back(mn_temp4);
  }

  mn_m = (m%mn_dimi == 0)?m/mn_dimi: m/mn_dimi+1;
  mn_n = (n%mn_dimj == 0)?n/mn_dimj: n/mn_dimj+1;

  for(int mn_i=0; mn_i<mn_m; mn_i++){
    for(int mn_j=0; mn_j<mn_n; mn_j++){
      
      int tile_i_end;
      int tile_j_end;
      if(cornor_blk && (mn_i==(mn_m-1)) && (mn_j==(mn_n-1))){
         shared_ptr<MacroNode> mn(new MacroNode(mn_temp4, mns.size()));
         mns.push_back(mn);
	 tile_i_end = m;
	 tile_j_end = n;
      }
      else if(rightcol_blk && (mn_j==(mn_n-1))){
         shared_ptr<MacroNode> mn(new MacroNode(mn_temp2, mns.size()));
	 mns.push_back(mn);
	 tile_i_end = (mn_i+1)*mn_dimi;
	 tile_j_end = n;
      }
      else if(bottomrow_blk && (mn_i==(mn_m-1))){
         shared_ptr<MacroNode> mn(new MacroNode(mn_temp3, mns.size()));
	 mns.push_back(mn);
	 tile_i_end = m;
	 tile_j_end = (mn_j+1)*mn_dimj;
      }
      else{
         shared_ptr<MacroNode> mn(new MacroNode(mn_temp1, mns.size()));
	 mns.push_back(mn);
	 tile_i_end = (mn_i+1)*mn_dimi;
	 tile_j_end = (mn_j+1)*mn_dimj;
      }

      for(int tile_i=mn_i*mn_dimi; tile_i<tile_i_end; tile_i++){
	for(int tile_j=mn_j*mn_dimj; tile_j<tile_j_end; tile_j++){
	  int tile_idx = n*tile_i+tile_j;
	  int tile_idx_mn = (*mns.rbegin())->tile_map.size();
	  (*mns.rbegin())->tile_map[tiles_idx[tile_idx]] = tile_idx_mn;  
	}
      }
    }
  }

}

void Load_mtx::MacroNodeGen_idxorder(vector<MacroNode*> &mns){
  int num_tiles_mn = 36;
  int num_tiles = tiles_idx.size();

  int num_mns = (num_tiles%num_tiles_mn==0)?num_tiles/num_tiles_mn:num_tiles/num_tiles_mn+1;
  int mn_idx_start = mns.size();

  if(num_tiles/num_tiles_mn!=0){
    MacroNodeTemplate* mn1_temp = new MacroNodeTemplate("load mtx temp1");
    mn1_temp->MN_load(num_tiles_mn, true);
    mn_temps.push_back(mn1_temp);
    for(int i=0; i<int(num_tiles/num_tiles_mn); i++){
      MacroNode* mn = new MacroNode(mn1_temp, mns.size());
      mns.push_back(mn);
    }
  }
  if(num_tiles%num_tiles_mn!=0){
    MacroNodeTemplate* mn2_temp = new MacroNodeTemplate("load mtx temp2");
    mn2_temp->MN_load(num_tiles%num_tiles_mn, true);
    mn_temps.push_back(mn2_temp);
    if(num_tiles%num_tiles_mn!=0){
      MacroNode* mn = new MacroNode(mn2_temp, mns.size());
      mns.push_back(mn);
    }
  }

  //map real tiles into mns
  if(row_major_mn){
    for(vector<int>::iterator t=tiles_idx.begin(); t!=tiles_idx.end(); t++){
      int mn_idx = (t-tiles_idx.begin())/num_tiles_mn;
      int tile_idx_mn = mns[mn_idx_start+mn_idx]->tile_map.size();
      mns[mn_idx_start+mn_idx]->tile_map[*t] = tile_idx_mn;
      //cout << "real tile idx " << *t << ": " << "template tile idx " << tile_idx_mn << endl;
    }
  }
  else{
    for(vector<int>::iterator t=tiles_idx.begin(); t!=tiles_idx.end(); t++){
      int tile_idx = t-tiles_idx.begin();
      int row_idx = tile_idx/n;
      int col_idx = tile_idx-n*row_idx;
      int mn_idx = (col_idx*m+row_idx)/num_tiles_mn;
      int tile_idx_mn = mns[mn_idx_start+mn_idx]->tile_map.size();
      mns[mn_idx_start+mn_idx]->tile_map[*t] = tile_idx_mn;
      //cout << "real tile idx " << *t << ": " << "template tile idx " << tile_idx_mn << endl;
    }
  }

  cout << "pattern dimension (macronode): 1 x " << num_mns << endl;
}

// Store final results
Store_vec::Store_vec(vector<Operation> &ops, string &in_cp, const vector<int> &in_vec){
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t i=0; i<in_vec.size(); i++){
    PatternUnit pu;

    pre_idx++;
    Operation op;
    op.idx = pre_idx;
    op.in.push_back(in_vec[i]);
    op.fu = FunctionUnitLib::STORE;
    ops.push_back(op);
    pu.ops.push_back(op.idx);

    pus.push_back(pu);
  }

  name = "store_vector";

  cb = ComputeBlockLib::cbs["store"];	
}

// Store final results - matrix
Store_mtx::Store_mtx(vector<Operation> &ops, string &in_cp, const vector<vector<int>> &in_mtx){
  m = in_mtx.size();
  n = in_mtx[0].size();

  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t i=0; i<in_mtx.size(); i++){
    for(size_t j=0; j<in_mtx[0].size(); j++){
      PatternUnit pu;

      pre_idx++;
      Operation op;
      op.idx = pre_idx;
      op.in.push_back(in_mtx[i][j]);
      op.fu = FunctionUnitLib::STORE;
      ops.push_back(op);
      pu.ops.push_back(op.idx);

      pus.push_back(pu);
    }
  }
  name = "store_matrix";

  cb = ComputeBlockLib::cbs["store"];
}

Store_mtx::~Store_mtx(){
  /*
  for(auto it=mns.begin(); it!=mns.end(); it++){
    delete *it;
  }
  */
}

void Store_mtx::TileGen(vector<Tile> &tiles, vector<Operation> &ops){
  int num_tile = (pus.size()%cb->width==0)?pus.size()/cb->width:pus.size()/cb->width+1;

  int tile_idx_start = tiles.size();
  for(int i=0; i<num_tile; i++){
    Tile tile(cb);
    tile.latency = cb->latency;
    tile.pattern_name = name;
    tile.cpsection_name = cp;
    tiles.push_back(tile);
    tiles_idx.push_back(tiles.size()-1);
  }

  for(vector<PatternUnit>::iterator pu=pus.begin(); pu!=pus.end(); pu++){
    int idx = pu-pus.begin();
    int tile_idx = tile_idx_start + int(idx/cb->width);
    //add pu to the tiles[tile_idx];
    for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
      tiles[tile_idx].ops.push_back(*op);
      ops[*op].tile = tile_idx;
    }
  }

  tile_m = m;
  tile_n = n;

  cout << "pattern dimension (pu): 1 x " << pus.size() << endl;
  cout << "pattern dimension (tile): 1 x " << num_tile << endl;
}

void Store_mtx::MacroNodeGen(int mn_dimi, int mn_dimj, int mn_diml){
  mn_m = (m%mn_dimi == 0)?m/mn_dimi: m/mn_dimi+1;
  mn_n = (n%mn_dimj == 0)?n/mn_dimj: n/mn_dimj+1;

  for(int mn_i=0; mn_i<mn_m; mn_i++){
    for(int mn_j=0; mn_j<mn_n; mn_j++){
      cout << endl << "dblk " << dblks.size() << endl;
      shared_ptr<DataBlock> dblk(new DataBlock(dblks.size()));
      dblks.push_back(dblk);

      //add ops
      for(int i=mn_i*mn_dimi; i<(mn_i+1)*mn_dimi; i++){
        for(int j=mn_j*mn_dimj; j<(mn_j+1)*mn_dimj; j++){
	  //pu idx
	  int pu_idx = i*n+j;
	  dblk->ops.push_back(pus[pu_idx].ops[0]);
	  cout << pus[pu_idx].ops[0] << endl;
	}
      }
    }	
  }
}

//void Store_mtx::MacroNodeGen(vector<MacroNode*> &mns, int mn_dimi, int mn_dimj){
void Store_mtx::MacroNodeGen_tile(int mn_dimi, int mn_dimj, int mn_diml){

  bool main_blk = (m/mn_dimi != 0) && (n/mn_dimj != 0);
  bool rightcol_blk = (m/mn_dimi != 0) && (n%mn_dimj != 0);
  bool bottomrow_blk = (n/mn_dimj != 0) && (m%mn_dimi != 0);
  bool cornor_blk = (m%mn_dimi != 0) && (n%mn_dimj != 0);

  MacroNodeTemplate* mn_temp1;
  MacroNodeTemplate* mn_temp2;
  MacroNodeTemplate* mn_temp3;
  MacroNodeTemplate* mn_temp4;
  
  if(main_blk){
    cout << "mn temp for main blk" << endl;
    mn_temp1 = new MacroNodeTemplate("store mtx");
    mn_temp1->MN_store(mn_dimi*mn_dimj, true);
    mn_temps.push_back(mn_temp1);
  }
  if(rightcol_blk){
    cout << "mn temp for rightcol blk" << endl; 
    mn_temp2 = new MacroNodeTemplate("store mtx");
    mn_temp2->MN_store(mn_dimi*(n%mn_dimj), true);
    mn_temps.push_back(mn_temp2);
  }
  if(bottomrow_blk){
    cout << "mn temp for bottomrow blk" << endl;
    mn_temp3 = new MacroNodeTemplate("store mtx");
    mn_temp3->MN_store((m%mn_dimi)*mn_dimj, true);
    mn_temps.push_back(mn_temp3);
  }
  if(cornor_blk){
    cout << "mn temp for cornor blk" << endl;
    mn_temp4 = new MacroNodeTemplate("store mtx");
    mn_temp4->MN_store((m%mn_dimi)*(n%mn_dimj), true);
    mn_temps.push_back(mn_temp4);
  }

  mn_m = (m%mn_dimi == 0)?m/mn_dimi: m/mn_dimi+1;
  mn_n = (n%mn_dimj == 0)?n/mn_dimj: n/mn_dimj+1;

  for(int mn_i=0; mn_i<mn_m; mn_i++){
    for(int mn_j=0; mn_j<mn_n; mn_j++){
      
      int tile_i_end;
      int tile_j_end;
      if(cornor_blk && (mn_i==(mn_m-1)) && (mn_j==(mn_n-1))){
         shared_ptr<MacroNode> mn(new MacroNode(mn_temp4, mns.size()));
         mns.push_back(mn);
	 tile_i_end = m;
	 tile_j_end = n;
      }
      else if(rightcol_blk && (mn_j==(mn_n-1))){
         shared_ptr<MacroNode> mn(new MacroNode(mn_temp2, mns.size()));
	 mns.push_back(mn);
	 tile_i_end = (mn_i+1)*mn_dimi;
	 tile_j_end = n;
      }
      else if(bottomrow_blk && (mn_i==(mn_m-1))){
         shared_ptr<MacroNode> mn(new MacroNode(mn_temp3, mns.size()));
	 mns.push_back(mn);
	 tile_i_end = m;
	 tile_j_end = (mn_j+1)*mn_dimj;
      }
      else{
         shared_ptr<MacroNode> mn(new MacroNode(mn_temp1, mns.size()));
	 mns.push_back(mn);
	 tile_i_end = (mn_i+1)*mn_dimi;
	 tile_j_end = (mn_j+1)*mn_dimj;
      }

      for(int tile_i=mn_i*mn_dimi; tile_i<tile_i_end; tile_i++){
	for(int tile_j=mn_j*mn_dimj; tile_j<tile_j_end; tile_j++){
	  int tile_idx = n*tile_i+tile_j;
	  int tile_idx_mn = (*mns.rbegin())->tile_map.size();
	  (*mns.rbegin())->tile_map[tiles_idx[tile_idx]] = tile_idx_mn;  
	}
      }
    }
  }
}

void Store_mtx::MacroNodeGen_idxorder(vector<MacroNode*> &mns){
  int num_tiles_mn = 36;
  int num_tiles = tiles_idx.size();

  int num_mns = (num_tiles%num_tiles_mn==0)?num_tiles/num_tiles_mn:num_tiles/num_tiles_mn+1;
  int mn_idx_start = mns.size();

  if(num_tiles/num_tiles_mn!=0){
    MacroNodeTemplate* mn1_temp = new MacroNodeTemplate("store mtx temp1");
    mn1_temp->MN_store(num_tiles_mn, true);
    mn_temps.push_back(mn1_temp);
    for(int i=0; i<int(num_tiles/num_tiles_mn); i++){
      MacroNode* mn = new MacroNode(mn1_temp, mns.size());
      mns.push_back(mn);
    }
  }
  if(num_tiles%num_tiles_mn!=0){
    MacroNodeTemplate* mn2_temp = new MacroNodeTemplate("store mtx temp2");
    mn2_temp->MN_store(num_tiles%num_tiles_mn, true);
    mn_temps.push_back(mn2_temp);
    if(num_tiles%num_tiles_mn!=0){
      MacroNode* mn = new MacroNode(mn2_temp, mns.size());
      mns.push_back(mn);
    }
  }

  //map real tiles into mns
  for(vector<int>::iterator t=tiles_idx.begin(); t!=tiles_idx.end(); t++){
    int mn_idx = (t-tiles_idx.begin())/num_tiles_mn;
    int tile_idx_mn = mns[mn_idx_start+mn_idx]->tile_map.size();
    mns[mn_idx_start+mn_idx]->tile_map[*t] = tile_idx_mn;
  }

  cout << "pattern dimension (macronode): 1 x " << num_mns << endl;
}

// Matrix multiplication
MtxMul::MtxMul(vector<Operation> &ops, string &in_cp, const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, vector<vector<int>> &out_mtx)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t i=0; i<in_mtx1.size(); i++){
    vector<int> out_mtx_row;
    for(size_t j=0; j<in_mtx2[0].size(); j++){
      // one dot product
      for(size_t k=0; k<in_mtx1[0].size(); k++){
	PatternUnit pu;

	pre_idx++;
	Operation op1;
	op1.idx = pre_idx;
	op1.in.push_back(in_mtx1[i][k]);
	op1.in.push_back(in_mtx2[k][j]);
	op1.fu = FunctionUnitLib::MUL;

	ops.push_back(op1);
	pu.ops.push_back(op1.idx);

	pre_idx++;
	Operation op2;
	op2.idx = pre_idx;
	if(k==0){
	  op2.in.push_back(op1.idx);
	  op2.in.push_back(-1);
	  op2.fu = FunctionUnitLib::ADD;
	}
	else{
	  // last one and second last one
	  op2.in.push_back(op1.idx);
	  op2.in.push_back(op1.idx-1);
	  op2.fu = FunctionUnitLib::ADD;
	}
	ops.push_back(op2);
	pu.ops.push_back(op2.idx);

	if(k==in_mtx1[0].size()-1){
	  out_mtx_row.push_back(op2.idx);
	}

	pus.push_back(pu);
      }
    }
    out_mtx.push_back(out_mtx_row);
  }

  name = "matrix_multiplication";

  cb = ComputeBlockLib::cbs["mul_acc"];	

  m = in_mtx1.size();
  k = in_mtx1[0].size();
  n = in_mtx2[0].size();
}

MtxMul::~MtxMul(){
  /*
  for(auto it=mns.begin(); it!=mns.end(); it++){
    delete *it;
  }
  */

}

void MtxMul::TileGen(vector<Tile> &tiles, vector<Operation> &ops){
  //initialize tiles
  int num_tile_row = ((m*n)%cb->width == 0)? (m*n)/cb->width: (m*n)/cb->width+1;
  int num_tile_col = (k%cb->max_depth == 0)? k/cb->max_depth: k/cb->max_depth+1;

  int tile_idx_start = tiles.size();
  for(int i=0; i<num_tile_col; i++){
    for(int j=0; j<num_tile_row; j++){
      Tile tile(cb);
      if((k%cb->max_depth != 0) && (i == num_tile_col-1)){
	int cb_real_depth = k%cb->max_depth;
	tile.latency = cb->GetRealLatency(cb_real_depth);
      }
      else{
	tile.latency = cb->latency;
      }
      tile.pattern_name = name;
      tile.cpsection_name = cp;
      tiles.push_back(tile);
      tiles_idx.push_back(tiles.size()-1);
    }
  }

  for(vector<PatternUnit>::iterator pu=pus.begin(); pu!=pus.end(); pu++){
    int idx = pu-pus.begin();
    int col_idx = idx/k;
    int row_idx = idx-col_idx*k;
    int tile_idx = tile_idx_start + int(row_idx/cb->max_depth)*num_tile_row + int(col_idx/cb->width);
    //add pu to the tiles[tile_idx];
    for(vector<int>::iterator op=pu->ops.begin(); op!=pu->ops.end(); op++){
      tiles[tile_idx].ops.push_back(*op);
      ops[*op].tile = tile_idx;
    }
  }

  tile_m = m;
  tile_n = n;
  tile_k = (k%cb->max_depth==0)?k/cb->max_depth:k/cb->max_depth+1;
  
  cout << "pattern dimension (pu): " << k << " x " << m*n << endl;
  cout << "pattern dimension (tile): " << num_tile_col << " x " << num_tile_row << endl;

}

void MtxMul::MacroNodeGen(int mn_dimi, int mn_dimj, int mn_diml){
  int mn_dimi_tile = mn_dimi;
  int mn_dimj_tile = mn_dimj;
  int mn_diml_tile = mn_diml/cb->max_depth;

  mn_m = (m%mn_dimi==0)?m/mn_dimi:m/mn_dimi+1;
  mn_n = (n%mn_dimj==0)?n/mn_dimj:n/mn_dimj+1;
  mn_k = (k%mn_diml==0)?k/mn_diml:k/mn_diml+1;

  MacroNodeTemplate* mn_temp = new MacroNodeTemplate("mtxmul");
  mn_temp->MN_mtxmul(mn_dimi, mn_dimj, mn_diml, true);
  mn_temps.push_back(mn_temp);

  for(int mn_i=0; mn_i<mn_m; mn_i++){
    for(int mn_j=0; mn_j<mn_n; mn_j++){
      for(int mn_l=0; mn_l<mn_k; mn_l++){

	shared_ptr<MacroNode> mn(new MacroNode(mn_temp, mns.size()));
	mns.push_back(mn);
	
	for(int tile_i=mn_i*mn_dimi_tile; tile_i<(mn_i+1)*mn_dimi_tile; tile_i++){
	  for(int tile_j=mn_j*mn_dimj_tile; tile_j<(mn_j+1)*mn_dimj_tile; tile_j++){
	    for(int tile_l=mn_l*mn_diml_tile; tile_l<(mn_l+1)*mn_diml_tile; tile_l++){
	      int tile_idx = (tile_m*tile_n)*tile_l + tile_n*tile_i + tile_j;
	      int tile_idx_mn = (*mns.rbegin())->tile_map.size();
	      (*mns.rbegin())->tile_map[tiles_idx[tile_idx]] = tile_idx_mn;
	    }
	  }
	}
      }
    }
  }
  
}


//void MtxMul::MacroNodeGen(vector<MacroNode*> &mns, int mn_dimi, int mn_dimj){	
void MtxMul::MacroNodeGen_tile(int mn_dimi, int mn_dimj, int mn_diml){	
 //mn_dim: mn dimension in terms of #tiles

  bool main_m = (tile_m/mn_dimi != 0);
  bool res_m = (tile_m%mn_dimi != 0);
  bool main_n = (tile_n/mn_dimj != 0);
  bool res_n = (tile_n%mn_dimj != 0);
  bool main_k = (tile_k/mn_diml != 0);
  bool res_k = (tile_k%mn_diml != 0);

  bool main_blk = main_m && main_n && main_k;
  bool rightplane_blk = main_m && main_k && res_n;
  bool frontplane_blk = main_k && main_n && res_m;
  bool cornorcol_blk = main_k && res_m && res_n;
  bool bottommain_blk = main_m && main_n && res_k;
  bool bottomright_blk = main_m && res_n && res_k;
  bool bottomfront_blk = main_n && res_m && res_k;
  bool bottomcornor_blk = res_m && res_n && res_k;

  MacroNodeTemplate* mn_temp1;
  MacroNodeTemplate* mn_temp2;
  MacroNodeTemplate* mn_temp3;
  MacroNodeTemplate* mn_temp4;
  MacroNodeTemplate* mn_temp5;
  MacroNodeTemplate* mn_temp6;
  MacroNodeTemplate* mn_temp7;
  MacroNodeTemplate* mn_temp8;
  
  if(main_blk){
    cout << "mn temp for main blk" << endl;
    mn_temp1 = new MacroNodeTemplate("mtxmul");
    mn_temp1->MN_mtxmul(mn_dimi, mn_dimj, mn_diml*cb->max_depth, true);
    mn_temps.push_back(mn_temp1);
  }
  if(rightplane_blk){
    cout << "mn temp for rightplane blk" << endl;
    mn_temp2 = new MacroNodeTemplate("mtxmul");
    mn_temp2->MN_mtxmul(mn_dimi, n%mn_dimj, mn_diml*cb->max_depth, true);
    mn_temps.push_back(mn_temp2);
  }
  if(frontplane_blk){
    cout << "mn temp for frontplane blk" << endl;
    mn_temp3 = new MacroNodeTemplate("mtxmul");
    mn_temp3->MN_mtxmul(m%mn_dimi, mn_dimj, mn_diml*cb->max_depth, true);
    mn_temps.push_back(mn_temp3);
  }
  if(cornorcol_blk){
    cout << "mn temp for cornorcol blk" << endl;
    mn_temp4 = new MacroNodeTemplate("mtxmul");
    mn_temp4->MN_mtxmul(m%mn_dimi, n%mn_dimj, mn_diml*cb->max_depth, true);
    mn_temps.push_back(mn_temp4);
  }
  if(bottommain_blk){
    cout << "mn temp for bottommain blk" << endl;
    mn_temp5 = new MacroNodeTemplate("mtxmul");
    mn_temp5->MN_mtxmul(mn_dimi, mn_dimj, (tile_k%mn_diml)*cb->max_depth, true);
    mn_temps.push_back(mn_temp5);
  }
  if(bottomright_blk){
    cout << "mn temp for bottomright blk" << endl;
    mn_temp6 = new MacroNodeTemplate("mtxmul");
    mn_temp6->MN_mtxmul(mn_dimi, n%mn_dimj, (tile_k%mn_diml)*cb->max_depth, true);
    mn_temps.push_back(mn_temp6);
  }
  if(bottomfront_blk){
    cout << "mn temp for bottomfront blk" << endl;
    mn_temp7 = new MacroNodeTemplate("mtxmul");
    mn_temp7->MN_mtxmul(m%mn_dimi, mn_dimj, (tile_k%mn_diml)*cb->max_depth, true);
    mn_temps.push_back(mn_temp7);
  }
  if(bottomcornor_blk){
    cout << "mn temp for bottomcornor blk" << endl;
    mn_temp8 = new MacroNodeTemplate("mtxmul");
    mn_temp8->MN_mtxmul(m%mn_dimi, n%mn_dimj, (tile_k%mn_diml)*cb->max_depth, true);
    mn_temps.push_back(mn_temp8);
  }
  
  mn_m = (tile_m%mn_dimi==0)?tile_m/mn_dimi:tile_m/mn_dimi+1;
  mn_n = (tile_n%mn_dimj==0)?tile_n/mn_dimj:tile_n/mn_dimj+1;
  mn_k = (tile_k%mn_diml==0)?tile_k/mn_diml:tile_k/mn_diml+1;

  for(int mn_i=0; mn_i<mn_m; mn_i++){
    for(int mn_j=0; mn_j<mn_n; mn_j++){
      for(int mn_l=0; mn_l<mn_k; mn_l++){
        //build new mn
        int tile_i_end;
        int tile_j_end;
	int tile_l_end;

        if(bottomcornor_blk && (mn_i==(mn_m-1)) && (mn_j==(mn_n-1)) && (mn_l==(mn_k-1))){
          shared_ptr<MacroNode> mn(new MacroNode(mn_temp8, mns.size()));
          mns.push_back(mn);
	  tile_i_end = tile_m;
	  tile_j_end = tile_n;
	  tile_l_end = tile_k;
        }
	else if(bottomfront_blk && (mn_i==(mn_m-1)) && (mn_l==(mn_k-1))){
	  shared_ptr<MacroNode> mn(new MacroNode(mn_temp7, mns.size()));
	  mns.push_back(mn);
	  tile_i_end = tile_m;
	  tile_j_end = (mn_j+1)*mn_dimj;
	  tile_l_end = tile_k;
	}
	else if(bottomright_blk && (mn_j==(mn_n-1)) && (mn_l==(mn_k-1))){
	  shared_ptr<MacroNode> mn(new MacroNode(mn_temp6, mns.size()));
	  mns.push_back(mn);
	  tile_i_end = (mn_i+1)*mn_dimi;
	  tile_j_end = tile_n;
	  tile_l_end = tile_k;
	}
	else if(bottommain_blk && (mn_l==(mn_k-1))){
	  shared_ptr<MacroNode> mn(new MacroNode(mn_temp5, mns.size()));
	  mns.push_back(mn);
	  tile_i_end = (mn_i+1)*mn_dimi;
	  tile_j_end = (mn_j+1)*mn_dimj;
	  tile_l_end = tile_k;
	}
	else if(cornorcol_blk && (mn_i==(mn_m-1)) && (mn_j==(mn_n-1))){
	  shared_ptr<MacroNode> mn(new MacroNode(mn_temp4, mns.size()));
	  mns.push_back(mn);
	  tile_i_end = tile_m;
	  tile_j_end = tile_n;
	  tile_l_end = (mn_l+1)*mn_diml;
	}
	else if(frontplane_blk && (mn_i==(mn_m-1))){
	  shared_ptr<MacroNode> mn(new MacroNode(mn_temp3, mns.size()));
	  mns.push_back(mn);
	  tile_i_end = tile_m;
	  tile_j_end = (mn_j+1)*mn_dimj;
	  tile_l_end = (mn_l+1)*mn_diml;
	}
	else if(rightplane_blk && (mn_j==(mn_n-1))){
	  shared_ptr<MacroNode> mn(new MacroNode(mn_temp2, mns.size()));
	  mns.push_back(mn);
	  tile_i_end = (mn_i+1)*mn_dimi;
	  tile_j_end = tile_n;
	  tile_l_end = (mn_l+1)*mn_diml;
	}
	else{
	  shared_ptr<MacroNode> mn(new MacroNode(mn_temp1, mns.size()));
	  mns.push_back(mn);
	  tile_i_end = (mn_i+1)*mn_dimi;
	  tile_j_end = (mn_j+1)*mn_dimj;
	  tile_l_end = (mn_l+1)*mn_diml;
	}

	cout << endl << "mn generated: " << mns.size() << endl;

	for(int tile_i=mn_i*mn_dimi; tile_i<tile_i_end; tile_i++){
	  for(int tile_j=mn_j*mn_dimj; tile_j<tile_j_end; tile_j++){
	    for(int tile_l=mn_l*mn_diml; tile_l<tile_l_end; tile_l++){
	      cout << "tile_i " << tile_i << endl;
	      cout << "tile_j " << tile_j << endl;
	      cout << "tile_l " << tile_l << endl;
	      int tile_idx = (tile_m*tile_n)*tile_l + tile_n*tile_i + tile_j;
	      int tile_idx_mn = (*mns.rbegin())->tile_map.size();
	      (*mns.rbegin())->tile_map[tiles_idx[tile_idx]] = tile_idx_mn;
	    }
	  }
	}
      }
    }
  }

}

void MtxMul::MacroNodeGen_idxorder(vector<MacroNode*> &mns){
  int k_tile = (k%cb->max_depth==0)?k/cb->max_depth:k/cb->max_depth+1;

  int num_tiles_mn = 256;

  if(n*m < sqrt(num_tiles_mn)){
    //cout << "CASE1: m*n in same mn" << endl;
    //mn template
    int k_tile_mn = num_tiles_mn/(m*n);
    int num_mn1 = k_tile/k_tile_mn;
    int num_mn2 = k_tile%k_tile_mn;;

    int mn_idx_start = mns.size();
    if(num_mn1 != 0){
      //need mn template mn1
      MacroNodeTemplate* mn1_temp = new MacroNodeTemplate("mtxmul temp1");
      int k_mn = k_tile_mn*cb->max_depth;
      mn1_temp->MN_mtxmul(m,n,k_mn,true);
      mn_temps.push_back(mn1_temp);
      //cout << "number of tiles in mn1_temp " << mn1_temp.tiles.size() << endl;

      //initialize mns
      for(int i=0; i<num_mn1; i++){
	MacroNode* mn = new MacroNode(mn1_temp, mns.size());
	mns.push_back(mn);
      }
    }
    //cout << "num mn2 " << num_mn2 << endl;
    if(num_mn2 != 0){
      //need mn template mn2
      MacroNodeTemplate* mn2_temp = new MacroNodeTemplate("mtxmul temp2");
      int k_mn = k-num_mn1*k_tile_mn*cb->max_depth;
      mn2_temp->MN_mtxmul(m,n,k_mn,true);
      mn_temps.push_back(mn2_temp);
      //cout << "number of tiles in mn2_temp " << mn2_temp.tiles.size() << endl;

      MacroNode* mn = new MacroNode(mn2_temp, mns.size());
      mns.push_back(mn);
    }

    //cout << "num of mns generated " << mns.size() << endl;

    //map tiles into mns
    for(vector<int>::iterator t=tiles_idx.begin(); t!=tiles_idx.end(); t++){
      int mn_idx = ((t-tiles_idx.begin())/(m*n))/k_tile_mn;
      int tile_idx = mns[mn_idx_start+mn_idx]->tile_map.size();
      mns[mn_idx_start+mn_idx]->tile_map[*t] = tile_idx;  
    }

    cout << "pattern dimension (macronode): " << num_mn1+num_mn2 << " x 1" << endl;	
  }
  else if(n <= sqrt(num_tiles_mn)){
    //cout << "CASE2: multiple n in same mn" << endl;
    //mn template
    int m_tile_mn = sqrt(num_tiles_mn)/n;
    int k_tile_mn = num_tiles_mn/(n*m_tile_mn);

    //cout << "m_tile_mn " << m_tile_mn << endl;
    //cout << "k_tile_mn " << k_tile_mn << endl;

    int mn_idx_start = mns.size();
    int num_mn_col = (k_tile%k_tile_mn==0)?(k_tile/k_tile_mn):(k_tile/k_tile_mn+1);
    int num_mn_row = (m%m_tile_mn==0)?(m/m_tile_mn):(m/m_tile_mn+1);

    if(k_tile/k_tile_mn != 0){
      MacroNodeTemplate* mn1_temp;
      MacroNodeTemplate* mn2_temp;
      if(m/m_tile_mn != 0){
	mn1_temp = new MacroNodeTemplate("mtxmul temp1");
	int k_mn = k_tile_mn*cb->max_depth;
	mn1_temp->MN_mtxmul(m_tile_mn, n, k_mn, true);
	mn_temps.push_back(mn1_temp);
	//cout << "number of tiles in mn1_temp " << mn1_temp.tiles.size() << endl;
      }
      if(m%m_tile_mn != 0){
	mn2_temp = new MacroNodeTemplate("mtxmul temp2");
	int m_mn = m%m_tile_mn;
	int k_mn = k_tile_mn*cb->max_depth;
	mn2_temp->MN_mtxmul(m_mn, n, k_mn, true);
	mn_temps.push_back(mn2_temp);
	//cout << "number of tiles in mn2_temp " << mn2_temp.tiles.size() << endl;
      }

      for(int i=0; i<(k_tile/k_tile_mn); i++){
	for(int j=0; j<(m/m_tile_mn); j++){
	  MacroNode* mn = new MacroNode(mn1_temp, mns.size());
	  mns.push_back(mn);
	}
	if(m%m_tile_mn != 0){
	  MacroNode* mn = new MacroNode(mn2_temp, mns.size());
	  mns.push_back(mn);
	}
      }
    }
    if(k_tile%k_tile_mn != 0){
      MacroNodeTemplate* mn3_temp;	
      MacroNodeTemplate* mn4_temp;
      if(m/m_tile_mn != 0){
	mn3_temp = new MacroNodeTemplate("mtxmul temp3");	
	int k_mn = k-(int(k_tile/k_tile_mn))*k_tile_mn*cb->max_depth;
	mn3_temp->MN_mtxmul(m_tile_mn, n, k_mn, true);
	mn_temps.push_back(mn3_temp);
	//cout << "number of tiles in mn3_temp " << mn3_temp.tiles.size() << endl;
      }
      if(m%m_tile_mn != 0){
	mn4_temp = new MacroNodeTemplate("mtxmul temp4");
	int m_mn = m%m_tile_mn;
	int k_mn = k-(int(k_tile/k_tile_mn))*k_tile_mn*cb->max_depth;
	mn4_temp->MN_mtxmul(m_mn, n, k_mn, true);
	mn_temps.push_back(mn4_temp);
	//cout << "number of tiles in mn4_temp " << mn4_temp.tiles.size() << endl;
      }

      for(int j=0; j<(m/m_tile_mn); j++){
	MacroNode* mn = new MacroNode(mn3_temp, mns.size());
	mns.push_back(mn);
      }
      if(m%m_tile_mn != 0){
	MacroNode* mn = new MacroNode(mn4_temp, mns.size());
	mns.push_back(mn);
      }
    }

    //map tile into mns	
    for(vector<int>::iterator t=tiles_idx.begin(); t!=tiles_idx.end(); t++){
      int idx = t-tiles_idx.begin();
      int k_idx = idx/(m*n);
      int m_idx = (idx-k_idx*m*n)/n;
      int num_mn_row = (m%m_tile_mn==0)?m/m_tile_mn:m/m_tile_mn+1;
      int mn_idx = (k_idx/k_tile_mn)*num_mn_row + (m_idx/m_tile_mn);
      int tile_idx_mn = mns[mn_idx_start+mn_idx]->tile_map.size();
      mns[mn_idx_start+mn_idx]->tile_map[*t] = tile_idx_mn;  
    }

    cout << "pattern dimension (macronode): " << num_mn_col << " x " << num_mn_row << endl;	   

  }
  else{
    //cout << "CASE3: multiple mn in same n" << endl;
    int m_tile_mn = 1;
    int n_tile_mn = int(sqrt(num_tiles_mn));
    int k_tile_mn = num_tiles_mn/(m_tile_mn*n_tile_mn);

    //cout << "n_tile_mn " << n_tile_mn << endl;
    //cout << "k_tile_mn " << k_tile_mn << endl;
    int mn_idx_start = mns.size();

    int num_mn_col = (k_tile%k_tile_mn==0)?(k_tile/k_tile_mn):(k_tile/k_tile_mn+1);
    int num_mn_row = (n%n_tile_mn==0)?m*(n/n_tile_mn):m*(n/n_tile_mn+1);

    if(k_tile/k_tile_mn != 0){
      MacroNodeTemplate* mn1_temp;
      MacroNodeTemplate* mn2_temp;
      int k_mn = k_tile_mn*cb->max_depth;
      if(n/n_tile_mn != 0){
	mn1_temp = new MacroNodeTemplate("mtxmul temp1");
	mn1_temp->MN_mtxmul(m_tile_mn, n_tile_mn, k_mn, true);
	mn_temps.push_back(mn1_temp);
	//cout << "number of tiles in mn1_temp " << mn1_temp.tiles.size() << endl;
      }
      if(n%n_tile_mn != 0){
	mn2_temp = new MacroNodeTemplate("mtxmul temp2");
	int n_mn = n%n_tile_mn;
	mn2_temp->MN_mtxmul(m_tile_mn, n_mn, k_mn, true);
	mn_temps.push_back(mn2_temp);
	//cout << "number of tiles in mn2_temp " << mn2_temp.tiles.size() << endl;
      }

      for(int i=0; i<(k_tile/k_tile_mn); i++){
	for(int j=0; j<m; j++){
	  for(int l=0; l<(n/n_tile_mn); l++){
	    MacroNode* mn = new MacroNode(mn1_temp, mns.size());
	    mns.push_back(mn);
	  }
	  if(n%n_tile_mn != 0){
	    MacroNode* mn = new MacroNode(mn2_temp, mns.size());
	    mns.push_back(mn);
	  }
	}
      }
    }
    if(k_tile%k_tile_mn != 0){
      MacroNodeTemplate* mn3_temp;
      MacroNodeTemplate* mn4_temp;
      int k_mn = k-(int(k_tile/k_tile_mn))*k_tile_mn*cb->max_depth;
      if(n/n_tile_mn != 0){
	mn3_temp = new MacroNodeTemplate("mtxmul temp3");
	mn3_temp->MN_mtxmul(m_tile_mn, n_tile_mn, k_mn, true);
	mn_temps.push_back(mn3_temp);
	//cout << "number of tiles in mn3_temp " << mn3_temp.tiles.size() << endl;
      }
      if(n%n_tile_mn != 0){
	mn4_temp = new MacroNodeTemplate("mtxmul temp4");
	int n_mn = n%n_tile_mn;
	mn4_temp->MN_mtxmul(m_tile_mn, n_mn, k_mn, true);
	mn_temps.push_back(mn4_temp);
	//cout << "number of tiles in mn4_temp " << mn4_temp.tiles.size() << endl;
      }

      for(int j=0; j<m; j++){
	for(int l=0; l<(n/n_tile_mn); l++){
	  MacroNode* mn = new MacroNode(mn3_temp, mns.size());
	  mns.push_back(mn);
	}
	if(n%n_tile_mn != 0){
	  MacroNode* mn = new MacroNode(mn4_temp, mns.size());
	  mns.push_back(mn);
	}
      }
    }

    //map tile into mns	
    for(vector<int>::iterator t=tiles_idx.begin(); t!=tiles_idx.end(); t++){
      int idx = t-tiles_idx.begin();
      int k_idx = idx/(m*n);
      int m_idx = (idx-k_idx*m*n)/n;
      int n_idx = (idx-k_idx*m*n)-m_idx*n;
      int num_mn_n = (n%n_tile_mn==0)?(n/n_tile_mn):(n/n_tile_mn+1);
      int num_mn_row = m*num_mn_n;
      int mn_idx = (k_idx/k_tile_mn)*num_mn_row + (m_idx/m_tile_mn)*num_mn_n + (n_idx/n_tile_mn);
      int tile_idx_mn = mns[mn_idx_start+mn_idx]->tile_map.size();
      mns[mn_idx_start+mn_idx]->tile_map[*t] = tile_idx_mn;  
    }

    cout << "pattern dimension (macronode): " << num_mn_col << " x " << num_mn_row << endl;	   
  }
}

MtxMulAdd::MtxMulAdd(vector<Operation> &ops, string &in_cp, const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, const vector<vector<int>> &in_mtx3, vector<vector<int>> &out_mtx)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t i=0; i<in_mtx1.size(); i++){
    vector<int> out_mtx_row;
    for(size_t j=0; j<in_mtx2[0].size(); j++){
      // one dot product
      for(size_t k=0; k<in_mtx1[0].size(); k++){
	PatternUnit pu;

	pre_idx++;
	Operation op1;
	op1.idx = pre_idx;
	op1.in.push_back(in_mtx1[i][k]);
	op1.in.push_back(in_mtx2[k][j]);
	op1.fu = FunctionUnitLib::MUL;

	ops.push_back(op1);
	pu.ops.push_back(op1.idx);

	pre_idx++;
	Operation op2;
	op2.idx = pre_idx;
	if(k==0){
	  op2.in.push_back(op1.idx);
	  op2.in.push_back(in_mtx3[i][j]);
	  op2.fu = FunctionUnitLib::ADD;
	}
	else{
	  // last one and second last one
	  op2.in.push_back(op1.idx);
	  op2.in.push_back(op1.idx-1);
	  op2.fu = FunctionUnitLib::ADD;
	}
	ops.push_back(op2);
	pu.ops.push_back(op2.idx);

	if(k==in_mtx1[0].size()-1){
	  out_mtx_row.push_back(op2.idx);
	}

	pus.push_back(pu);
      }
    }
    out_mtx.push_back(out_mtx_row);
  }

  name = "matrix_multiplication_addition";

  cb = ComputeBlockLib::cbs["mul_acc"];	
}


// Calculate the sum of each column
ColSum::ColSum(vector<Operation> &ops, string &in_cp, const vector<vector<int>> &in_mtx, vector<int> &out_vec)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t j=0; j<in_mtx[0].size(); j++){
    for(size_t i=0; i<in_mtx.size(); i++){
      PatternUnit pu;

      pre_idx++;
      Operation op;
      op.idx = pre_idx;
      if(i==0){
	op.fu = FunctionUnitLib::ADD;
	op.in.push_back(-1);
	op.in.push_back(in_mtx[i][j]);
      }
      else{
	op.fu = FunctionUnitLib::ADD;
	op.in.push_back(in_mtx[i][j]);
	op.in.push_back(pre_idx-1);
      }
      ops.push_back(op);
      pu.ops.push_back(op.idx);

      if(i==in_mtx.size()-1){
	out_vec.push_back(op.idx);
      }

      pus.push_back(pu);
    }
  }

  name = "Pattern: sum of each column of matrix";
  description = "Each column sum consists of a series of addition.";

  cb = ComputeBlockLib::cbs["acc"];
}

Div_vec::Div_vec(vector<Operation> &ops, string &in_cp, const vector<int> &in_vec1, const vector<int> &in_vec2, vector<int> &out_vec)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t i=0; i<in_vec1.size(); i++){
    PatternUnit pu;

    pre_idx++;
    Operation op;
    op.idx = pre_idx;
    op.fu = FunctionUnitLib::DIV;
    op.in.push_back(in_vec1[i]);
    op.in.push_back(in_vec2[i]);
    ops.push_back(op);
    pu.ops.push_back(op.idx);
    out_vec.push_back(op.idx);

    pus.push_back(pu);
  }

  name = "Pattern: vector element-wise division";
  description = "Each vector element is divided by the other vector element";

  cb = ComputeBlockLib::cbs["div"];
}


Div_mtx::Div_mtx(vector<Operation> &ops, string &in_cp, const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, vector<vector<int>> &out_mtx)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t i=0; i<in_mtx1.size(); i++){
    vector<int> out_mtx_row;
    for(size_t j=0; j<in_mtx1[0].size(); j++){
      PatternUnit pu;

      pre_idx++;
      Operation op;
      op.idx = pre_idx;
      op.fu = FunctionUnitLib::DIV;
      op.in.push_back(in_mtx1[i][j]);
      op.in.push_back(in_mtx2[i][j]);
      ops.push_back(op);
      pu.ops.push_back(op.idx);
      out_mtx_row.push_back(op.idx);

      pus.push_back(pu);
    }
    out_mtx.push_back(out_mtx_row);
  }

  name = "Pattern: mtx element-wise division";
  description = "Each mtx element is divided by the other mtx element";

  cb = ComputeBlockLib::cbs["div"];
}


Sub_mtx::Sub_mtx(vector<Operation> &ops, string &in_cp, const vector<vector<int>> &in_mtx1, const vector<vector<int>> &in_mtx2, vector<vector<int>> &out_mtx)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t i=0; i<in_mtx1.size(); i++){
    vector<int> out_mtx_row;
    for(size_t j=0; j<in_mtx1[0].size(); j++){
      PatternUnit pu;

      pre_idx++;
      Operation op;
      op.idx = pre_idx;
      op.fu = FunctionUnitLib::SUB;
      op.in.push_back(in_mtx1[i][j]);
      op.in.push_back(in_mtx2[i][j]);
      ops.push_back(op);
      pu.ops.push_back(op.idx);
      out_mtx_row.push_back(op.idx);

      pus.push_back(pu);
    }
    out_mtx.push_back(out_mtx_row);
  }

  name = "Pattern: mtx element-wise subtraction";
  description = "Each mtx element is subtracted by the other mtx element";

  cb = ComputeBlockLib::cbs["sub"];
}

SquareAcc_vec::SquareAcc_vec(vector<Operation> &ops, string &in_cp, const vector<int> &in_vec, int &out)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t i=0; i<in_vec.size(); i++){
    PatternUnit pu;

    pre_idx++;
    Operation op1;
    op1.idx = pre_idx;
    op1.in.push_back(in_vec[i]);
    op1.in.push_back(in_vec[i]);
    op1.fu = FunctionUnitLib::MUL;
    ops.push_back(op1);
    pu.ops.push_back(op1.idx);

    pre_idx++;
    Operation op2;
    op2.idx = pre_idx;
    op2.in.push_back(op1.idx);
    if(i == 0){
      op2.in.push_back(-1);
    }
    else{
      op2.in.push_back(op1.idx-1);
    }
    op2.fu = FunctionUnitLib::ADD;
    ops.push_back(op2);
    pu.ops.push_back(op2.idx);

    if(i == in_vec.size()-1){
      out = op2.idx;
    }

    pus.push_back(pu);
  }

  name = "Pattern: sum of element square";
  description = "Sum of element square in the vector";

  cb = ComputeBlockLib::cbs["mul_acc"];
}

//square acc of mtx col
SquareAcc_mtx::SquareAcc_mtx(vector<Operation> &ops, string &in_cp, const vector<vector<int>> &in_mtx, vector<int> &out_vec)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t j=0; j<in_mtx[0].size(); j++){
    for(size_t i=0; i<in_mtx.size(); i++){

      PatternUnit pu;

      pre_idx++;
      Operation op1;
      op1.idx = pre_idx;
      op1.in.push_back(in_mtx[i][j]);
      op1.in.push_back(in_mtx[i][j]);
      op1.fu = FunctionUnitLib::MUL;
      ops.push_back(op1);
      pu.ops.push_back(op1.idx);

      pre_idx++;
      Operation op2;
      op2.idx = pre_idx;
      op2.in.push_back(op1.idx);
      if(i == 0){
	op2.in.push_back(-1);
      }
      else{
	op2.in.push_back(op1.idx-1);
      }
      op2.fu = FunctionUnitLib::ADD;
      ops.push_back(op2);
      pu.ops.push_back(op2.idx);

      if(i == in_mtx.size()-1){
	out_vec.push_back(op2.idx);
      }

      pus.push_back(pu);
    }
  }

  name = "Pattern: sum of element square";
  description = "Sum of element square in the mtx";

  cb = ComputeBlockLib::cbs["mul_acc"];
}

DivRoot::DivRoot(vector<Operation> &ops, string &in_cp, const vector<int> &in_vec1, const vector<int> &in_vec2, vector<int> &out_vec)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  for(size_t i=0; i<in_vec1.size(); i++){
    PatternUnit pu;

    pre_idx++;
    Operation op1;
    op1.idx = pre_idx;
    op1.in.push_back(in_vec1[i]);
    op1.in.push_back(in_vec2[i]);
    op1.fu = FunctionUnitLib::DIV;
    ops.push_back(op1);
    pu.ops.push_back(op1.idx);

    pre_idx++;
    Operation op2;
    op2.idx = pre_idx;
    op2.in.push_back(op1.idx);
    op2.fu = FunctionUnitLib::ROOT;
    ops.push_back(op2);
    pu.ops.push_back(op2.idx);

    out_vec.push_back(op2.idx);

    pus.push_back(pu);
  }

  name = "Pattern: division between two vectors and then get root";
  description = "Division between two vectors and then get root";

  cb = ComputeBlockLib::cbs["div_root"];
}


Root_s::Root_s(vector<Operation> &ops, string &in_cp, int in_s, int &out_s)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  PatternUnit pu;

  pre_idx++;
  Operation op;
  op.idx = pre_idx;
  op.in.push_back(in_s);
  op.fu = FunctionUnitLib::ROOT;
  ops.push_back(op);
  pu.ops.push_back(op.idx);
  out_s = op.idx;

  pus.push_back(pu);

  name = "Pattern: root of each element in vector";
  description = "Get root value of each vector element. ";

  cb = ComputeBlockLib::cbs["root_s"];	 
}

Div_s::Div_s(vector<Operation> &ops, string &in_cp, int in_s1, int in_s2, int &out_s)
{
  cp = in_cp;
  int pre_idx = ops.size()-1;

  PatternUnit pu;
  pre_idx++;
  Operation op;
  op.idx = pre_idx;
  op.in.push_back(in_s1);
  op.in.push_back(in_s2);
  ops.push_back(op);
  pu.ops.push_back(op.idx);
  out_s = op.idx;

  pus.push_back(pu);

  name = "Pattern: division between two scalar";
  description = "Division between two scalar";

  cb = ComputeBlockLib::cbs["div_s"];
}

Jacobi_cs::Jacobi_cs(vector<Operation> &ops, string &in_cp, int app, int aqq, int apq, int &c, int &s, int &ns){
  cp = in_cp;
  int pre_idx = ops.size()-1;

  PatternUnit pu;

  pre_idx++;
  Operation op1;
  op1.idx = pre_idx;
  op1.in.push_back(aqq);
  op1.in.push_back(app);
  op1.fu = FunctionUnitLib::SUB;
  ops.push_back(op1);
  pu.ops.push_back(op1.idx);

  pre_idx++;
  Operation op2;
  op2.idx = pre_idx;
  op2.in.push_back(apq);
  op2.in.push_back(op1.idx);
  op2.fu = FunctionUnitLib::DIV;
  ops.push_back(op2);
  pu.ops.push_back(op2.idx);

  //t:op2.idx

  pre_idx++;
  Operation op3;
  op3.idx = pre_idx;
  op3.in.push_back(op2.idx);
  op3.in.push_back(op2.idx);
  op3.fu = FunctionUnitLib::MUL;
  ops.push_back(op3);
  pu.ops.push_back(op3.idx);

  pre_idx++;
  Operation op4;
  op4.idx = pre_idx;
  op4.in.push_back(op3.idx);
  op4.in.push_back(-1);
  op4.fu = FunctionUnitLib::ADD;
  ops.push_back(op4);
  pu.ops.push_back(op4.idx);

  pre_idx++;
  Operation op5;
  op5.idx = pre_idx;
  op5.in.push_back(op4.idx);
  op5.fu = FunctionUnitLib::ROOT;
  ops.push_back(op5);
  pu.ops.push_back(op5.idx);

  pre_idx++;
  Operation op6;
  op6.idx = pre_idx;
  op6.in.push_back(-1);
  op6.in.push_back(op5.idx);
  op6.fu = FunctionUnitLib::DIV;
  ops.push_back(op6);
  pu.ops.push_back(op6.idx);

  c = op6.idx;

  pre_idx++;
  Operation op7;
  op7.idx = pre_idx;
  op7.in.push_back(op2.idx);
  op7.in.push_back(op6.idx);
  op7.fu = FunctionUnitLib::MUL;
  ops.push_back(op7);
  pu.ops.push_back(op7.idx);

  s = op7.idx;

  pre_idx++;
  Operation op8;
  op8.idx = pre_idx;
  op8.in.push_back(-1);
  op8.in.push_back(op7.idx);
  op8.fu = FunctionUnitLib::SUB;
  ops.push_back(op8);
  pu.ops.push_back(op8.idx);

  ns = op8.idx;

  pus.push_back(pu);

  name = "Pattern: c,s in Jacobi transformation";
  description = "generate c,s used in Jacobi transformation";

  cb = ComputeBlockLib::cbs["jacobi_s"];
}

Givens_cs::Givens_cs(vector<Operation> &ops, string &in_cp, int arp, int arq, bool zero_arp, int &c, int &s, int &ns){

  //cout << "in givens pattern, arp: " << arp << endl;
  //cout << "in givens pattern, arq: " << arq << endl;

  cp = in_cp;
  int pre_idx = ops.size()-1;

  PatternUnit pu;

  pre_idx++;
  Operation op1;
  op1.idx = pre_idx;
  op1.in.push_back(arp);
  op1.in.push_back(arp);
  op1.fu = FunctionUnitLib::MUL;
  ops.push_back(op1);
  pu.ops.push_back(op1.idx);

  pre_idx++;
  Operation op2;
  op2.idx = pre_idx;
  op2.in.push_back(arq);
  op2.in.push_back(arq);
  op2.fu = FunctionUnitLib::MUL;
  ops.push_back(op2);
  pu.ops.push_back(op2.idx);

  pre_idx++;
  Operation op3;
  op3.idx = pre_idx;
  op3.in.push_back(op1.idx);
  op3.in.push_back(op2.idx);
  op3.fu = FunctionUnitLib::ADD;
  ops.push_back(op3);
  pu.ops.push_back(op3.idx);

  pre_idx++;
  Operation op4;
  op4.idx = pre_idx;
  op4.in.push_back(op3.idx);
  op4.fu = FunctionUnitLib::ROOT;
  ops.push_back(op4);
  pu.ops.push_back(op4.idx);

  pre_idx++;
  Operation op5;
  op5.idx = pre_idx;
  if(zero_arp){
    op5.in.push_back(arq);
  }
  else{
    op5.in.push_back(arp);
  }
  op5.in.push_back(op4.idx);
  op5.fu = FunctionUnitLib::DIV;
  ops.push_back(op5);
  pu.ops.push_back(op5.idx);

  c = op5.idx;

  pre_idx++;
  Operation op6;
  op6.idx = pre_idx;
  if(zero_arp){
    op6.in.push_back(arp);
  }
  else{
    op6.in.push_back(arq);
  }
  op6.in.push_back(op4.idx);
  op6.fu = FunctionUnitLib::DIV;
  ops.push_back(op6);
  pu.ops.push_back(op6.idx);

  s = op6.idx;

  pre_idx++;
  Operation op7;
  op7.idx = pre_idx;
  op7.in.push_back(-1);
  op7.in.push_back(op6.idx);
  op7.fu = FunctionUnitLib::SUB;
  ops.push_back(op7);
  pu.ops.push_back(op7.idx);

  ns = op7.idx;

  pus.push_back(pu);

  name = "Pattern: C, S in Givens transformation";
  description = "generate c,s used in Givens transformation";

  cb = ComputeBlockLib::cbs["givens_s"];	
};

