#include<iostream>
#include<vector>
#include<algorithm>
#include<climits>

#include "MacroNode.hpp"
#include "Setting.hpp"

using namespace std;
using namespace DRAMSim;

LoadStoreDblk::LoadStoreDblk(){
  read_cb = new Callback<TransactionReceiver, void, unsigned, uint64_t, uint64_t>(&transactionReceiver, &TransactionReceiver::read_complete);
  write_cb = new Callback<TransactionReceiver, void, unsigned, uint64_t, uint64_t>(&transactionReceiver, &TransactionReceiver::write_complete);
  dram->RegisterCallbacks(read_cb, write_cb, NULL);
}

LoadStoreDblk::~LoadStoreDblk(){
  delete read_cb;
  delete write_cb;
}

void LoadStoreDblk::genTransactions(DataBlock* dblk, queue<uint64_t>& trans_addr){
  //sweep all elements in the dblk
  int i_start = dblk->blk_i*dblk->blk_dimi;
  int i_end = (dblk->blk_i+1)*dblk->blk_dimi;
  int j_start = dblk->blk_j*dblk->blk_dimj;
  int j_end = (dblk->blk_j+1)*dblk->blk_dimj;

  //map<uint64_t, map<int, int>> addr_bank;
  set<uint64_t> addr; 
  for(int i=i_start; i<i_end; i++){
    for(int j=j_start; j<j_end; j++){
      //get address for this element
      //cout << "i,j: " << i << "," << j << endl;
      uint64_t phy_addr = data_arrays[dblk->matrix_name]->genPhysicalAddr(i,j);
      //cout << "phy addr: " << phy_addr << endl;
      //transaction addr
      unsigned throwAwayBits = THROW_AWAY_BITS;
      phy_addr >>= throwAwayBits;
      phy_addr <<= throwAwayBits;
      //cout << "tran addr: " << phy_addr << endl;
    
      if(addr.find(phy_addr) == addr.end()){
        addr.insert(phy_addr);
      }
      //get bank
      //int bank = dblk->getElementSPAddr(i,j)[0];
      //cout << "bank " << bank << endl;

      //if(addr_bank.find(phy_addr) == addr_bank.end()){
      //  addr_bank[phy_addr][bank] = 1;
      //}
      //else{
      //  if(addr_bank[phy_addr].find(bank) == addr_bank[phy_addr].end()){
	//  addr_bank[phy_addr][bank] = 1;
	//}
	//else{
	  //addr_bank[phy_addr][bank]++;
	//}
      //}
    }
  }

  for(auto &i: addr){
    trans_addr.push(i);
  }
  //for(auto &tran: addr_bank){
  //  auto max_bank_use = max_element(tran.second.begin(), tran.second.end(), tran.second.value_comp());
  //  int num_batch = (max_bank_use->second%2==0)?max_bank_use->second/2:max_bank_use->second/2+1;
  //  for(int i=0; i<num_batch; i++){
  //    trans_addr.push(tran.first);
  //  }
  //}
}

int LoadStoreDblk::loadDblk(DataBlock* dblk){
  //cout << "create transaction receiver" << endl;
  queue<uint64_t> trans_addr;
  genTransactions(dblk, trans_addr);
  //cout << "trans addr" << endl;
  //for(auto &i: trans_addr){
    //cout << hex << i << endl;
  //}
  
  int numCycles = INT_MAX;
  bool pendingTrans = false;
  Transaction* trans = NULL;
  void* data = NULL;

  int finish_cycle;
  for(int i=0; i<numCycles; i++){
    if(!pendingTrans){
      if(!trans_addr.empty()){
        //new tran
	uint64_t addr = trans_addr.front();
	//cout << "***tran addr " << addr << endl;
	trans_addr.pop();
	trans = new Transaction(DATA_READ, addr, data);
	if(!dram->addTransaction(false, addr)){
	  pendingTrans = true;
	}
	else{
	  transactionReceiver.add_pending(*trans,i);
	  trans = NULL;
	}
      }
      else{
        pendingTrans = false;
      }
    }
    else{
      pendingTrans = !dram->addTransaction(false, trans->address);
      if(!pendingTrans){
        transactionReceiver.add_pending(*trans,i);
	trans = NULL;
      }
    }
    dram->update();
    //has callback
    transactionReceiver.cpu_callback(i);

    //cout << "pending num: " << transactionReceiver.num_pending << endl;
    if(transactionReceiver.num_pending == 0){
      cout << "***************finish cycle " << i << endl;
      finish_cycle = i;
      break;
    }
  }
  if(trans){
    delete trans;
  }
  return finish_cycle;
}

