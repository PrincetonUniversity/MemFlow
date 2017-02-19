#include<iostream>
#include<fstream>
#include<algorithm>
#include<math.h>
#include<queue>
#include<list>
#include<vector>
#include<set>
#include<array>
#include<string>
#include<chrono>
#include<memory>
#include<climits>

#include "Hardware.hpp"
#include "Scheduling.hpp"
#include "Setting.hpp"

using namespace std;


TileScheduling::TileScheduling(MacroNodeTemplate &in_macro, int in_mode):mode(in_mode), macro(in_macro){
  //tiles = &in_tiles;
  //ops = &in_ops;

  sum_cycle = 0;
  last_cycle = 0;

  //iterator over memory banks
  cout << "macro mem num_bank " << macro.mem->num_bank << endl;
  new_num_port = macro.mem->num_port; 
  new_num_live = macro.mem->num_live; 
  
  total_port=0;
  for(int i=0; i<macro.mem->num_bank; i++){
    total_port += macro.mem->membanks[i].num_port;
  }

  for(map<string, int>::iterator i=ComputeBlockLib::num_cb.begin(); i!=ComputeBlockLib::num_cb.end(); i++){
    num_cb_max[i->first] = 0;
  }

  for(vector<Tile>::iterator i=macro.tiles.begin(); i!=macro.tiles.end(); i++){
    tile_sche tile_sche_tp;
    tile_sche_tp.start = 0;
    tile_sche_tp.end = 0;
    macro.tiles_sche.push_back(tile_sche_tp);
  }
  cout << "initialize mn scheduling" << endl;
}

TileScheduling::~TileScheduling(){
  for(auto it=original_inop_lrs.begin(); it!=original_inop_lrs.end(); it++){
    delete it->second;
  }
  for(auto it=updated_inop_lrs.begin(); it!=updated_inop_lrs.end(); it++){
    delete it->second;
  }
  for(auto it=tilein_lrs.begin(); it!=tilein_lrs.end(); it++){
    delete it->second;
  }
  for(auto it=tileout_lrs.begin(); it!=tileout_lrs.end(); it++){
    delete it->second;
  }
  for(auto it=spill_lrs.begin(); it!=spill_lrs.end(); it++){
    delete it->second;
  }
  for(auto it=bankex_lrs.begin(); it!=bankex_lrs.end(); it++){
    delete it->second;
  }

}


void TileScheduling::PrintScheduling(){
  cout << "current scheduling: " << endl;
  cout << "tiles sche size " << macro.tiles_sche.size() << endl;
  for(vector<tile_sche>::iterator i=macro.tiles_sche.begin(); i!=macro.tiles_sche.end(); i++){
    int tile = i-macro.tiles_sche.begin();
    cout << "tile " << tile << endl;
    cout << "start at " << i->start << endl;
    cout << "end at " << i->end << endl;
    for(map<int,int>::iterator j=macro.tiles[tile].livein_ops.begin(); j!=macro.tiles[tile].livein_ops.end(); j++){
      cout << "livein_op " << j->first << " read at " << i->start+j->second << endl;
    }
    for(vector<int>::iterator j = macro.tiles[tile].liveout_ops.begin(); j!=macro.tiles[tile].liveout_ops.end(); j++){
      cout << "liveout_op " << *j << endl;
    }
    cout << endl;
  }

}


//*************Liverange

void Liverange::GetLiverange(int in_op, int cycle){

  op = in_op;

  mem_bank = sche->macro.mem->getOpBank(op, cycle);
  mem_loc = sche->macro.mem->getOpLoc(op, cycle);

  allocated = true;

  write_cycle = cycle;
  while((sche->macro.mem->getOpBank(op,write_cycle) == mem_bank)
      &&(sche->macro.mem->getOpLoc(op,write_cycle == mem_loc))){
    write_cycle--;
  }

  last_cycle = cycle;
  while((sche->macro.mem->getOpBank(op,last_cycle) == mem_bank)
      &&(sche->macro.mem->getOpLoc(op,last_cycle == mem_loc))){
    last_cycle++;
  }
  last_cycle--;

  //find the read
  read_cycles.insert(last_cycle);

  int tile = sche->macro.ops[op].tile;
  if(tile == -1){
    for(set<int>::iterator user_op=sche->macro.ops[op].out.begin(); user_op!=sche->macro.ops[op].out.end(); user_op++){
      int user_tile = sche->macro.ops[*user_op].tile;
      if(sche->macro.tiles_sche[user_tile].start != 0){
	int read_cycle = sche->macro.tiles_sche[user_tile].start+sche->macro.tiles[user_tile].livein_ops[op];
	if((read_cycle > write_cycle) && (read_cycle <= last_cycle)){
	  if(read_cycles.find(read_cycle) == read_cycles.end()){
	    read_cycles.insert(read_cycle);
	  }
	}
      }
    }
  }
  else{
    for(vector<int>::iterator user_tile=sche->macro.tiles[tile].post_tiles[op].begin(); user_tile!=sche->macro.tiles[tile].post_tiles[op].end(); user_tile++){
      if(sche->macro.tiles_sche[*user_tile].start != 0){
	int read_cycle = sche->macro.tiles_sche[*user_tile].start+sche->macro.tiles[*user_tile].livein_ops[op];
	if((read_cycle > write_cycle) && (read_cycle <= last_cycle)){
	  if(read_cycles.find(read_cycle) == read_cycles.end()){
	    read_cycles.insert(read_cycle);
	  }
	}
      }
    }
  }
}

void Liverange::UpdateLiverange(int old_read_cycle, int new_read_cycle){

  if(read_cycles.find(old_read_cycle) != read_cycles.end()){
    if(old_read_cycle == last_cycle){
      if(sche->macro.op_in_cycle[op] == old_read_cycle){
	read_cycles.erase(old_read_cycle);
      }
    }
    else{
      read_cycles.erase(old_read_cycle);
    }
  }

  if(read_cycles.find(new_read_cycle) == read_cycles.end()){
    read_cycles.insert(new_read_cycle);
  }

  last_cycle = 0;
  for(set<int>::iterator i=read_cycles.begin(); i!=read_cycles.end(); i++){
    if(*i > last_cycle){
      last_cycle = *i;
    }
  }
}

void Liverange::UpdateLR(int tile, int new_read_cycle){
  if(new_read_cycle < sche->macro.op_in_cycle[op]){
    if(read_cycles.find(new_read_cycle) == read_cycles.end()){
      read_cycles.insert(new_read_cycle);
    }
  }
  else if(new_read_cycle > sche->macro.op_in_cycle[op]){
    last_cycle = new_read_cycle;

    //whether to delete ready at op_in_cycle
    bool first_user = true;
    int in_tile = sche->macro.ops[op].tile;
    if(in_tile != -1){
      for(vector<int>::iterator user_tile=sche->macro.tiles[in_tile].post_tiles[op].begin();user_tile!=sche->macro.tiles[in_tile].post_tiles[op].end(); user_tile++){
	if((*user_tile != tile) && (sche->macro.tiles_sche[*user_tile].start != 0)){
	  first_user = false;
	  break;
	}
      }	    
      if(first_user)
	read_cycles.erase(sche->macro.op_in_cycle[op]);
    } 

    read_cycles.insert(new_read_cycle);
  }

}

void Liverange::RemoveLRfromMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live, vector<int> &num_port_left){
  allocated = false;
  new_num_port->at(mem_bank)[write_cycle]--;
  num_port_left[write_cycle]++;
  for(int c=write_cycle+1; c<=last_cycle; c++){
    new_num_live->at(mem_bank)[mem_loc][c]--;
  }
  for(set<int>::iterator read=read_cycles.begin(); read!=read_cycles.end(); read++){
    new_num_port->at(mem_bank)[*read]--;
    num_port_left[*read]++;
  }
}

void Liverange::RemoveLRfromMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live){
  allocated = false;
  new_num_port->at(mem_bank)[write_cycle]--;
  for(int c=write_cycle+1; c<=last_cycle; c++){
    new_num_live->at(mem_bank)[mem_loc][c]--;
  }
  for(set<int>::iterator read=read_cycles.begin(); read!=read_cycles.end(); read++){
    new_num_port->at(mem_bank)[*read]--;
  }
}

void Liverange::AddLRtoMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live, vector<int> &num_port_left){
  allocated = true;
  new_num_port->at(mem_bank)[write_cycle]++;
  num_port_left[write_cycle]--;

  for(int c=write_cycle+1; c<=last_cycle; c++){
    new_num_live->at(mem_bank)[mem_loc][c]++;
  }
  for(set<int>::iterator read=read_cycles.begin(); read!=read_cycles.end(); read++){
    new_num_port->at(mem_bank)[*read]++;
    num_port_left[*read]--;
  }
}

void Liverange::AddLRtoMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live){
  allocated = true;
  new_num_port->at(mem_bank)[write_cycle]++;
  for(int c=write_cycle+1; c<=last_cycle; c++){
    new_num_live->at(mem_bank)[mem_loc][c]++;
  }
  for(set<int>::iterator read=read_cycles.begin(); read!=read_cycles.end(); read++){
    new_num_port->at(mem_bank)[*read]++;
  }
}

void Liverange::CanAddLRtoMem(vector<vector<int>>* new_num_port, vector<vector<vector<int>>>* new_num_live){
  can_add = true;
  if(new_num_port->at(mem_bank)[write_cycle]+1 > sche->macro.mem->membanks[mem_bank].num_port){
    can_add = false;
    cause = "write";
  }
  else{
    for(set<int>::iterator read=read_cycles.begin(); read!=read_cycles.end(); read++){
      if(new_num_port->at(mem_bank)[*read]+1 > sche->macro.mem->membanks[mem_bank].num_port){
	can_add = false;
	cause = "read";
	break;
      }
    }
    if(can_add){
      for(int c=write_cycle+1; c<=last_cycle; c++){
	if(new_num_live->at(mem_bank)[mem_loc][c]+1 > 1){
	  can_add = false;
	  cause = "live";
	  break;
	}
      }
    }
  }
}

//***********End Liverange



bool TileScheduling::CanAddLRstoMem_bf(set<Liverange*> &lrs, vector<vector<int>> &num_port, vector<vector<vector<int>>> &num_live){
  vector<vector<int>> num_port_c = num_port;
  vector<vector<vector<int>>> num_live_c = num_live;

  Liverange* lr = *lrs.begin();

  for(int i=0; i<macro.mem->num_bank; i++){
    lr->mem_bank = i;
    for(int j=0; j<macro.mem->membanks[i].size; j++){
      lr->mem_loc = j;

      lr->CanAddLRtoMem(&num_port_c, &num_live_c);
      if(lr->can_add){
	lr->AddLRtoMem(&num_port_c, &num_live_c);
	if(lrs.size() == 1){
	  return true;
	}
	else{
	  set<Liverange*> lrs_c = lrs;
	  lrs_c.erase(lrs_c.begin());

	  if(CanAddLRstoMem_bf(lrs_c, num_port_c, num_live_c)){
	    return true;
	  }
	  else{
	    lr->RemoveLRfromMem(&num_port_c, &num_live_c);
	    continue;
	  }
	}
      }
      else{
	continue;
      }

    }
  }
  return false;
}

