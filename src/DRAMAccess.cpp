#include "DRAMAccess.hpp"

void TransactionReceiver::add_pending(const Transaction& t, uint64_t cycle){
  if(t.transactionType == DATA_READ){
    pendingReadRequests[t.address].push_back(cycle);
    num_pending++;
  }
  else if(t.transactionType == DATA_WRITE){
    pendingWriteRequests[t.address].push_back(cycle);
    num_pending++;
  }
  else{
    cout << "Transaction is not read or write" << endl;
  }
}

void TransactionReceiver::read_complete(unsigned id, uint64_t address, uint64_t done_cycle){
  if(pendingReadRequests.find(address) == pendingReadRequests.end()){
    cout << "Can't find a pending read for this one" << endl;
  }
  else{
    uint64_t added_cycle = pendingReadRequests[address].front();
    uint64_t latency = done_cycle - added_cycle;
    pendingReadRequests[address].pop_front();
    num_pending--;
    cout<<"Read Callback: 0x"<<std::hex<<address<<std::dec<<" latency="<<latency<<" cycles("<<added_cycle<<"->"<<done_cycle<<")"<<endl;
  }
}

void TransactionReceiver::write_complete(unsigned id, uint64_t address, uint64_t done_cycle){
  if(pendingWriteRequests.find(address) == pendingWriteRequests.end()){
    cout << "Can't find a pending write for this one" << endl;
  }
  else{
    uint64_t added_cycle = pendingWriteRequests[address].front();
    uint64_t latency = done_cycle-added_cycle;
    pendingWriteRequests[address].pop_front();
    num_pending--;
    cout<<"Write Callback: 0x"<<std::hex<<address<<std::dec<<" latency="<<latency<<" cycles("<<added_cycle<<"->"<<done_cycle<<")"<<endl;
  }
}

void TransactionReceiver::alignTransactionAddress(Transaction& trans){
  unsigned throwAwayBits = THROW_AWAY_BITS;
  trans.address >>= throwAwayBits;
  trans.address <<= throwAwayBits;
}
