#ifndef WORKLOAD_QR
#define WORKLOAD_QR

#include "Workload.hpp"

using namespace std;

class Workload_QR: public Workload{
  public:
    Workload_QR(string app_name, int in_m, int in_n, int in_k):Workload(app_name, in_m, in_n, in_k){cout << "constructor of workload qr" << endl; cout << "m " << m << endl;};
    ~Workload_QR(){};

    void setupDataInfo();

    void genSubblkSet(vector<array<int,3>>& sb_dim_set);
    void updatePara_QR();
    void updatePara_QRCPL();
    void updatePara_QRUPDATETR();
    void updatePara_QRUPDATE();
    unsigned long long getSpill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2);
    void optiPara();

    void genMacroNode();
    void LoopStructure();
    void setDblkIdx();
    void runMacroNode();

    typedef void(Workload_QR::*func_p)();
    func_p func_iteration;
};

#endif