struct lr_size{
  bool operator()(const Liverange* lr1, const Liverange* lr2){
    if(lr1->read_cycles.size() > lr2->read_cycles.size()){
      return true;
    }
    return false;
  }
};

bool TileScheduling::CanAddLRstoMem(set<Liverange*> &lrs, vector<vector<int>> &num_port, vector<vector<vector<int>>> &num_live){
  vector<vector<int>> num_port_c = num_port;
  vector<vector<vector<int>>> num_live_c = num_live;
  
  for(set<Liverange*>::iterator i=lrs.begin(); i!=lrs.end(); i++){ 
    Liverange* lr = *i;

    bool has_mem = false;
    for(int i=0; i<macro.mem->num_bank; i++){
      lr->mem_bank = i;
      for(int j=0; j<macro.mem->membanks[i].size; j++){
	lr->mem_loc = j;
	lr->CanAddLRtoMem(&num_port_c, &num_live_c);
	if(lr->can_add){
	  lr->AddLRtoMem(&num_port_c, &num_live_c);
	  has_mem = true;
	  break;
	}
      }
      if(has_mem){
        break;
      }
    }
    if(!has_mem){
      return false;
    }
  }
  return true;
}

void Liverange::PrintLR(){
  cout << "LR op: " << op << endl;
  cout << "mem bank: " << mem_bank << endl;
  cout << "mem loc: " << mem_loc << endl;
  cout << "write cycle " << write_cycle << endl;
  cout << "last cycle " << last_cycle << endl;
  cout << "read cycles: ";
  for(set<int>::iterator r=read_cycles.begin(); r!=read_cycles.end(); r++){
    cout << *r << ",";
  }
  cout << endl;
}

void TileScheduling::ExtendMemData(int max_cycle){
  for(int i=0; i<macro.mem->num_bank; i++){
    for(int j=0; j<macro.mem->membanks[i].size; j++){
      if(int(macro.mem->num_live[i][j].size())-1 < max_cycle){
	for(int n=macro.mem->num_live[i][j].size(); n<=max_cycle; n++){
	  macro.mem->num_live[i][j].push_back(0);
	}
      }      
    }
    if(int(macro.mem->num_port[i].size())-1 < max_cycle){
      for(int n=macro.mem->num_port[i].size(); n<=max_cycle; n++){
	macro.mem->num_port[i].push_back(0);
      }
    }
    if(int(num_port_left.size())-1 < max_cycle){
      for(int n=num_port_left.size(); n<=max_cycle; n++){
	num_port_left.push_back(total_port);
      }
    }

  }

}

void TileScheduling::ExtendMemData(int max_cycle, vector<vector<int>>& new_num_port, vector<vector<vector<int>>>& new_num_live){
  for(int i=0; i<macro.mem->num_bank; i++){
    for(int j=0; j<macro.mem->membanks[i].size; j++){
      if(int(new_num_live[i][j].size())-1 < max_cycle){
        for(int n=new_num_live[i][j].size(); n<=max_cycle; n++){
	  new_num_live[i][j].push_back(0);
        }
      }      
    }
    if(int(new_num_port[i].size())-1 < max_cycle){
      for(int n=new_num_port[i].size(); n<=max_cycle; n++){
	new_num_port[i].push_back(0);
      }
    }
  }

}

void TileScheduling::ExtendNumCB(int max_cycle){
  for(map<string, int>::iterator i=ComputeBlockLib::num_cb.begin(); i!=ComputeBlockLib::num_cb.end(); i++){
    if(int(num_cb[i->first].size())-1 < max_cycle){
      for(int n=num_cb[i->first].size(); n<=max_cycle; n++){
	num_cb[i->first].push_back(0);
      }
    }
  }
}

void TileScheduling::ExtendOpinBank(int max_cycle){
  for(vector<Tile>::iterator i=macro.tiles.begin(); i!=macro.tiles.end(); i++){
    for(map<int,int>::iterator in_op=i->livein_ops.begin(); in_op!=i->livein_ops.end(); in_op++){
      if(int(macro.mem->op_in_bank[in_op->first].size())-1 < max_cycle){
	for(int n=macro.mem->op_in_bank[in_op->first].size(); n<=max_cycle; n++){
	  array<int,2> a = {-1,-1};
	  macro.mem->op_in_bank[in_op->first].push_back(a);
	}
      }
    }
    for(vector<int>::iterator out_op=i->liveout_ops.begin(); out_op!=i->liveout_ops.end(); out_op++){
      if(int(macro.mem->op_in_bank[*out_op].size())-1 < max_cycle){
	for(int n=macro.mem->op_in_bank[*out_op].size(); n<=max_cycle; n++){
	  array<int,2> a = {-1,-1};
	  macro.mem->op_in_bank[*out_op].push_back(a);
	}
      }
    }
  }

}


bool TileScheduling::AllocateLiveout(int cycle, int tile){
  int write_cycle = cycle+macro.tiles[tile].latency-1;
  for(vector<int>::iterator out_op=macro.tiles[tile].liveout_ops.begin(); out_op!=macro.tiles[tile].liveout_ops.end(); out_op++){
    int cycle_to_read = write_cycle+1;
    int bank;
    int loc;

    set<int> banks;
    for(int i=0; i<macro.mem->num_bank; i++){
      if(new_num_port[i][write_cycle]+1 <= macro.mem->membanks[i].num_port){
	banks.insert(i);
      }
    }
    if(banks.empty()){
      return false;
    }
    else{
      bool can_read = false;
      while(!can_read){
	vector<int> bank_to_remove;
	for(set<int>::iterator b=banks.begin(); b!=banks.end(); b++){
	  //bank: *b
	  bool has_live = false;
	
	  int j;
          for(j=0; j<macro.mem->membanks[*b].size; j++){
	    bool has_loc = true;
	    for(int c=write_cycle+1; c<=cycle_to_read; c++){
	      if(new_num_live[*b][j][c]+1 > 1){
		has_loc = false;
		break;
	      }
	    }
	    if(has_loc){
	      has_live = true;
	      break;
	    }
	  }
	  if(has_live){
	    if(new_num_port[*b][cycle_to_read]+1 <= macro.mem->membanks[*b].num_port){
	      can_read = true;
	      bank = *b;
	      loc = j;
	      break;
	    }
	  }
	  else{
	    bank_to_remove.push_back(*b);
	  }
	}
	if(!can_read){
	  for(vector<int>::iterator b=bank_to_remove.begin(); b!=bank_to_remove.end(); b++){
	    banks.erase(*b);
	  }
	  if(banks.empty()){
	    break;
	  }
	  else{
	    cycle_to_read++;
	    ExtendMemData(cycle_to_read);
	    ExtendMemData(cycle_to_read, new_num_port, new_num_live);
	    ExtendNumCB(cycle_to_read);
	    ExtendOpinBank(cycle_to_read);
	  }
	}
      }
      if(!can_read){
	return false;
      }
      else{
	Liverange* lr = new Liverange(this);
	lr->op = *out_op;
	lr->write_cycle = write_cycle;
	lr->read_cycles.insert(cycle_to_read);
	lr->last_cycle = cycle_to_read;
	lr->mem_bank = bank;
	lr->mem_loc = loc;
	lr->AddLRtoMem(&new_num_port, &new_num_live);
	tileout_lrs[*out_op] = lr;
      }
    }
  }
  return true;
}

bool TileScheduling::RemoveLiveinLR(int cycle, int tile){

  for(map<int,int>::iterator in_op=macro.tiles[tile].livein_ops.begin(); in_op!=macro.tiles[tile].livein_ops.end(); in_op++){
    int read_cycle = cycle+in_op->second;

    if((read_cycle < macro.op_in_cycle[in_op->first]) && (macro.mem->op_in_bank[in_op->first][read_cycle][0] < 0)){
      return false;
    }

    Liverange *lr = new Liverange(this);
    if(read_cycle > macro.op_in_cycle[in_op->first]){
      lr->GetLiverange(in_op->first, macro.op_in_cycle[in_op->first]);
    }		 
    else{
      lr->GetLiverange(in_op->first, read_cycle);
    }
    lr->RemoveLRfromMem(&new_num_port, &new_num_live);
    original_inop_lrs[in_op->first] = lr;
  }
  return true;
}



