#include <map>
#include <list>

#include "../DRAMSim2/MemorySystem.h"
#include "../DRAMSim2/Transaction.h"
#include "../DRAMSim2/IniReader.h"

using namespace std;
using namespace DRAMSim;

class TransactionReceiver{
  public:
    map<uint64_t, list<uint64_t>> pendingReadRequests;
    map<uint64_t, list<uint64_t>> pendingWriteRequests;
    int num_pending;

    TransactionReceiver(){num_pending=0;};
    ~TransactionReceiver(){};

    void add_pending(const Transaction&, uint64_t cycle);
    void read_complete(unsigned id, uint64_t address, uint64_t done_cycle);
    void write_complete(unsigned id, uint64_t address, uint64_t done_cycle);
    void alignTransactionAddress(Transaction &trans);
};
