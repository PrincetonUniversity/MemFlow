#include <vector>
#include <iostream>

#include "Memory.hpp"
#include "Setting.hpp"

using namespace std;

MemoryTrack::MemoryTrack(Parameters& in_opti_para):opti_para(in_opti_para){
  num_bank = Memory::num_bank;
  membanks = Memory::membanks;
}

MemoryTrack::MemoryTrack(vector<int> &ops, Parameters& in_opti_para):opti_para(in_opti_para){
  num_bank = Memory::num_bank;
  membanks = Memory::membanks;
 
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

void MemoryTrack::Slice2Dblks(){
  int ablk = opti_para.blk_dimi*opti_para.blk_diml;
  int bblk = opti_para.blk_dimj*opti_para.blk_diml;
  int cblk = opti_para.blk_dimi*opti_para.blk_dimj;

  a_region = (ablk%opti_para.num_bank_a==0)?ablk/opti_para.num_bank_a: ablk/opti_para.num_bank_a+1;
  b_region = (bblk%opti_para.num_bank_b==0)?bblk/opti_para.num_bank_b: bblk/opti_para.num_bank_b+1;
  c_region = (cblk%opti_para.num_bank_c==0)?cblk/opti_para.num_bank_c: cblk/opti_para.num_bank_c+1;

  cout << "a blk " << ablk << endl;
  cout << "b blk " << bblk << endl;
  cout << "c blk " << cblk << endl;
  cout << "num bank a " << opti_para.num_bank_a << endl;
  cout << "num bank b " << opti_para.num_bank_b << endl;
  cout << "num bank c " << opti_para.num_bank_c << endl;
  cout << "a region " << a_region << endl;
  cout << "b region " << b_region << endl;
  cout << "c region " << c_region << endl;

  num_region_a = Memory::membanks[0].size/a_region;
  num_region_b = Memory::membanks[0].size/b_region;
  num_region_c = Memory::membanks[0].size/c_region;

  cout << "a mem dblk num: " << num_region_a << endl;
  cout << "b mem dblk num: " << num_region_b << endl;
  cout << "c mem dblk num: " << num_region_c << endl;
}


array<int,2> MemoryTrack::getAddr_a_ele(int base, int m, int n, int i, int j){
  array<int,2> addr;
  
  int blk_i = i/opti_para.subblk_dimi;
  int blk_idx = (blk_i*n+j)/opti_para.subblk_diml;
  //addr in bank
  int addr_offset = base+2*blk_idx;

  int i_insubblk = i%opti_para.subblk_dimi;
  int j_insubblk = (blk_i*n+j)%opti_para.subblk_diml;
  int idx_insubblk = j_insubblk*opti_para.subblk_dimi + i_insubblk;
  //bank
  addr[0] = idx_insubblk/2;
  addr[1] = addr_offset+idx_insubblk%2;

  return addr;
}

array<int,2> MemoryTrack::getAddr_b_ele(int base, int m, int n, int i, int j){
  array<int,2> addr;

  int blk_n = n/opti_para.subblk_dimj;
  int blk_i = i/opti_para.subblk_diml;
  int blk_j = j/opti_para.subblk_dimj;
  int blk_idx = blk_i*blk_n+blk_j;
  int addr_offset = base+2*blk_idx;

  int i_insubblk = i%opti_para.subblk_diml;
  int j_insubblk = j%opti_para.subblk_dimj;
  int idx_insubblk = i_insubblk*opti_para.subblk_dimj+j_insubblk;
  addr[0] = opti_para.num_bank_a+idx_insubblk/2;
  addr[1] = addr_offset+idx_insubblk%2;
  return addr;
}

array<int,2> MemoryTrack::getAddr_c_ele(int base, int m, int n, int i, int j, int in_out_latency){
  array<int,2> addr;

  int blk_n = n/opti_para.subblk_dimj;
  int blk_i = i/opti_para.subblk_dimi;
  int blk_j = j/opti_para.subblk_dimj;
  int blk_idx = blk_i*blk_n+blk_j;
  
  //int blk_batch_idx = blk_idx/in_out_latency;
  //int blk_idx_inbatch = blk_idx%in_out_latency;

  //cout << "blk batch idx " << blk_batch_idx << endl;
  //cout << "blk idx in batch " << blk_idx_inbatch << endl;

  //int blk_type = blk_batch_idx%2;
  //int blk_round = blk_batch_idx/2;

  //cout << "blk type " << blk_type << endl;
  //cout << "blk round " << blk_round << endl;

  //int addr_offset = dblk_idx+2*(blk_round*in_out_latency+blk_idx_inbatch);
  
  int i_insubblk = i%opti_para.subblk_dimi;
  int j_insubblk = j%opti_para.subblk_dimj;
  int idx_insubblk = i_insubblk*opti_para.subblk_dimj+j_insubblk;

  //if(blk_type == 0){
  //  addr[0] = (opti_para.num_bank_a+opti_para.num_bank_b)+idx_insubblk/2;
  //}
  //else{
  //  addr[0] = (opti_para.num_bank_a+opti_para.num_bank_b+opti_para.num_bank_c/2)+idx_insubblk/2;
  //}
  addr[0] = (opti_para.num_bank_a+opti_para.num_bank_b)+idx_insubblk;
  addr[1] = base+blk_idx;
  return addr;
}

int MemoryTrack::getBase_a(int blk_idx){
  return blk_idx*a_region;
}

int MemoryTrack::getBase_b(int blk_idx){
  return blk_idx*b_region;
}

int MemoryTrack::getBase_c(int blk_idx){
  return blk_idx*c_region;
}