bool TileScheduling::CanAddwithMaxSpilling(int cycle, int tile){

  for(map<int,int>::iterator in_op=macro.tiles[tile].livein_ops.begin(); in_op!=macro.tiles[tile].livein_ops.end(); in_op++){
    int read_cycle = cycle+in_op->second;
    if(macro.op_in_cycle.find(in_op->first) != macro.op_in_cycle.end()){
      if(read_cycle > macro.op_in_cycle[in_op->first]){
	Liverange* tilein_lr = new Liverange(*original_inop_lrs[in_op->first]);
	tilein_lrs[in_op->first] = tilein_lr;
      }
      else{
	Liverange* tilein_lr = new Liverange(*updated_inop_lrs[in_op->first]);
	tilein_lrs[in_op->first] = tilein_lr;
      }
    }
    else{
      Liverange* tilein_lr = new Liverange(*updated_inop_lrs[in_op->first]);
      tilein_lrs[in_op->first] = tilein_lr;
    }
  }

  //priority_queue<Liverange*, vector<Liverange*>, lr_size> lrs_min;
  set<Liverange*> lrs_min;
  for(map<int, Liverange*>::iterator l=tilein_lrs.begin(); l!=tilein_lrs.end(); l++){
    lrs_min.insert(l->second);
  }

  if(!CanAddLRstoMem(lrs_min, new_num_port, new_num_live)){
    //if(!CanUpdateLRsinMem(original_inop_lrs, tilein_lrs, new_num_read, new_num_write, new_num_live)){
    return false;
  }
  else{

    bool can_add = true;;
    //add lrs_min first;
    vector<vector<int>> new_num_port_c = new_num_port;
    vector<vector<vector<int>>> new_num_live_c = new_num_live;

    for(map<int, Liverange*>::iterator lr=tilein_lrs.begin(); lr!=tilein_lrs.end(); lr++){
      //original_inop_lrs[lr->first].RemoveLRfromMem(&new_num_read_c, &new_num_write_c, &new_num_live_c);
      lr->second->AddLRtoMem(&new_num_port_c, &new_num_live_c);
    }

    //need to add another part of lrs for those read_cycle > op_in_cycle
    for(map<int,int>::iterator in_op=macro.tiles[tile].livein_ops.begin(); in_op!=macro.tiles[tile].livein_ops.end(); in_op++){
      int read_cycle = cycle+in_op->second;
      if((macro.op_in_cycle.find(in_op->first) != macro.op_in_cycle.end()) && (read_cycle > macro.op_in_cycle[in_op->first])){
	//consider add bankex
	int bankex_start = macro.op_in_cycle[in_op->first];
	int bankex_end = bankex_start+BANKEX_LATENCY-1;

	if(bankex_end <= read_cycle-1){
	  //shared_ptr<Liverange> lr1(new Liverange(*tilein_lrs[in_op->first]));
	  Liverange* lr1 = new Liverange(*tilein_lrs[in_op->first]);
	  lr1->RemoveLRfromMem(&new_num_port_c, &new_num_live_c);

	  bool can_bankex = false;
	  while(bankex_end <= read_cycle-1){
	    //adjust first part of lr
	    lr1->UpdateLR(tile, bankex_start);

	    Liverange* lr_bankex = new Liverange(this);
	    lr_bankex->op = in_op->first;
	    lr_bankex->write_cycle = bankex_end;
	    lr_bankex->read_cycles.insert(read_cycle);
	    lr_bankex->last_cycle = read_cycle;

	    //priority_queue<Liverange*, vector<Liverange*>, lr_size> lrs_toadd;
	    set<Liverange*> lrs_toadd;
	    lrs_toadd.insert(lr1);
	    lrs_toadd.insert(lr_bankex);
	    if(CanAddLRstoMem_bf(lrs_toadd, new_num_port_c, new_num_live_c)){
	      //can add this bankex
	      lr1->AddLRtoMem(&new_num_port_c, &new_num_live_c);
	      lr_bankex->AddLRtoMem(&new_num_port_c, &new_num_live_c);

	      delete tilein_lrs[in_op->first];
	      tilein_lrs[in_op->first] = new Liverange(*lr1);
	      bankex_lrs[in_op->first] = lr_bankex;

	      can_bankex = true;
	      break;
	    }
	    else{
	      bankex_end++;
	      bankex_start++;
	    }
	  }
	  if(!can_bankex){
	    //consider spilling
	    //int spill_idx;
	    int spill_start = macro.op_in_cycle[in_op->first];
	    int spill_latency;
	    if(macro.tiles[tile].cb->name == "load"){
	      spill_latency = LOAD_LATENCY;
	    }
	    else{
	      spill_latency = LOAD_LATENCY+STORE_LATENCY;
	    }

	    //for(vector<int>::iterator sp=op_spills[in_op->first].begin(); sp!=op_spills[in_op->first].end(); sp++){
	    //   if(spills_sche[*sp].cond == 0){
	    //      spill_idx = *sp;
	    //      break;
	    //   }
	    //}
	    int spill_end = spill_start+spill_latency-1;
	    if(spill_end <= read_cycle-1){
	      delete lr1;
	      lr1 = new Liverange(*original_inop_lrs[in_op->first]);

	      bool can_spill = false;
	      while(spill_end <= read_cycle-1){
		Liverange* lr_spill = new Liverange(this);
		lr_spill->op = in_op->first;
		lr_spill->write_cycle = spill_end;
		lr_spill->read_cycles.insert(read_cycle);
		lr_spill->last_cycle = read_cycle;

		//priority_queue<Liverange*, vector<Liverange*>, lr_size> lrs_toadd;
		set<Liverange*> lrs_toadd;
		lrs_toadd.insert(lr1);
		lrs_toadd.insert(lr_spill);
		if(CanAddLRstoMem_bf(lrs_toadd, new_num_port_c, new_num_live_c)){
		  lr1->AddLRtoMem(&new_num_port_c, &new_num_live_c);
		  lr_spill->AddLRtoMem(&new_num_port_c, &new_num_live_c);

		  delete tilein_lrs[in_op->first];
		  tilein_lrs[in_op->first] = new Liverange(*lr1);
		  //spillidx[in_op->first] = spill_idx;
		  spill_lrs[in_op->first] = lr_spill;			        

		  can_spill = true;
		  break;
		}
		else{
		  spill_end++;
		}
	      }
	      if(!can_spill){
		can_add = false;
		delete lr1;
		break;
	      }
	    }
	    else{
	      //not enough cycle space for spill
	      can_add = false;
	      delete lr1;
	      break;
	    }
	  }
	  delete lr1;
	}
	else{
	  //not enough cycle space for bankex
	  can_add = false;
	  break;
	}
      }
    }
    if(!can_add){
      //lrs_select = lrs_select_c;
      return false;
    }
  }
  return true;
  }

  /*
     void TileScheduling::RemoveSpilling(int tile){

     vector<int> bankex_to_remove;
     for(map<int, Liverange>::iterator bs=bankex_lrs.begin(); bs!=bankex_lrs.end(); bs++){
     map<int,Liverange> lrs_select_c;
     for(map<int, Liverange>::iterator it_lr = tilein_lrs.begin(); it_lr!=tilein_lrs.end(); it_lr++){
     lrs_select_c[it_lr->first] = Liverange(it_lr->second);
     }

     tilein_lrs[bs->first] = updated_inop_lrs[bs->first];

     Liverange lr_bankex = bs->second;
     lr_bankex.RemoveLRfromMem(&new_num_read, &new_num_write, &new_num_live);

//priority_queue<Liverange*, vector<Liverange*>, lr_size> lrs_select_p;
set<Liverange*> lrs_select_p;
for(map<int, Liverange>::iterator l=tilein_lrs.begin(); l!=tilein_lrs.end(); l++){
lrs_select_p.insert(&l->second);
}
if(CanAddLRstoMem_bf(lrs_select_p, new_num_read, new_num_write, new_num_live)){
bankex_to_remove.push_back(bs->first);
}
else{
tilein_lrs = lrs_select_c;
lr_bankex.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}

}

for(vector<int>::iterator bs=bankex_to_remove.begin(); bs!=bankex_to_remove.end(); bs++){
bankex_lrs.erase(*bs);
}

vector<int> spill_to_remove;
for(map<int, Liverange>::iterator sp=spill_lrs.begin(); sp!=spill_lrs.end(); sp++){
map<int,Liverange> lrs_select_c;
for(map<int, Liverange>::iterator it_lr = tilein_lrs.begin(); it_lr!=tilein_lrs.end(); it_lr++){
lrs_select_c[it_lr->first] = Liverange(it_lr->second);
}

tilein_lrs[sp->first] = updated_inop_lrs[sp->first];			     

Liverange lr_spill = sp->second;
lr_spill.RemoveLRfromMem(&new_num_read, &new_num_write, &new_num_live);

//priority_queue<Liverange*, vector<Liverange*>, lr_size> lrs_select_p;
set<Liverange*> lrs_select_p;
for(map<int, Liverange>::iterator l=tilein_lrs.begin(); l!=tilein_lrs.end(); l++){
lrs_select_p.insert(&l->second);
}
if(CanAddLRstoMem_bf(lrs_select_p, new_num_read, new_num_write, new_num_live)){
spill_to_remove.push_back(sp->first);
}
else{
tilein_lrs = lrs_select_c;

int read_cycle = updated_inop_lrs[sp->first].last_cycle;
int bankex_start = original_inop_lrs[sp->first].last_cycle;
int bankex_end = bankex_start+BANKEX_LATENCY-1;

bool can_bankex = false;
while(bankex_end <= read_cycle-1){
Liverange lr_bankex;
lr_bankex.op = sp->first;
lr_bankex.write_cycle = bankex_end;
lr_bankex.read_cycles.insert(read_cycle);
lr_bankex.last_cycle = read_cycle;

tilein_lrs[sp->first].UpdateLR(tile, bankex_start);

//priority_queue<Liverange*, vector<Liverange*>, lr_size> lrs_p;
set<Liverange*> lrs_p;
for(map<int, Liverange>::iterator l=tilein_lrs.begin(); l!=tilein_lrs.end(); l++){
  lrs_p.insert(&l->second);
}
lrs_p.insert(&lr_bankex);

//consider bankex
if(CanAddLRstoMem_bf(lrs_p, new_num_read, new_num_write, new_num_live)){
  spill_to_remove.push_back(sp->first);

  bankex_lrs[sp->first] = lr_bankex;
  lr_bankex.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);

  can_bankex = true;
  break;
}
else{
  tilein_lrs = lrs_select_c;
  bankex_start++;
  bankex_end++;
}
}
if(!can_bankex){
  tilein_lrs = lrs_select_c;
  lr_spill.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}
}

}

for(vector<int>::iterator sp=spill_to_remove.begin(); sp!=spill_to_remove.end(); sp++){
  spill_lrs.erase(*sp);
}
}
*/


/*
   bool Scheduling::CheckCycle(int cycle, int tile){

   ComputeBlock* cb = tiles[tile].cb;
   if(num_cb[cb->name][cycle] >= ComputeBlockLib::num_cb[cb->name]){
//cout << " no hw cb" << endl;
return false;
}

if(tiles[tile].liveout_ops.size()!=0){
//CHECK: can handle liveout ops*************************************
bool can_output;
if(!AllocateLiveout(cycle, tile)){
//cout << " can't allocate liveout" << endl;
return false;
}
}

if(tiles[tile].livein_ops.size() != 0){

for(map<int,int>::iterator in_op=tiles[tile].livein_ops.begin(); in_op!=tiles[tile].livein_ops.end(); in_op++){
int read_cycle = cycle+in_op->second;
if((read_cycle < op_in_cycle[in_op->first]) && (op_in_bank[in_op->first][read_cycle] < 0)){
return false;
}
}

for(map<int,int>::iterator in_op=tiles[tile].livein_ops.begin(); in_op!=tiles[tile].livein_ops.end(); in_op++){
int read_cycle = cycle+in_op->second;
Liverange lr;
if(read_cycle > op_in_cycle[in_op->first]){
lr.GetLiverange(in_op->first, op_in_cycle[in_op->first], this);
}		 
else{
lr.GetLiverange(in_op->first, read_cycle, this);
}
original_inop_lrs[in_op->first] = lr;

Liverange lr_updated = Liverange(lr);
lr_updated.UpdateLR(tile, read_cycle);
updated_inop_lrs[in_op->first] = lr_updated;
}


if(CanUpdateLRsinMem(original_inop_lrs, updated_inop_lrs, new_num_read, new_num_write, new_num_live)){
cout << "can update initial lrs" << endl;
for(map<int, Liverange>::iterator lr=updated_inop_lrs.begin(); lr!=updated_inop_lrs.end(); lr++){
original_inop_lrs[lr->first].RemoveLRfromMem(&new_num_read, &new_num_write, &new_num_live);
tilein_lrs[lr->first] = lr->second;
lr->second.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}
}
else{
if(!CanAddwithMaxSpilling(cycle,tile)){
cout << "can't allocate with max spilling" << endl;
//cout << " can't allocate livein wth max spilling" << endl;
return false;
}
else{
for(map<int, Liverange>::iterator bs=bankex_lrs.begin(); bs!=bankex_lrs.end(); bs++){
bs->second.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}
for(map<int, Liverange>::iterator sp=spill_lrs.begin(); sp!=spill_lrs.end(); sp++){
sp->second.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}

RemoveSpilling(tile);

for(map<int,Liverange>::iterator lr=tilein_lrs.begin(); lr!=tilein_lrs.end(); lr++){
//tilein_lrs[lr->first] = lr->second;
original_inop_lrs[lr->first].RemoveLRfromMem(&new_num_read, &new_num_write, &new_num_live);
lr->second.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}
}

}

}

//allocate updated lrs
for(int i=0; i<Memory::num_bank; i++){
  for(int c=0; c<new_num_read[i].size(); c++){
    if(new_num_read[i][c] > Memory::membanks[i].num_readport){
      cout << "new num read of bank " << i << " at cycle " << c << " is " << new_num_read[i][c] << endl; 
      throw exception();
    }
  }
}

return true;

}
*/

