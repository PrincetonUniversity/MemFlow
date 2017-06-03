#ifndef APPLICATION
#define APPLICATION

#include "MacroNodeTemplate.hpp"
#include "OptiMacroNode.hpp"
#include "Hardware.hpp"
#include "Memory.hpp"
#include "Setting.hpp"
#include "CGScheduling.hpp"

using namespace std;

extern vector<MacroNodeTemplate*> mn_temps;

struct data_info{
  string name;
  int dimi;
  int dimj;
  int blk_dimi;
  int blk_dimj;
  data_info(string in_name, int in_dimi, int in_dimj, int in_blk_dimi, int in_blk_dimj):name(in_name),dimi(in_dimi),dimj(in_dimj),blk_dimi(in_blk_dimi),blk_dimj(in_blk_dimj){};
};

class Application{
  public:
  Application(string app_name, int in_m, int in_n, int in_k);
  ~Application(){};

  void setupDataInfo();
  void setup();
  void run();
  void test_mntemp();

  string name;
  int m;
  int n;
  int k;

  vector<data_info> datas_info;
};

#endif
