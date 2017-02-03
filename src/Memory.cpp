#include <vector>
#include <iostream>

#include "Memory.hpp"
#include "Setting.hpp"

using namespace std;

MemoryTrack::MemoryTrack(){
  num_bank = Memory::num_bank;
  membanks = Memory::membanks;
}

MemoryTrack::MemoryTrack(vector<int> &ops, bool use_compute_size){
  num_bank = Memory::num_bank;
  membanks = Memory::membanks;
 
  if(use_compute_size){
    for(int i=0; i<num_bank; i++){
      membanks[i].size = membanks[i].compute_size;
    }
  }

  for(int i=0; i<num_bank; i++){
    vector<int> m;
    num_port.push_back(m);

    vector<vector<int>> bank;
    for(int j=0; j<membanks[i].size; j++){
      vector<int> live;
      bank.push_back(live);
    }
    num_live.push_back(bank);
  }

  for(vector<int>::iterator op=ops.begin(); op!=ops.end(); op++){
    vector<array<int,2>> t;
    op_in_bank[*op] = t;
  }

  begin = 0;
}

void MemoryTrack::setBankSize(int bank_i, int size){
  membanks[bank_i].size = size;
}

void MemoryTrack::useComputeSize(){
  for(int i=0; i<num_bank; i++){
    membanks[i].size = membanks[i].compute_size;
  }
}

int MemoryTrack::getLength(){
  return num_port[0].size();
}

void MemoryTrack::ExtendLength(int to_size){
  if(getLength() < to_size){
    for(auto i=op_in_bank.begin(); i!=op_in_bank.end(); i++){
      for(int n=begin+i->second.size(); n<=to_size-1; n++){
	array<int,2> a = {-1,-1};
	i->second.push_back(a);
      }
    }

    for(int i=0; i<num_bank; i++){
      for(int j=0; j<membanks[i].size; j++){
        for(int n=begin+num_live[i][j].size(); n<=to_size-1; n++){
	  num_live[i][j].push_back(0);
        }
      }
      for(int n=begin+num_port[i].size(); n<=to_size-1; n++){
	num_port[i].push_back(0);
      }
    }
  }
}

/*
void MemoryTrack::AddMN(int cycle, MacroNode* mn){
  for(int i=0; i<Memory::num_bank; i++){
    for(int n=0; n<mn->mn_temp->max_cycle; n++){
      num_port[i][cycle+n-begin] += mn->mn_temp->num_port[i][n];
      num_live[i][cycle+n-begin] += mn->mn_temp->num_live[i][n];
    }
  }
}

void MemoryTrack::RemoveMN(int cycle, MacroNode* mn){
  for(int i=0; i<Memory::num_bank; i++){
    for(int n=0; n<mn->mn_temp->max_cycle; n++){
      num_port[i][cycle+n-begin] -= mn->mn_temp->num_port[i][n];
      num_live[i][cycle+n-begin] -= mn->mn_temp->num_live[i][n];
    }
  }
}

bool MemoryTrack::CanAddMN(int cycle, MacroNode* mn){
  for(int i=0; i<Memory::num_bank; i++){
    for(int n=0; n<mn->mn_temp->max_cycle; n++){
      if(num_port[i][cycle+n-begin]+mn->mn_temp->num_port[i][n] > Memory::membanks[i].num_port){
	return false; 
      }
      if(num_live[i][cycle+n-begin]+mn->mn_temp->num_live[i][n] > Memory::membanks[i].size){
	return false;
      }
    }
  }
  return true;
}
*/


bool MemoryTrack::CanPortInc(int bank_i, int cycle){
  if(num_port[bank_i][cycle-begin]+1 > membanks[bank_i].num_port){
    return false;
  }
  else{
    return true;
  }
}

bool MemoryTrack::CanLiveInc(int bank_i, int loc_j, int cycle){
  if(num_live[bank_i][loc_j][cycle-begin] != 0){
    return false;
  }
  else{
    return true;
  }
}

void MemoryTrack::PortInc(int bank_i, int cycle){
  num_port[bank_i][cycle-begin]++;
}

void MemoryTrack::LiveInc(int bank_i, int loc_j, int cycle){
  num_live[bank_i][loc_j][cycle-begin]++;
}

int MemoryTrack::CanLiveInc_range(int bank_i, int start_c, int end_c){
  for(int j=0; j<membanks[bank_i].size; j++){
    bool can_inc = true;
    for(int c=start_c; c<=end_c; c++){
      if(!CanLiveInc(bank_i, j, c)){
        can_inc = false;
	break;
      }
    }
    if(can_inc){
      return j;
    }
  }
  return -1;
}

void MemoryTrack::LiveInc_range(int bank_i, int loc_j, int start_c, int end_c){
  for(int c=start_c; c<=end_c; c++){
    LiveInc(bank_i, loc_j, c);
  }
}

void MemoryTrack::OpinbankSet(int op, int bank_i, int loc_j, int cycle){
  array<int,2> a = {bank_i, loc_j};
  op_in_bank[op][cycle-begin] = a;
}

