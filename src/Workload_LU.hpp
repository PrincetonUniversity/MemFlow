#ifndef WORKLOAD_LU
#define WORKLOAD_LU

#include "Workload.hpp"

using namespace std;

class Workload_LU: public Workload{
  public:
    Workload_LU(string app_name, int in_m, int in_n, int in_k):Workload(app_name, in_m, in_n, in_k){cout << "constructor of workload lu" << endl; cout << "m " << m << endl;};
    ~Workload_LU(){};

    void setupDataInfo();

    void genSubblkSet(vector<array<int,3>>& sb_dim_set);
    void updatePara_LU();
    void updatePara_LUCPL();
    void updatePara_TRS();
    void updatePara_SUBMM();
    unsigned long long getSpill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2);
    void optiPara();

    void genMacroNode();
    void LoopStructure();
    void setDblkIdx();
    void runMacroNode();

    typedef void(Workload_LU::*func_p)();
    func_p func_iteration;
};

#endif
