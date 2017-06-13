#ifndef WORKLOAD_MM
#define WORKLOAD_MM

#include "Workload.hpp"

using namespace std;

class Workload_MM: public Workload{
  public:
    Workload_MM(string app_name, int in_m, int in_n, int in_k):Workload(app_name, in_m, in_n, in_k){cout << "constructor of workload mm" << endl; cout << "m " << m << endl;};
    ~Workload_MM(){};

    void setupDataInfo();

    void genSubblkSet(vector<array<int,3>>& sb_dim_set);
    void updatePara();
    unsigned long long getSpill(LoopOrder& lo, unsigned long long& spill1, unsigned long long& spill2);
    void optiPara();

    void genMacroNode();
    void LoopStructure();
    void setDblkIdx();
    void runMacroNode();

    typedef void(Workload_MM::*func_p)();
    func_p func_iteration;
};

#endif
