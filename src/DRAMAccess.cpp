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
  //done cycle: mem cycle
  //added cycle: cpu cycle
  if(pendingReadRequests.find(address) == pendingReadRequests.end()){
    cout << "Can't find a pending read for this one" << endl;
  }
  else{
    read_added_cycle = pendingReadRequests[address].front();
    //uint64_t latency = done_cycle - added_cycle;
    pendingReadRequests[address].pop_front();
    num_pending--;
    has_read = true;
    //cout << "Read Callback: 0x"<<std::hex<<address<<std::dec<<" memory cycle " << done_cycle << endl;
  }
}

void TransactionReceiver::write_complete(unsigned id, uint64_t address, uint64_t done_cycle){
  if(pendingWriteRequests.find(address) == pendingWriteRequests.end()){
    cout << "Can't find a pending write for this one" << endl;
  }
  else{
    write_added_cycle = pendingWriteRequests[address].front();
    //uint64_t latency = done_cycle-added_cycle;
    pendingWriteRequests[address].pop_front();
    num_pending--;
    has_write = true;
    //cout << "Write Callback: 0x"<<std::hex<<address<<std::dec<<" memory cycle "<< done_cycle << endl;
  }
}

void TransactionReceiver::cpu_callback(uint64_t cpu_cycle){
  if(has_read){
    uint64_t latency = cpu_cycle-read_added_cycle;
    //cout << "CPU cycle latency " << latency << " cycles("<<read_added_cycle << "->"<<cpu_cycle<<")" << endl;
    has_read = false;
  }
  if(has_write){
    uint64_t latency = cpu_cycle-write_added_cycle;
    //cout << "CPU cycle latency " << latency << " cycles("<<write_added_cycle<<"->"<<cpu_cycle<<")" << endl;
    has_write = false;
  }
}



void TransactionReceiver::alignTransactionAddress(Transaction& trans){
  unsigned throwAwayBits = THROW_AWAY_BITS;
  trans.address >>= throwAwayBits;
  trans.address <<= throwAwayBits;
}
