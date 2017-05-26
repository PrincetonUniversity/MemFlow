#include <vector>
#include <iostream>

#include "Memory.hpp"
#include "Setting.hpp"

using namespace std;

SPRegion::SPRegion(string in_name, int in_start_bank, int in_end_bank){
  name = in_name;
  start_bank = in_start_bank;
  end_bank = in_end_bank;

  farest_next_use = 0;
  next_base_2replace = 0;
}

void SPRegion::setupSPRegion(int in_num_blks){
  num_blks = in_num_blks;
  
  for(int i=0; i<num_blks; i++){
    dblk_idx.push_back(-1);
    next_empty.push(i);
  }
}

void SPRegion::updateNextBase2Replace(int changed_base){
  //new next use
  int idx = dblk_idx[changed_base];
  if(!data_arrays[name]->dblks[idx].next_use_t.empty()){
    int next_use = data_arrays[name]->dblks[idx].next_use_t.front();
    if(next_use > farest_next_use){
      next_base_2replace = changed_base;
      farest_next_use = next_use;
    }
  }
}

MemoryTrack::MemoryTrack(){
  num_bank = Memory::num_bank;
  membanks = Memory::membanks;

  for(int i=0; i<num_bank; i++){
    vector<int> m;
    num_port.push_back(m);
  }

  total_port = 0;
  for(int i=0; i<Memory::num_bank; i++){
    total_port += Memory::membanks[i].num_port;
  }

  total_size = 0;
  for(int i=0; i<Memory::num_bank; i++){
    total_size += Memory::membanks[i].size;
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

    for(int i=0; i<num_bank; i++){
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

void MemoryTrack::getOptiPara(Parameters* in_opti_para){
  opti_para = in_opti_para;
}

void MemoryTrack::Slice2Dblks(){
  cout << "in dblk slicing" << endl;
  int bank_start = 0;
  for(auto &i: opti_para->dblks){
    sp_regions[i.first] = SPRegion(i.first, bank_start, bank_start+i.second.num_bank-1);
    bank_start += i.second.num_bank;

    blk_interval[i.first] = i.second.interval;
    sp_regions[i.first].setupSPRegion(i.second.num_blk_mem);
  }
}


void MemoryTrack::Slice2Dblks_debug(){
  cout << "in dblk slicing" << endl;
  int bank_start = 0;
  for(auto &i: opti_para->dblks){
    sp_regions[i.first] = SPRegion(i.first, bank_start, bank_start+i.second.num_bank-1);
    bank_start += i.second.num_bank;
  }
}
/*
array<int,2> MemoryTrack::getAddr_a_ele(DblkAddr dblk_addr, int m, int n, int i, int j){
  array<int,2> addr;
  
  int blk_i = i/opti_para->subblk_dimi;
  int blk_idx = (blk_i*n+j)/opti_para->subblk_diml;
  //addr in bank
  int addr_offset = dblk_addr.base+2*blk_idx;

  int i_insubblk = i%opti_para->subblk_dimi;
  int j_insubblk = (blk_i*n+j)%opti_para->subblk_diml;
  int idx_insubblk = j_insubblk*opti_para->subblk_dimi + i_insubblk;
  //bank
  //cout << "dblk region " << dblk_addr.region << endl;
  //cout << "start bank " << sp_regions[dblk_addr.region].start_bank << endl;
  //cout << "start bank " << sp_regions["A"].start_bank << endl;
  addr[0] = sp_regions[dblk_addr.region].start_bank+idx_insubblk/2;
  addr[1] = addr_offset+idx_insubblk%2;

  return addr;
}

array<int,2> MemoryTrack::getAddr_b_ele(DblkAddr dblk_addr, int m, int n, int i, int j){
  array<int,2> addr;

  int blk_n = n/opti_para->subblk_dimj;
  int blk_i = i/opti_para->subblk_diml;
  int blk_j = j/opti_para->subblk_dimj;
  int blk_idx = blk_i*blk_n+blk_j;
  int addr_offset = dblk_addr.base+2*blk_idx;

  int i_insubblk = i%opti_para->subblk_diml;
  int j_insubblk = j%opti_para->subblk_dimj;
  int idx_insubblk = i_insubblk*opti_para->subblk_dimj+j_insubblk;
  //cout << "dblk region " << dblk_addr.region << endl;
  //cout << "start bank " << sp_regions[dblk_addr.region].start_bank << endl;
  //cout << "start bank " << sp_regions["B"].start_bank << endl;
  addr[0] = sp_regions[dblk_addr.region].start_bank+idx_insubblk/2;
  addr[1] = addr_offset+idx_insubblk%2;
  return addr;
}

array<int,2> MemoryTrack::getAddr_c_ele(DblkAddr dblk_addr, int m, int n, int i, int j, int in_out_latency){
  array<int,2> addr;

  int blk_n = n/opti_para->subblk_dimj;
  int blk_i = i/opti_para->subblk_dimi;
  int blk_j = j/opti_para->subblk_dimj;
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
  
  int i_insubblk = i%opti_para->subblk_dimi;
  int j_insubblk = j%opti_para->subblk_dimj;
  int idx_insubblk = i_insubblk*opti_para->subblk_dimj+j_insubblk;

  addr[0] = sp_regions[dblk_addr.region].start_bank+idx_insubblk;
  addr[1] = dblk_addr.base+blk_idx;
  return addr;
}
*/

DblkAddr MemoryTrack::getDblkAddr(string mtx_name, int blk_idx){
  DblkAddr da;
  da.region = mtx_name;
  da.base = blk_idx*blk_interval[mtx_name];
  return da;
}



void MemoryTrack::PrintInfo(){
  cout << endl << "SRAM parameters: " << endl;
  cout << "Bank allocation: " << endl;
  for(auto &i: sp_regions){
    cout << "    #banks for " << i.first << ": " << opti_para->dblks[i.first].num_bank << "(" << i.second.start_bank << "-" << i.second.end_bank << ")" << endl;
    cout << "      #blks can be allocated: " << i.second.num_blks << endl;
  }
}