/*
   bool TileScheduling::CheckCycle(int cycle, int tile){
//cout << "check cycle " << cycle << endl;
ComputeBlock* cb = macro.tiles[tile].cb;

//CHECK: if has cb available***************************************
if(num_cb[cb->name][cycle] >= ComputeBlockLib::num_cb[cb->name]){
//cout << " no hw cb" << endl;
return false;
}

if(macro.tiles[tile].liveout_ops.size()!=0){
//CHECK: can handle liveout ops*************************************
bool can_output;
if(!AllocateLiveout(cycle, tile)){
//cout << " can't allocate liveout" << endl;
return false;
}
}

if(macro.tiles[tile].livein_ops.size() != 0){
//CHECK: if can handle livein ops***************************************************
if(!RemoveLiveinLR(cycle, tile)){
//cout << " input spilled out" << endl;
return false;
}

//update lr;
for(map<int,int>::iterator in_op=macro.tiles[tile].livein_ops.begin(); in_op!=macro.tiles[tile].livein_ops.end(); in_op++){
int read_cycle = cycle+in_op->second;
Liverange lr(original_inop_lrs[in_op->first]);
lr.UpdateLR(tile, read_cycle);
updated_inop_lrs[in_op->first] = lr;
}

set<Liverange*> updated_inop_lrs_p;
for(map<int, Liverange>::iterator l=updated_inop_lrs.begin(); l!=updated_inop_lrs.end(); l++){
updated_inop_lrs_p.insert(&l->second);
}
chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
if(CanAddLRstoMem_bf(updated_inop_lrs_p, new_num_read, new_num_write, new_num_live)){
chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
auto duration = chrono::duration_cast<chrono::microseconds>(t2-t1).count();	      
cout << "time spent on check initial lrs allocation " << duration << endl;

for(map<int, Liverange>::iterator lr=updated_inop_lrs.begin(); lr!=updated_inop_lrs.end(); lr++){
tilein_lrs[lr->first] = lr->second;
lr->second.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}
}
else{
chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
auto duration = chrono::duration_cast<chrono::microseconds>(t2-t1).count();	      
cout << "time spent on check initial lrs allocation " << duration << endl;

//Algorithm: if can add lrs with max spill, then remove redundent spill and bankex iteratively

if(!CanAddwithMaxSpilling(cycle,tile)){
//cout << " can't allocate livein wth max spilling" << endl;
return false;
}
else{
for(map<int, Liverange>::iterator bs=bankex_lrs.begin(); bs!=bankex_lrs.end(); bs++){
bs->second.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}
for(map<int, Liverange>::iterator sp=spill_lrs.begin(); sp!=spill_lrs.end(); sp++){
sp->second.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}

//RemoveSpilling(tile);

for(map<int,Liverange>::iterator lr=tilein_lrs.begin(); lr!=tilein_lrs.end(); lr++){
  //tilein_lrs[lr->first] = lr->second;
  lr->second.AddLRtoMem(&new_num_read, &new_num_write, &new_num_live);
}
}
}

for(int i=0; i<Memory::num_bank; i++){
  for(int c=0; c<new_num_read[i].size(); c++){
    if(new_num_read[i][c] > Memory::membanks[i].num_readport){
      cout << "new num read of bank " << i << " at cycle " << c << " is " << new_num_read[i][c] << endl; 
      throw exception();
    }
  }
}
}
return true;

}
*/

bool TileScheduling::CheckCycle_MN(int cycle, int tile){

  ComputeBlock* cb = macro.tiles[tile].cb;

  if(num_cb[cb->name][cycle] >= ComputeBlockLib::num_cb[cb->name]){
    return false;
  }

  if(macro.tiles[tile].liveout_ops.size()!=0){
    //CHECK: can handle liveout ops*************************************
    if(!AllocateLiveout(cycle, tile)){
      return false;
    }
  }

  if(macro.tiles[tile].livein_ops.size() != 0){
    for(map<int,int>::iterator in_op=macro.tiles[tile].livein_ops.begin(); in_op!=macro.tiles[tile].livein_ops.end(); in_op++){
      if(macro.op_in_cycle.find(in_op->first) != macro.op_in_cycle.end()){
	int read_cycle = cycle+in_op->second;
	if((read_cycle < macro.op_in_cycle[in_op->first]) && (macro.mem->op_in_bank[in_op->first][read_cycle][0] < 0)){
	  return false;
	}

	Liverange* lr = new Liverange(this);
	if(read_cycle > macro.op_in_cycle[in_op->first]){
	  lr->GetLiverange(in_op->first, macro.op_in_cycle[in_op->first]);
	}		 
	else{
	  lr->GetLiverange(in_op->first, read_cycle);
	}

	lr->RemoveLRfromMem(&new_num_port, &new_num_live);
	original_inop_lrs[in_op->first] = lr;

      }
    }

    //udpate lrs
    for(map<int,int>::iterator in_op=macro.tiles[tile].livein_ops.begin(); in_op!=macro.tiles[tile].livein_ops.end(); in_op++){
      int read_cycle = cycle+in_op->second;
      if(original_inop_lrs.find(in_op->first) != original_inop_lrs.end()){
	Liverange* lr = new Liverange(*original_inop_lrs[in_op->first]);
	lr->UpdateLR(tile, read_cycle);
	updated_inop_lrs[in_op->first] = lr;
      }
      else{
	Liverange* lr = new Liverange(this);
	lr->op = in_op->first;
	lr->write_cycle = read_cycle-1;
	lr->read_cycles.insert(read_cycle);
	lr->last_cycle = read_cycle;
	updated_inop_lrs[in_op->first] = lr;
      }
    }

    set<Liverange*> updated_inop_lrs_p;
    for(map<int, Liverange*>::iterator l=updated_inop_lrs.begin(); l!=updated_inop_lrs.end(); l++){
      updated_inop_lrs_p.insert(l->second);
    }

    if(CanAddLRstoMem(updated_inop_lrs_p, new_num_port, new_num_live)){
      for(map<int, Liverange*>::iterator lr=updated_inop_lrs.begin(); lr!=updated_inop_lrs.end(); lr++){
	Liverange* tilein_lr = new Liverange(*lr->second);
	tilein_lrs[lr->first] = tilein_lr;
	lr->second->AddLRtoMem(&new_num_port, &new_num_live);
      }
    }
    else{
      if(!CanAddwithMaxSpilling(cycle,tile)){
	return false;
      }
      else{
	for(map<int, Liverange*>::iterator bs=bankex_lrs.begin(); bs!=bankex_lrs.end(); bs++){
	  (bs->second)->AddLRtoMem(&new_num_port, &new_num_live);
	}
	for(map<int, Liverange*>::iterator sp=spill_lrs.begin(); sp!=spill_lrs.end(); sp++){
	  (sp->second)->AddLRtoMem(&new_num_port, &new_num_live);
	}

	//RemoveSpilling(tile);

	for(map<int,Liverange*>::iterator lr=tilein_lrs.begin(); lr!=tilein_lrs.end(); lr++){
	  lr->second->AddLRtoMem(&new_num_port, &new_num_live);
	}

      }
    }
  }
  for(int i=0; i<macro.mem->num_bank; i++){
    for(size_t c=0; c<new_num_port[i].size(); c++){
      if(new_num_port[i][c] > macro.mem->membanks[i].num_port){
	cout << "new num port of bank " << i << " at cycle " << c << " is " << new_num_port[i][c] << endl; 
	throw exception();
      }
    }
  }
  return true;		

}