void MemoryTrack::OpinbankSet_range(int op, int bank_i, int loc_j, int start_c, int end_c){
  for(int c=start_c; c<=end_c; c++){
    OpinbankSet(op, bank_i, loc_j, c);
  }
}

int MemoryTrack::getOpBank(int op, int cycle){
  return op_in_bank[op][cycle-begin][0];
}

int MemoryTrack::getOpLoc(int op, int cycle){
  return op_in_bank[op][cycle-begin][1];
}

void MemoryTrack::PrintPort(int bank_i){
  cout << "num of ports of bank " << bank_i << endl;
  for(int i=0; i<num_port[bank_i].size(); i++){
    cout << begin+i << ": " << num_port[bank_i][i] << endl;
  }
}

void MemoryTrack::PrintLive(int bank_i, int loc_j){
  cout << "num of live of bank " << bank_i << " loc " << loc_j << endl;
  for(int i=0; i<num_live[bank_i][loc_j].size(); i++){
    cout << begin+i << ": " << num_live[bank_i][loc_j][i] << endl;
  }
}

/*
void MemoryTrack::CutMem(int new_begin){
  for(auto &i: num_port){
    i.erase(i.begin(), i.begin()+new_begin-begin);
  }
  for(auto &i: num_live){
    i.erase(i.begin(), i.begin()+new_begin-begin);
  }
  for(auto &i: op_in_bank){
    i.second.erase(i.second.begin(), i.second.begin()+new_begin-begin);
  }
  begin = new_begin;
}
*/

void MemoryTrack::VerifyPortBound(){
  for(int i=0; i<num_bank; i++){
    for(size_t c=0; c<num_port[i].size(); c++){
      if(num_port[i][c] > membanks[i].num_port){
	cout << "num_port of bank " << i << " at cycle " << begin+c << " is " << num_port[i][c] << endl;
	throw exception();
      }
    }
  }
}

void MemoryTrack::VerifyLiveBound(){
  for(int i=0; i<num_bank; i++){
    for(int j=0; j<membanks[i].size; j++){
      for(size_t c=0; c<num_live[i][j].size(); c++){
        if(num_live[i][j][c] > 1){
	  cout << "num_live of bank " << i << " loc " << j << " at cycle " << begin+c << " is " << num_live[i][j][c] << endl;
        }
      }
    }
  }
}

int MemoryTrack::getLastCycle(){
  return begin+num_port[0].size()-1;
}

void MemoryTrack::getMaxNumLive(){
  cout << "max liveness in each bank " << endl;
  for(int i=0; i<num_bank; i++){

    int max_num = 0;
    for(size_t c=0; c<getLength(); c++){
      int sum_live = 0;
      for(int j=0; j<membanks[i].size; j++){
        sum_live += num_live[i][j][c];
      }
      if(sum_live > max_num){
	max_num = sum_live;
      }
    }
    max_num_live.push_back(max_num);
    cout << max_num << ",";
  }
  cout << endl;
}

void MemoryTrack::Slice2Blocks(int blk_dimi, int blk_dimj, int blk_diml, int m, int n, int k){
  int a_blk_size = blk_dimi*blk_diml;
  int b_blk_size = blk_dimj*blk_diml;
  int c_blk_size = blk_dimi*blk_dimj;

  a_row_space = (a_blk_size%num_bank==0)?a_blk_size/num_bank: a_blk_size/num_bank+1;
  b_row_space = (b_blk_size%num_bank==0)?b_blk_size/num_bank: b_blk_size/num_bank+1;
  c_row_space = (c_blk_size%num_bank==0)?c_blk_size/num_bank: c_blk_size/num_bank+1;

  a_space = a_row_space * num_bank;
  b_space = b_row_space * num_bank;
  c_space = c_row_space * num_bank;

  int num_a = k/blk_diml;
  int num_b = (n/blk_dimj)*(k/blk_diml);

  int idx = membanks[0].compute_size;
  //two space for c
  c.push_back(-1);
  c.push_back(-1);
  //one space for a,b
  a.push_back(-1);
  b.push_back(-1);
  idx += 2*c_row_space+a_row_space+b_row_space;

  for(int i=1; i<num_a; i++){
    if(idx+a_row_space < membanks[0].size){
      a.push_back(-1);
      idx += a_row_space;
    }
    else{
      break;
    }
  }

  for(int i=1; i<num_b; i++){
    if(idx+b_row_space < membanks[0].size){
      b.push_back(-1);
      idx += b_row_space;
    }
    else{
      break;
    }
  }

  c_base = membanks[0].compute_size;
  a_base = c_base + c.size()*c_row_space;
  b_base = a_base + a.size()*a_row_space;

  cout << "a mem blk size " << a.size() << endl;
  cout << "b mem blk size " << b.size() << endl;
  cout << "c mem blk size " << c.size() << endl;

}

int MemoryTrack::getBase_a(int idx){
  return a_base+idx*a_row_space;
}

int MemoryTrack::getBase_b(int idx){
  return b_base+idx*b_row_space;
}

int MemoryTrack::getBase_c(int idx){
  return c_base+idx*c_row_space;
}


