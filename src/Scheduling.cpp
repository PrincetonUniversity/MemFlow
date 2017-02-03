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





int TileScheduling::FindCycle_dblks(int tile){
  cout << endl << "tile " << tile << endl;
  int last_tile_cycle = -1;
  if(tile > 0){
    last_tile_cycle = macro.tiles_sche[tile-1].start;
  }

  map<int, int> read_cycle;
  
  array<int, 3> bank_read = {0,0,0};
  for(auto op=macro.tiles[tile].livein_ops.begin(); op!=macro.tiles[tile].livein_ops.end(); op++){

    int start_cycle;
    if(tile == 0){
      start_cycle = 0;
    }
    else{
      start_cycle = last_tile_cycle+op->second;
    }
    macro.mem->ExtendLength(start_cycle+1);

    int bank = macro.ioop_addr[op->first][0];
    int cycle = start_cycle;
    while(macro.mem->num_port[bank][cycle]+1 > macro.mem->membanks[bank].num_port){
      cycle++;
      macro.mem->ExtendLength(cycle+1);
    }
    macro.mem->num_port[bank][cycle]++;
    read_cycle[op->first] = cycle;
    bank_read[bank]++;
  }

  cout << "read cycles" << endl;
  for(auto i=read_cycle.begin(); i!=read_cycle.end(); i++){
    cout << i->first << ":" << i->second << endl;
  }

  cout << "num read in each bank " << endl;
  for(int i=0; i<3; i++){
    cout << "bank " << i << ": " << bank_read[i] << endl;
  }

  //find the earliest cycle the tile can be scheduled
  int cycle = 0;
  for(auto op=macro.tiles[tile].livein_ops.begin(); op!=macro.tiles[tile].livein_ops.end(); op++){
    if(cycle+op->second < read_cycle[op->first]){
      cycle = read_cycle[op->first]-op->second;
    }
  }
  cout << "schedule tile at cycle " << cycle << endl;
  return cycle;
}


void TileScheduling::AllocateTile_dblks(int tile, int cycle){
  macro.tiles_sche[tile].start = cycle;
  macro.tiles_sche[tile].end = cycle+macro.tiles[tile].latency-1;
}


void TileScheduling::Scheduling_dblks(){
  for(auto t=macro.tiles.begin(); t!=macro.tiles.end(); t++){
    int tile = t-macro.tiles.begin();
    int cycle = FindCycle_dblks(tile);
    AllocateTile_dblks(tile, cycle);
  }

  for(int i=0; i<macro.mem->num_bank; i++){
    //bank
    cout << "bank " << i << endl;
    for(int c=0; c<macro.mem->num_port[i].size(); c++){
      cout << macro.mem->num_port[i][c] << endl;
    }
  }
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