void TileScheduling::AllocateTile(int cycle, int tile){
  //place tile at cycle
  macro.tiles_sche[tile].start = cycle;
  macro.tiles_sche[tile].end = cycle+macro.tiles[tile].latency-1;

  ComputeBlock* cb = macro.tiles[tile].cb;
  //UPDATE: cb usage
  num_cb[cb->name][cycle]++;

  //UPDATE: liveout
  for(vector<int>::iterator out_op=macro.tiles[tile].liveout_ops.begin(); out_op!=macro.tiles[tile].liveout_ops.end(); out_op++){
    Liverange* lr = tileout_lrs[*out_op];
    lr->AddLRtoMem(&macro.mem->num_port, &macro.mem->num_live, num_port_left);

    for(int c=lr->write_cycle+1; c<=lr->last_cycle; c++){
      macro.mem->op_in_bank[*out_op][c][0] = lr->mem_bank;
      macro.mem->op_in_bank[*out_op][c][1] = lr->mem_loc;
    }

    macro.op_in_cycle[*out_op] = lr->last_cycle;
    if(macro.op_in_cycle[*out_op] > last_cycle){
      last_cycle = macro.op_in_cycle[*out_op];
    }
  }
  if(macro.tiles_sche[tile].end > last_cycle){
    last_cycle = macro.tiles_sche[tile].end;
  }

  //UDPATE: livein
  for(map<int,int>::iterator in_op=macro.tiles[tile].livein_ops.begin(); in_op!=macro.tiles[tile].livein_ops.end(); in_op++){
    //need to read
    int read_cycle = cycle+in_op->second;

    //remove old lr
    if(macro.op_in_cycle.find(in_op->first) != macro.op_in_cycle.end()){
      Liverange lr(this);
      if(read_cycle > macro.op_in_cycle[in_op->first]){
	lr.GetLiverange(in_op->first, macro.op_in_cycle[in_op->first]);
      }
      else{
	lr.GetLiverange(in_op->first, read_cycle);
      }
      lr.RemoveLRfromMem(&macro.mem->num_port, &macro.mem->num_live, num_port_left);
      for(int c=lr.write_cycle+1; c<=lr.last_cycle; c++){
	macro.mem->op_in_bank[in_op->first][c][0] = -1;
	macro.mem->op_in_bank[in_op->first][c][1] = -1;
      }
    }
    else{
      macro.inop_firstread[in_op->first] = read_cycle;
    }

    //add new lr
    Liverange* lr_new = tilein_lrs[in_op->first];

    lr_new->AddLRtoMem(&macro.mem->num_port, &macro.mem->num_live, num_port_left);
    for(int c=lr_new->write_cycle+1; c<=lr_new->last_cycle; c++){
      macro.mem->op_in_bank[in_op->first][c][0] = lr_new->mem_bank;
      macro.mem->op_in_bank[in_op->first][c][1] = lr_new->mem_loc;
    }

    if(spill_lrs.find(in_op->first) != spill_lrs.end()){
      //add lr after spilling
      Liverange* lr_spill = spill_lrs[in_op->first];
      lr_spill->AddLRtoMem(&macro.mem->num_port, &macro.mem->num_live, num_port_left);
      for(int c=lr_spill->write_cycle+1; c<=lr_spill->last_cycle; c++){
	macro.mem->op_in_bank[in_op->first][c][0] = lr_spill->mem_bank;
	macro.mem->op_in_bank[in_op->first][c][1] = lr_spill->mem_loc;
      }

      spill_sche ps;
      ps.cond = 1;
      ps.start = lr_new->last_cycle;
      ps.end = lr_spill->write_cycle;
      ps.start_membank = lr_new->mem_bank;
      ps.end_membank = lr_spill->mem_bank;
      macro.spills_sche.push_back(ps);

    }
    else if(bankex_lrs.find(in_op->first) != bankex_lrs.end()){
      //add lr after bankex
      Liverange* lr_bankex = bankex_lrs[in_op->first];
      lr_bankex->AddLRtoMem(&macro.mem->num_port, &macro.mem->num_live, num_port_left);
      for(int c=lr_bankex->write_cycle+1; c<=lr_bankex->last_cycle; c++){
	macro.mem->op_in_bank[in_op->first][c][0] = lr_bankex->mem_bank;
	macro.mem->op_in_bank[in_op->first][c][1] = lr_bankex->mem_loc;
      }

      bankex_sche bs;
      bs.op = in_op->first;
      bs.start = lr_new->last_cycle;
      bs.end = lr_bankex->write_cycle;
      bs.start_membank = lr_new->mem_bank;
      bs.end_membank = lr_bankex->mem_bank;
      macro.bankexs_sche.push_back(bs);
    }

    if(macro.op_in_cycle.find(in_op->first) == macro.op_in_cycle.end()){
      macro.op_in_cycle[in_op->first] = read_cycle;
    }
    else{
      if(read_cycle > macro.op_in_cycle[in_op->first]){
	macro.op_in_cycle[in_op->first] = read_cycle;
      }
    }
  }
}

bool TileScheduling::HasReadWrite(int cycle, int tile){
  int write_cycle = cycle+macro.tiles[tile].latency-1;
  if(num_port_left[write_cycle] < int(macro.tiles[tile].liveout_ops.size())){
    return false;
  }

  map<int, int> num_read_left_c;
  for(map<int,int>::iterator in_op=macro.tiles[tile].livein_ops.begin(); in_op!=macro.tiles[tile].livein_ops.end(); in_op++){
    int read_cycle = cycle+in_op->second;
    if((macro.op_in_cycle.find(in_op->first) != macro.op_in_cycle.end()) && (read_cycle != macro.op_in_cycle[in_op->first])){
      //need use one read
      if(num_read_left_c.find(read_cycle) == num_read_left_c.end()){
	num_read_left_c[read_cycle] = num_port_left[read_cycle]-1;
      }
      else{
	num_read_left_c[read_cycle]--;
      }
      if(num_read_left_c[read_cycle] < 0){
	return false;
      }
    }
  }
  //for(int c=0; c<num_livein.size(); c++){
  //   int read_cycle = cycle+c;
  //   if(num_read_left[read_cycle] < num_livein[c]){
  //   //if(num_read_left[read_cycle] == 0){
  //      return false;
  //   }
  //}
  return true;
}

void TileScheduling::ScheduleTile(int tile){
  cout << "******tile " << tile << endl;
  //cycle to check initially
  int cycle = 0;
  for(vector<int>::iterator pred_tile=macro.tiles[tile].pred_tiles.begin(); pred_tile!=macro.tiles[tile].pred_tiles.end(); pred_tile++){
    if(macro.tiles_sche[*pred_tile].end > cycle){
      cycle = macro.tiles_sche[*pred_tile].end;
    }
  }

  int cycle_count=0;
  bool valid_cycle = false;
  while(!valid_cycle)
  {
    cycle++;

    if(!spillidx.empty()){
      spillidx.clear();
    }

    if(!tilein_lrs.empty()){
      for(map<int,Liverange*>::iterator i=tilein_lrs.begin(); i!=tilein_lrs.end(); i++){
	delete i->second;
      }
      tilein_lrs.clear();
    }

    if(!tileout_lrs.empty()){
      for(map<int,Liverange*>::iterator i=tileout_lrs.begin(); i!=tileout_lrs.end(); i++){
        delete i->second;
      }
      tileout_lrs.clear();
    }

    if(!original_inop_lrs.empty()){
      for(map<int,Liverange*>::iterator i=original_inop_lrs.begin(); i!=original_inop_lrs.end(); i++){
	delete i->second;
      }
      original_inop_lrs.clear();
    }

    if(!updated_inop_lrs.empty()){
      for(map<int,Liverange*>::iterator i=updated_inop_lrs.begin(); i!=updated_inop_lrs.end(); i++){
	delete i->second;
      }
      updated_inop_lrs.clear();
    }

    if(!spill_lrs.empty()){
      for(map<int,Liverange*>::iterator i=spill_lrs.begin(); i!=spill_lrs.end(); i++){
        delete i->second;
      }
      spill_lrs.clear();
    }

    if(!bankex_lrs.empty()){
      for(map<int,Liverange*>::iterator i=bankex_lrs.begin(); i!=bankex_lrs.end(); i++){
        delete i->second;
      }
      bankex_lrs.clear();
    }


    max_cycle = cycle+macro.tiles[tile].latency; //size of data structure
    //expand data structure---------------------------------------
    ExtendMemData(max_cycle);
    ExtendNumCB(max_cycle);
    ExtendOpinBank(max_cycle);
    //-------------------------------------------------------------

    new_num_port = macro.mem->num_port;
    new_num_live = macro.mem->num_live;

    valid_cycle = CheckCycle_MN(cycle, tile);
    cycle_count++;
  }
  cycle_delta.push_back(cycle_count);

  //--------------------------------------------------------------------------------------
  AllocateTile(cycle, tile);

  for(int i=0; i<macro.mem->num_bank; i++){
    for(size_t c=0; c<macro.mem->num_port[i].size(); c++){
      if(macro.mem->num_port[i][c] > macro.mem->membanks[i].num_port){
	cout << "num_port of bank " << i << " at cycle " << c << " is " << macro.mem->num_port[i][c] << endl;
	throw exception();
      }
    }
  }
}

TilePriority::TilePriority(int in_tile, TileScheduling* in_sche): tile(in_tile), sche(in_sche){
  //if(sche->tiles[tile].pred_tiles.size() == 0){
  priority = tile;
  //}
  //else{
  //   priority = -2000;
  //   int last_pred = 0;
  //   for(vector<int>::iterator pred_t=sche->tiles[tile].pred_tiles.begin(); pred_t!=sche->tiles[tile].pred_tiles.end(); pred_t++){
  //      if(sche->tiles_sche[*pred_t].end > last_pred){
  //         last_pred = sche->tiles_sche[*pred_t].end;
  //      }
  //   }
  //   priority += last_pred; 
  //}
}

bool operator<(const TilePriority& tp1, const TilePriority& tp2){
  if(tp1.priority > tp2.priority){
    return true;
  }
  return false;
}

void TileScheduling::Scheduling(){

  //iteratively allocate all tiles
  //for(vector<tile_sche>::iterator i=tiles_sche.begin(); i!=tiles_sche.end(); i++){
  //   int tile = i-tiles_sche.begin();
  //   cout << "tile " << tile << endl;
  //   ScheduleTile(tile);
  //}

  priority_queue<TilePriority> pq_tile;
  //intialize priority queue;
  for(vector<Tile>::iterator t=macro.tiles.begin(); t!=macro.tiles.end(); t++){
    int tile = t-macro.tiles.begin();
    if(t->pred_tiles.size() == 0){
      pq_tile.push(TilePriority(tile, this));
    }
  }

  //while(!pq_tile.empty()){
  //   cout << pq_tile.top().tile << endl;
  //   pq_tile.pop();
  //}

  while(!pq_tile.empty()){
    TilePriority tp = pq_tile.top();
    pq_tile.pop();
    int tile = tp.tile;

    ScheduleTile(tile);

    //update pq_tile
    //only need to check post-tile of the tile just scheduled
    set<int> post_t;
    for(map<int,vector<int>>::iterator i=macro.tiles[tile].post_tiles.begin(); i!=macro.tiles[tile].post_tiles.end(); i++){
      for(vector<int>::iterator t=i->second.begin(); t!=i->second.end(); t++){
	if(post_t.find(*t) == post_t.end()){
	  post_t.insert(*t);
	}
      }
    }

    for(set<int>::iterator t=post_t.begin(); t!=post_t.end(); t++){
      bool can_schedule = true;
      for(vector<int>::iterator pred_t=macro.tiles[*t].pred_tiles.begin(); pred_t!=macro.tiles[*t].pred_tiles.end(); pred_t++){
	if(macro.tiles_sche[*pred_t].start == 0){
	  can_schedule = false;
	  break;
	}
      }

      if(can_schedule){
	pq_tile.push(TilePriority(*t, this));
      }
    }
  }


  for(auto i=macro.mem->op_in_bank.begin(); i!=macro.mem->op_in_bank.end(); i++){
    cout << endl << "op " << i->first << endl;
    for(auto j=i->second.begin(); j!=i->second.end(); j++){
      cout << j->at(0) << "," << j->at(1) << endl;
    }
  }
}