int LoadStoreDblk::storeDblk(DataBlock* dblk){
  //cout << "create transaction receiver" << endl;
  
  queue<uint64_t> trans_addr;
  genTransactions(dblk, trans_addr);
  //cout << "generated trans size: " << trans_addr.size() << endl;
  //cout << "trans addr" << endl;
  //for(auto &i: trans_addr){
    //cout << hex << i << endl;
  //}
  
  //cout << "in store dblk" << endl;
  int numCycles = INT_MAX;
  bool pendingTrans = false;
  Transaction* trans = NULL;
  void* data = NULL;

  int finish_cycle;
  for(int i=0; i<numCycles; i++){
    if(!pendingTrans){
      if(!trans_addr.empty()){
        //new tran
	uint64_t addr = trans_addr.front();
	//cout << "***tran addr " << addr << endl;
	trans_addr.pop();
	trans = new Transaction(DATA_WRITE, addr, data);
	if(!dram->addTransaction(true, addr)){
	  pendingTrans = true;
	}
	else{
	  transactionReceiver.add_pending(*trans,i);
	  trans = NULL;
	}
      }
      else{
        pendingTrans = false;
      }
    }
    else{
      pendingTrans = !dram->addTransaction(true, trans->address);
      if(!pendingTrans){
        transactionReceiver.add_pending(*trans,i);
	trans = NULL;
      }
    }
    dram->update();
    transactionReceiver.cpu_callback(i);
    //cout << "pending num: " << transactionReceiver.num_pending << endl;
    if(transactionReceiver.num_pending == 0){
      cout << "***************finish cycle " << i << endl;
      finish_cycle = i;
      break;
    }
  }
  if(trans){
    delete trans;
  }
  return finish_cycle;
}

MacroNode::MacroNode(MacroNodeTemplate* in_mn_temp):mn_temp(in_mn_temp){
  name = mn_temp->name;
}

MacroNode::MacroNode(MacroNodeTemplate* in_mn_temp, int in_idx):mn_temp(in_mn_temp), idx(in_idx){
  name = mn_temp->name;
}

MacroNode::MacroNode(int in_idx):idx(in_idx){
}

void MacroNode::BuildOpMap(vector<Tile> &real_tiles, vector<Operation> &real_ops){
  //cout << endl << "mn " << idx << endl;
  for(map<int,int>::iterator t=tile_map.begin(); t!=tile_map.end(); t++){
    //cout << "real tile " << t->first << ": " << " tile in mn " << t->second << endl;
    vector<int>::iterator op2 = mn_temp->p_cg->tiles[t->second].ops.begin();
    for(vector<int>::iterator op1 = real_tiles[t->first].ops.begin(); op1!=real_tiles[t->first].ops.end(); op1++){
      //cout << "op in real tile " << *op1 << endl;
      //cout << "op in mn tile " << *op2 << endl;
      vector<int>::iterator inop2 = mn_temp->p_cg->ops[*op2].in.begin();
      for(vector<int>::iterator inop1 = real_ops[*op1].in.begin(); inop1!=real_ops[*op1].in.end(); inop1++){
	//cout <<"inop in real tile " << *inop1 << endl;
	//cout <<"inop in mn tile " << *inop2 << endl;
	if(real_tiles[t->first].livein_ops.find(*inop1) != real_tiles[t->first].livein_ops.end()){
	  op_map[*inop2] = *inop1;
	}
	inop2++;
      }
      op2++;
    }

    vector<int>::iterator out_op1 = real_tiles[t->first].liveout_ops.begin();
    for(vector<int>::iterator out_op2= mn_temp->p_cg->tiles[t->second].liveout_ops.begin(); out_op2!= mn_temp->p_cg->tiles[t->second].liveout_ops.end(); out_op2++){
      if(op_map.find(*out_op2) == op_map.end()){
	op_map[*out_op2] = *out_op1;
      }
      out_op1++;
    }
  }
}