bool TileScheduling::MoveBarrierRead(int tile, int op, int orig_read, int tile_sche_idx){
  //search if there is empty read

  //check bankex
  //old bank
  int old_bank = macro.tiles_sche.rbegin()->inop[op].addr[0];

  //find new bank
  int new_bank;
  int new_barrier;

  cout << endl << "in move barrier" << endl;
  for(int i=0; i<macro.mem->num_bank; i++){
    cout << "new bank " << i << endl;
    if(i!=old_bank){
      //does this bank has better cycle to read
      int cycle_produce = 0;
      if(macro.ops[op].tile != -1){
        int dep_tile_sche_idx = macro.tiles[macro.ops[op].tile].tile_sche_idx;
	cycle_produce = macro.tiles_sche[dep_tile_sche_idx].outop[op].cycle+1;
      }

      int last_op_leave;
      if(tile_sche_idx < 0){
        last_op_leave = 0;
      }
      else{
        last_op_leave = macro.tiles_sche[tile_sche_idx].start+macro.tiles[tile].livein_ops[op]+1;
      }
      
      bool can_read = false;
      int start_read_cycle = max(cycle_produce, last_op_leave);
      for(int c=start_read_cycle; c<orig_read; c++){
        if(new_num_port[i][c]+1 < macro.mem->membanks[i].num_port){
	  new_barrier = c;
	  can_read = true;
	  break;
	}
      }

      if(!can_read){
	cout << "can't read" << endl;
        continue;
      }

      cout << "new barrier " << new_barrier << endl; 
      //can allocate bankex
      bool can_allocate_bankex = false;
      int bankex_start_earliest = cycle_produce;
      int bankex_end_earliest = cycle_produce+BANKEX_LATENCY-1;
      //c is bankex end cycle
      for(int c=new_barrier-1; c >= bankex_end_earliest; c--){
        int bankex_start = c-BANKEX_LATENCY+1;
	int bankex_end = c;
	cout << "bankex start " << bankex_start << endl;
	if(new_num_port[i][bankex_end]+1 < macro.mem->membanks[i].num_port){
	  cout << "have write port " << endl;
	  if((op_read_cycles[op].find(bankex_start) != op_read_cycles[op].end())
	      || (new_num_port[old_bank][bankex_start]+1 < macro.mem->membanks[old_bank].num_port)){
	//if((new_num_port[old_bank][bankex_start]+1 < macro.mem->membanks[old_bank].num_port)
	//  && (new_num_port[i][bankex_end]+1 < macro.mem->membanks[i].num_port)){
	  //can allocate bankex
	  //check mem size
	    cout << "has port " << endl;
	    bool has_loc = false;
	    for(int loc=0; loc<new_num_live[i].size(); loc++){
	      bool valid_loc = true;
	      for(int cc=bankex_end+1; cc<=new_barrier; cc++){
		if(new_num_live[i][loc][cc] >= 1){
		  valid_loc = false;
		  break;
		}
	      }
	      if(valid_loc){
		has_loc = true;
		cout << "has loc " << loc << endl;
		break;
	      }
	    }
	    if(has_loc){
	      can_allocate_bankex = true;
	    }
	  }
	}
      }

      if(can_allocate_bankex){
        //find mem bank
	new_bank = i;
	cout << "find new bank " << new_bank << " with new barrier " << new_barrier << endl;
	return true;
      }
    }
  }
  return false;
}







bool livein_comp(const pair<int,int>& a, const pair<int,int>& b){
  return (a.second < b.second);
}

//udpate cycle to schedule tile
int TileScheduling::FindCycle_dblks(int tile, int tile_sche_idx, map<int,int> &inop_read_cycle){
  new_num_port = macro.mem->num_port;
  new_num_live = macro.mem->num_live;

  //sort livein op based on its read cycle in tile
  vector<pair<int,int>> sorted_livein_ops(macro.tiles[tile].livein_ops.begin(), macro.tiles[tile].livein_ops.end());
  sort(sorted_livein_ops.begin(), sorted_livein_ops.end(), livein_comp);

  for(auto op=sorted_livein_ops.begin(); op!=sorted_livein_ops.end(); op++){
    int cycle_produce = 0;
    if(macro.ops[op->first].tile != -1){
      //op generated by previous tile
      int dep_tile_sche_idx = macro.tiles[macro.ops[op->first].tile].tile_sche_idx;
      cycle_produce = macro.tiles_sche[dep_tile_sche_idx].outop[op->first].cycle+1;
    }

    //set tile_sche bank
    int bank = macro.tiles_sche.rbegin()->inop[op->first].addr[0];
    //cout << "bank " << bank << endl;

    //know when the op is produced in mem
    //determine when it can start read
    int last_op_leave;
    if(tile_sche_idx < 0){
      last_op_leave = 0;
    }
    else{
      last_op_leave = macro.tiles_sche[tile_sche_idx].start+op->second+1;
    }


    int can_start_read = max(cycle_produce, last_op_leave);
    int read_cycle = can_start_read;
    ExtendMemData(read_cycle+1, new_num_port, new_num_live);
    while(new_num_port[bank][read_cycle]+1 > macro.mem->membanks[bank].num_port){
      read_cycle++;
      ExtendMemData(read_cycle+1, new_num_port, new_num_live);
    }
    new_num_port[bank][read_cycle]++;
    inop_read_cycle[op->first] = read_cycle;
  }

  int cycle = 0;
  int barrier_op;
  //update cycle based on livein op
  for(auto op=macro.tiles[tile].livein_ops.begin(); op!=macro.tiles[tile].livein_ops.end(); op++){
    if(cycle+op->second < inop_read_cycle[op->first]){
      cycle = inop_read_cycle[op->first]-op->second;
      barrier_op = op->first;
    }
  }

  //if(cycle != 0){
  //bool can_move_barrier = true;
  //while(can_move_barrier){
    //can_move_barrier = MoveBarrierRead(tile, barrier_op, cycle, tile_sche_idx);
    //if(can_move_barrier){
      //generate new barrier
      //update cycle to new barrier
    //}
 // }
  //}
     
  //update cycle based on liveout op
  for(auto op=macro.tiles[tile].liveout_ops.begin(); op!=macro.tiles[tile].liveout_ops.end(); op++){
    //current produce cycle: cycle+macro.tiles[tile].latency-1;
    //last leave: 
    int last_op_leave;
    if(tile_sche_idx < 0){
      last_op_leave = 0;
    }
    else{
      int last_tile_idx = macro.tiles_sche[tile_sche_idx].tile_idx;
      int last_op = macro.tiles[last_tile_idx].liveout_ops[op-macro.tiles[tile].liveout_ops.begin()];
      last_op_leave = macro.tiles_sche[tile_sche_idx].outop[last_op].cycle+1;
    }
    if(cycle+macro.tiles[tile].latency-1 < last_op_leave){
      cycle = last_op_leave-macro.tiles[tile].latency+1;
    }
  }
  return cycle;
}

void TileScheduling::FindCB_dblks(int tile){
  //set tile_sche addr
  for(auto op=macro.tiles[tile].livein_ops.begin(); op!=macro.tiles[tile].livein_ops.end(); op++){
    if(macro.ops[op->first].tile != -1){
      int dep_tile_sche_idx = macro.tiles[macro.ops[op->first].tile].tile_sche_idx;
      macro.tiles_sche.rbegin()->inop[op->first].addr = macro.tiles_sche[dep_tile_sche_idx].outop[op->first].addr;
    }
    else{
      macro.tiles_sche.rbegin()->inop[op->first].addr = macro.ioop_addr[op->first];
    }
  }

  //try used cb;
  int cycle_used_cb = INT_MAX;
  map<int,int> read_cycle_used_cb;
  int cb_idx_used;

  if(cb_tile_sche.size() > 0){
    cb_idx_used = 0;
    int cb_cycle = macro.tiles_sche[cb_tile_sche[0]].start;

    for(int i=0; i<cb_tile_sche.size(); i++){
      if(macro.tiles_sche[cb_tile_sche[i]].start < cb_cycle){
        cb_idx_used = i;
      }
    }
    //find cb_idx
    int last_tile_sche = cb_tile_sche[cb_idx_used];
    cycle_used_cb = FindCycle_dblks(tile, last_tile_sche, read_cycle_used_cb);
  }

  //if has new cb, try schedule use new cb
  int cycle_new_cb = INT_MAX;
  map<int, int> read_cycle_new_cb;
  int cb_idx_new;

  if(cb_tile_sche.size() < ComputeBlockLib::num_cb["mul_acc"]){
    cb_idx_new = cb_tile_sche.size();
    cycle_new_cb = FindCycle_dblks(tile, -1, read_cycle_new_cb);
  }

  //int cycle;
  if(cycle_used_cb <= cycle_new_cb){
    cb_tile_sche[cb_idx_used] = macro.tiles_sche.size()-1;
    AllocateTile_dblks(tile, cycle_used_cb, cb_idx_used, read_cycle_used_cb);
  }
  else{
    cb_tile_sche.push_back(macro.tiles_sche.size()-1);
    AllocateTile_dblks(tile, cycle_new_cb, cb_idx_new, read_cycle_new_cb);
  }

}

void TileScheduling::AllocateTile_dblks(int tile, int cycle, int cb_idx, map<int,int>& read_cycle){
  macro.tiles_sche.rbegin()->start = cycle;
  macro.tiles_sche.rbegin()->end = cycle+macro.tiles[tile].latency-1;
  macro.tiles_sche.rbegin()->cb_idx = cb_idx;

  for(auto op=read_cycle.begin(); op!=read_cycle.end(); op++){
    macro.tiles_sche.rbegin()->inop[op->first].cycle = op->second;
    int bank = macro.tiles_sche.rbegin()->inop[op->first].addr[0];
    macro.mem->ExtendLength(op->second+1);
    macro.mem->num_port[bank][op->second]++;

    if(op_read_cycles[op->first].find(op->second) == op_read_cycles[op->first].end()){
      op_read_cycles[op->first].insert(op->second);	    
    }
  }

  //allocate liveout
  for(auto op=macro.tiles[tile].liveout_ops.begin(); op!=macro.tiles[tile].liveout_ops.end(); op++){
    //liveout op: *op
    //find the corresponding cout addr for liveout op
    if(macro.ioop_addr.find(*op) != macro.ioop_addr.end()){
      macro.tiles_sche.rbegin()->outop[*op].addr = macro.ioop_addr[*op];
    }
    else{
      int num_cout = macro.Cout.size() * macro.Cout[0].size();
      int c_idx = tile%num_cout;
      int cout_op = macro.Cout[c_idx/macro.Cout[0].size()][c_idx%macro.Cout[0].size()];
      macro.tiles_sche.rbegin()->outop[*op].addr = macro.ioop_addr[cout_op];
    }

    int bank = macro.tiles_sche.rbegin()->outop[*op].addr[0];
    int cycle = macro.tiles_sche.rbegin()->end;
    macro.mem->ExtendLength(cycle+1);
    while(macro.mem->num_port[bank][cycle]+1 > macro.mem->membanks[bank].num_port){
      cycle++;
      macro.mem->ExtendLength(cycle+1);
    }
    macro.mem->num_port[bank][cycle]++;
    macro.tiles_sche[tile].outop[*op].cycle = cycle;
  }
}



void TileScheduling::Scheduling_subblk(int subblk_i, int subblk_j, int subblk_l){
  //allocate all tiles in subblk
  vector<int> tiles;
  int end_i = min((subblk_i+1)*subblk_dimi_tile, macro.tile_m);
  int end_j = min((subblk_j+1)*subblk_dimj_tile, macro.tile_n);
  int end_l = min((subblk_l+1)*subblk_diml_tile, macro.tile_k);

  vector<map<int,set<int>>> sorted_livein_ops;
  //key: time to read, value: in op

  int cb_count = 0;
  for(int i=(subblk_i)*subblk_dimi_tile; i<end_i; i++){
    for(int j=(subblk_j)*subblk_dimj_tile; j<end_j; j++){
      for(int l=(subblk_l)*subblk_diml_tile; l<end_l; l++){
        //tile idx
	int tile = i*(macro.tile_n*macro.tile_k)+j*macro.tile_k+l;
	tiles.push_back(tile);

	//assign cb idx to tile
	tile_sche ts;
	ts.tile_idx = tile;
	ts.cb_idx = cb_count;
	ts.start = 0;
	ts.end = ts.start+macro.tiles[tile].latency-1;
	cout << endl <<  "tile sche " << macro.tiles_sche.size() << endl;

        for(auto op=macro.tiles[tile].liveout_ops.begin(); op!=macro.tiles[tile].liveout_ops.end(); op++){
          //liveout op: *op
          //find the corresponding cout addr for liveout op
	  cout << "out op " << *op << ":";
          if(macro.ioop_addr.find(*op) != macro.ioop_addr.end()){
            ts.outop[*op].addr = macro.ioop_addr[*op];
          }
          else{
      	    int num_cout = macro.Cout.size() * macro.Cout[0].size();
            int c_idx = tile%num_cout;
            int cout_op = macro.Cout[c_idx/macro.Cout[0].size()][c_idx%macro.Cout[0].size()];
            ts.outop[*op].addr = macro.ioop_addr[cout_op];
          }
	  cout << ts.outop[*op].addr[0] << endl;
	}

	for(auto op=macro.tiles[tile].livein_ops.begin(); op!=macro.tiles[tile].livein_ops.end(); op++){
	  cout << "in op " << op->first << ":";
	  if(macro.ops[op->first].tile != -1){
	    int dep_tile_sche_idx = macro.tiles[macro.ops[op->first].tile].tile_sche_idx;
	    ts.inop[op->first].addr = macro.tiles_sche[dep_tile_sche_idx].outop[op->first].addr;
	  }
	  else{
	    ts.inop[op->first].addr = macro.ioop_addr[op->first];
	  }
	  cout << ts.inop[op->first].addr[0] << endl;
	}

	cb_count++;
	macro.tiles[tile].tile_sche_idx = macro.tiles_sche.size();
	macro.tiles_sche.push_back(ts);
	cout << "tile " << tile << endl;

	map<int, set<int>> sorted_livein_ops_t;
	for(auto &i: macro.tiles[tile].livein_ops){
	  if(sorted_livein_ops_t.find(i.second) == sorted_livein_ops_t.end()){
	    set<int> s = {i.first};
	    sorted_livein_ops_t[i.second] = s;
	  }
	  else{
	    if(sorted_livein_ops_t[i.second].find(i.first) == sorted_livein_ops_t[i.second].end()){
	      sorted_livein_ops_t[i.second].insert(i.first);
	    }
	  }
	}
	sorted_livein_ops.push_back(sorted_livein_ops_t);
      }
    }
  }

  cout << "sorted livein ops" << endl;
  for(auto i=sorted_livein_ops.begin(); i!=sorted_livein_ops.end(); i++){
    cout << endl << "tile " << tiles[i-sorted_livein_ops.begin()] <<": " << endl;
    for(auto &j: *i){
      cout << j.first << ":";
      for(auto &k: j.second){
        cout << k << ",";
      }
      cout << endl;
    }
  }

  //read out all livein ops
  int num_batch = sorted_livein_ops[0].size();
  for(int i=0; i<num_batch; i++){
    cout << endl << "batch " << i << endl;
    for(auto t=sorted_livein_ops.begin(); t!=sorted_livein_ops.end(); t++){
      int tile = tiles[t-sorted_livein_ops.begin()];
      for(auto op=t->at(i).begin(); op!=t->at(i).end(); op++){
        cout << "op " << *op << ":";

	int cycle_produce = 0;
	if(macro.ops[*op].tile != -1){
          //op generated by previous tile
          int dep_tile_sche_idx = macro.tiles[macro.ops[*op].tile].tile_sche_idx;
          cycle_produce = macro.tiles_sche[dep_tile_sche_idx].outop[*op].cycle+1;
        }

	int tile_sche = macro.tiles[tile].tile_sche_idx;
	int cb_idx = macro.tiles_sche[tile_sche].cb_idx;

        int last_op_leave = 0;
        if(cb_tile_sche[cb_idx] < 0){
          last_op_leave = 0;
        }
        else{
          last_op_leave = macro.tiles_sche[cb_tile_sche[cb_idx]].start+i+1;
        }
	
        int bank = macro.tiles_sche[tile_sche].inop[*op].addr[0];
	cout << bank << endl;
	
	int can_start_read = max(cycle_produce, last_op_leave);

	int read_cycle = can_start_read;
        macro.mem->ExtendLength(read_cycle+1);
	//cout << "num of port " << endl;
	//for(int c=0; c<macro.mem->num_port[bank].size(); c++){
	  //cout << macro.mem->num_port[bank][c] << endl;
	//}

	bool has_read = (op_read_cycles[*op].find(read_cycle) != op_read_cycles[*op].end());
        while((!has_read)&&(!macro.mem->CanPortInc(bank, read_cycle))){
          read_cycle++;
  	  has_read = (op_read_cycles[*op].find(read_cycle) != op_read_cycles[*op].end());
          macro.mem->ExtendLength(read_cycle+1);
	  //cout << "num of port " << endl;
	  //for(int c=0; c<macro.mem->num_port[bank].size(); c++){
	   // cout << macro.mem->num_port[bank][c] << endl;
	  //}
        }
	cout << "read at cycle " << read_cycle << endl;
	int tile_sche_idx = macro.tiles[tile].tile_sche_idx;
	macro.tiles_sche[tile_sche_idx].inop[*op].cycle = read_cycle;

	if(!has_read){
          macro.mem->PortInc(bank, read_cycle);
	}
	op_read_cycles[*op].insert(read_cycle);
      }
    }
  }

}


CBEvents::CBEvents(int in_num_stage){
  num_stage = in_num_stage;

  for(int i=0; i<num_stage; i++){
    StageEvents st;
    stage_events.push_back(st);
  }
}


void CBEvents::setStages(int in_num_stage){
  num_stage = in_num_stage;

  for(int i=0; i<num_stage; i++){
    StageEvents st;
    stage_events.push_back(st);
  }
}

void StageEvents::Enqueue(set<int> &ops){
  q_ops.push(ops);
}

void StageEvents::Dequeue(){
  ops = q_ops.front();
  q_ops.pop();
}


int TileScheduling::Executing_cbs(){
  bool finished = false;
  int cycle = -1;
  while(!finished){
    cycle++;
    cout << endl << "cycle " << cycle << endl;

    vector<bool> finish_stamp(cb_events.size(), false);
    for(int i=0; i<cb_events.size(); i++){
      if(i == 0){
        cout << endl << "cb " << i << endl;
      }
      for(int j=cb_events[i].num_stage-1; j>=0; j--){
        //take new event
	if(i == 0){
	  cout << "stage " << j << endl;
	}
	cb_events[i].stage_events[j].stall = (!cb_events[i].stage_events[j].ops.empty());
	if((j<(cb_events[i].num_stage-1)) && cb_events[i].stage_events[j+1].stall){
	  cb_events[i].stage_events[j].stall = true;
	}
	if(!cb_events[i].stage_events[j].stall){
	  if(j==0){
	    cb_events[i].stage_events[j].produced_out = false;
	    if(!cb_events[i].stage_events[j].q_ops.empty()){
	      cb_events[i].stage_events[j].Dequeue();
	    }
	  }
	  else{ 
	    cb_events[i].stage_events[j].produced_out = cb_events[i].stage_events[j-1].produced_out;
	    if(cb_events[i].stage_events[j-1].produced_out){
	      if(!cb_events[i].stage_events[j].q_ops.empty()){
		cb_events[i].stage_events[j].Dequeue();
		cb_events[i].stage_events[j].produced_out = false;
	      }
	    }
	  }
	}

	//read / write ops
	vector<int> finished_ops;
	for(auto op=cb_events[i].stage_events[j].ops.begin(); op!=cb_events[i].stage_events[j].ops.end(); op++){
	  if(i == 0){
	    cout << "op " << *op << endl;
	  }
	  int bank = macro.ioop_addr[*op][0];
	  macro.mem->ExtendLength(cycle+1);
	  if(op_read_cycles[*op].find(cycle) == op_read_cycles[*op].end()){
	    if(macro.mem->CanPortInc(bank, cycle)){
	      //update op read cycles
	      if(j != (cb_events[i].num_stage-1)){
		op_read_cycles[*op].insert(cycle);
	      }
	      macro.mem->PortInc(bank, cycle);
	      finished_ops.push_back(*op);
	    }
	  }else{
	    finished_ops.push_back(*op);
	  }
	}
	for(auto &op: finished_ops){
	  cb_events[i].stage_events[j].ops.erase(op);
	}

	if(i == 0){
	  cout << "op left" << endl;
	  for(auto op=cb_events[i].stage_events[j].ops.begin(); op!=cb_events[i].stage_events[j].ops.end(); op++){
	    cout << "op " << *op << endl;
	  }
	}

	if(!finished_ops.empty() && cb_events[i].stage_events[j].ops.empty()){
	  cb_events[i].stage_events[j].produced_out = true;
	}

	if(j == (cb_events[i].num_stage-1)){
	  if(cb_events[i].stage_events[j].ops.empty() && cb_events[i].stage_events[j].q_ops.empty()){
	    finish_stamp[i] = true;
	  }
	}
      }
    }

    finished = true;
    for(int i=0; i<cb_events.size(); i++){
      finished = finished && finish_stamp[i];
    }
  }
  return cycle;
}


void TileScheduling::getBlkDimij(){
  int dimij = sqrt(num_cbs);
 
  subblk_dimi_tile = min(dimij, macro.tile_m);
  subblk_dimj_tile = min(num_cbs/subblk_dimi_tile, macro.tile_n);
}


void TileScheduling::Scheduling_pipe(){
  //initialize op_read_cycles
  for(auto t=macro.tiles.begin(); t!=macro.tiles.end(); t++){
    for(auto op=t->livein_ops.begin(); op!=t->livein_ops.end(); op++){
      set<int> t;
      op_read_cycles[op->first] = t;
    }
  }

  //determine block size
  //num_cbs = ComputeBlockLib::num_cb["mul_acc"];
  //subblk_diml_tile = 1;
  //getBlkDimij();
  //cout << "macro m " << macro.m << endl;
  //cout << "macro n " << macro.n << endl;
  //cout << "subblk_dimi_tile " << subblk_dimi_tile << endl;
  //cout << "subblk_dimj_tile " << subblk_dimj_tile << endl;

  //int num_cbs_used = subblk_dimi_tile*subblk_dimj_tile;
  for(int i=0; i<opti_para.num_cb; i++){
    CBEvents ce(ComputeBlockLib::cbs["mul_acc"]->latency);
    cb_events.push_back(ce);
  }

  //subblk_m = (macro.tile_m%subblk_dimi_tile==0)?macro.tile_m/subblk_dimi_tile: macro.tile_m/subblk_dimi_tile+1;
  //subblk_n = (macro.tile_n%subblk_dimj_tile==0)?macro.tile_n/subblk_dimj_tile: macro.tile_n/subblk_dimj_tile+1;
  //subblk_k = macro.tile_k/subblk_diml_tile;

  subblk_m = opti_para.blk_dimi/opti_para.subblk_dimi;
  subblk_n = opti_para.blk_dimj/opti_para.subblk_dimj;
  subblk_k = opti_para.blk_diml/opti_para.subblk_diml;

  for(auto t=macro.tiles.begin(); t!=macro.tiles.end(); t++){
    int tile = t-macro.tiles.begin();
    for(auto op=macro.tiles[tile].liveout_ops.begin(); op!=macro.tiles[tile].liveout_ops.end(); op++){
      if(macro.ioop_addr.find(*op) == macro.ioop_addr.end()){
	int num_cout = macro.Cout.size() * macro.Cout[0].size();
	int c_idx = tile%num_cout;
	int cout_op = macro.Cout[c_idx/macro.Cout[0].size()][c_idx%macro.Cout[0].size()];
	macro.ioop_addr[*op] = macro.ioop_addr[cout_op];
      }
    }
  }

  for(int subblk_l=0; subblk_l<subblk_k; subblk_l++){
    for(int subblk_i=0; subblk_i<subblk_m; subblk_i++){
      for(int subblk_j=0; subblk_j<subblk_n; subblk_j++){
	cout << endl;
	cout << "subblk i " << subblk_i << endl;
	cout << "subblk j " << subblk_j << endl;
	cout << "subblk l " << subblk_l << endl;

        //read out all input data
	//allocate all cbs
	int cb_idx = 0;
	for(int i=(subblk_i)*opti_para.subblk_dimi; i<(subblk_i+1)*opti_para.subblk_dimi; i++){
	  for(int j=(subblk_j)*opti_para.subblk_dimj; j<(subblk_j+1)*opti_para.subblk_dimj; j++){
	      //tile idx
	      int tile = i*(subblk_k*opti_para.blk_dimj)+j*subblk_k+subblk_l;
	      cout << "tile " << tile << endl;

	      //cb to use cb_idx
	      map<int, set<int>> sorted_livein_ops;
	      for(auto &i: macro.tiles[tile].livein_ops){
	        if(sorted_livein_ops.find(i.second) == sorted_livein_ops.end()){
	          set<int> s = {i.first};
	          sorted_livein_ops[i.second] = s;
	        }
	        else{
	          if(sorted_livein_ops[i.second].find(i.first) == sorted_livein_ops[i.second].end()){
	            sorted_livein_ops[i.second].insert(i.first);
	          }
	        }
	      }
	      
	      for(auto &livein: sorted_livein_ops){
		cout << livein.first << ":";
		for(auto &i: livein.second){
		  cout << i << ",";
		}
		cout << endl;
	        cb_events[cb_idx].stage_events[livein.first].Enqueue(livein.second);
	      }
		
	      set<int> liveout_ops;
	      for(auto &op: macro.tiles[tile].liveout_ops){
	        liveout_ops.insert(op);
	      }
	      cb_events[cb_idx].stage_events.rbegin()->Enqueue(liveout_ops);

	      cb_idx++;
	  }
	}
      }
    }
  }

  //executing cbs
  last_cycle = Executing_cbs();
  cout << "Last Cycle of mn template scheduling " << last_cycle << endl;

}
















void TileScheduling::GetNumCBMax(){
  for(int c=0; c<max_cycle; c++){
    for(map<string, int>::iterator i=num_cb_max.begin(); i!=num_cb_max.end(); i++){
      if(num_cb[i->first][c] > num_cb_max[i->first])
	num_cb_max[i->first] = num_cb[i->first][c];
    }
  }
}

void TileScheduling::PrintNumCBMax(){
  cout << "max #of cb used: " << endl;
  for(map<string, int>::iterator i=num_cb_max.begin(); i!=num_cb_max.end(); i++){
    cout << "cb " << i->first << " used " << i->second << endl;
  }
}

void TileScheduling::PrintNumPort(int mem_bank){
  cout << "# of ports of mem bank " << mem_bank << endl;
  for(vector<int>::iterator i=macro.mem->num_port[mem_bank].begin(); i!=macro.mem->num_port[mem_bank].end(); i++){
    cout << *i << endl;
  }
}

bool TileScheduling::PassTileDependency(){
  bool pass = true;
  for(vector<tile_sche>::iterator i=macro.tiles_sche.begin(); i!=macro.tiles_sche.end(); i++){
    int tile = i-macro.tiles_sche.begin();
    for(vector<int>::iterator j=macro.tiles[tile].pred_tiles.begin(); j!=macro.tiles[tile].pred_tiles.end(); j++){
      if(macro.tiles_sche[*j].end >= i->start){
	cout << "Voilate tile dependency!";
	cout << "  tile " << *j << " end at " << macro.tiles_sche[*j].end << endl;
	cout << "  tile " << tile << " start at " << i->start << endl;
	pass = false; 
      }
    }
  }
  return pass;
}

bool TileScheduling::PassValidSpill(){
  //op user can not across spilling
  bool pass = true;
  for(vector<Tile>::iterator i=macro.tiles.begin(); i!=macro.tiles.end(); i++){
    for(vector<int>::iterator op=i->liveout_ops.begin(); op!=i->liveout_ops.end(); op++){
      //liveout op: *op
      int last_user_loc = 0;
      for(vector<int>::iterator user_tile=i->post_tiles[*op].begin(); user_tile!=i->post_tiles[*op].end(); user_tile++){
	int user_loc = macro.tiles_sche[*user_tile].start+macro.tiles[*user_tile].livein_ops[*op];
	for(vector<int>::iterator sp=op_spills[*op].begin(); sp!=op_spills[*op].end(); sp++){
	  if(macro.spills_sche[*sp].cond == 1){
	    if((macro.spills_sche[*sp].start<user_loc) && (macro.spills_sche[*sp].end>=user_loc)){
	      cout << "Spill " << *sp << " across user tile " << *user_tile << " at loc " << user_loc << endl;
	      pass = false;
	    }
	  }
	}
	if(user_loc > last_user_loc)
	  last_user_loc = user_loc;
      }

      for(vector<int>::iterator sp=op_spills[*op].begin(); sp!=op_spills[*op].end(); sp++){
	if(macro.spills_sche[*sp].cond == 1){
	  if(macro.spills_sche[*sp].end >= last_user_loc){
	    cout << "Spill " << *sp << "behind last user" << endl;
	    pass = false;
	  }
	}
      }
    }
  }

  return pass;
}

bool TileScheduling::PassCBBound(){
  bool pass = true;
  for(map<string, vector<int>>::iterator i=num_cb.begin(); i!=num_cb.end(); i++){
    for(vector<int>::iterator c=i->second.begin(); c!=i->second.end(); c++){
      if(*c > ComputeBlockLib::num_cb[i->first]){
	cout << "CB " << i->first << " used " << *c << " at cycle " << c-i->second.begin() << endl; 
	pass = false;
      }
    }
  }
  return pass;
}

bool TileScheduling::PassPortBound(){
  bool pass = true;
  for(vector<vector<int>>::iterator i=macro.mem->num_port.begin(); i!=macro.mem->num_port.end(); i++){
    int mem_bank = i-macro.mem->num_port.begin();
    for(vector<int>::iterator j=i->begin(); j!=i->end(); j++){
      if(*j > macro.mem->membanks[mem_bank].num_port){
	cout << "bank " << mem_bank << ":num of port is " << *j << " at cycle " << j-i->begin() << endl;
	pass = false;
      }
    }
  }
  return pass;
}

bool TileScheduling::PassMemoryBound(){
  bool pass = true;
  for(auto i=macro.mem->num_live.begin(); i!=macro.mem->num_live.end(); i++){
    int mem_bank = i-macro.mem->num_live.begin();
    for(auto j=i->begin(); j!=i->end(); j++){
      int mem_loc = j-i->begin();
      for(auto k=j->begin(); k!=j->end(); k++){
	if(*k > 1){
	  cout << "bank " << mem_bank << " loc " << mem_loc << ":num of liveness is " << *k << " at cycle " << k-j->begin() << endl;
	  pass = false;
	}
      }
    }
  }
  return pass;
}

bool TileScheduling::Testing(){
  bool p1 = PassTileDependency();
  bool p2 = PassValidSpill();
  bool p3 = PassCBBound();
  bool p4 = PassPortBound();
  bool p5 = PassMemoryBound();
  bool p = p1 && p2 && p3 && p4 && p5;
  if(!p1)
    cout << "Tile dependency failed" << endl;
  if(!p2)
    cout << "Valid spill failed" << endl;
  if(!p3)
    cout << "CB Bound failed" << endl;
  if(!p4)
    cout << "Port bound failed" << endl;
  if(!p5)
    cout << "Memory bound failed" << endl;

  if(p){
    //cout << "Testing passed" << endl;
  }
  else{
    cout << "Testing failed" << endl;
    throw exception();
  }
  return p;
}

void TileScheduling::PrintPerf(){
  cout << "last cycle " << last_cycle << endl;
  cout << "sum of spills " << macro.spills_sche.size() << endl;
  cout << "sum of bankex " << macro.bankexs_sche.size() << endl;
}


