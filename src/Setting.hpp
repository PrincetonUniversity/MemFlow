#ifndef SETTING
#define SETTING

#include <map>
#include <vector>
#include "Hardware.hpp"
#include "MacroNodeTemplate.hpp"

namespace ComputeBlockLib{
  extern map<string, ComputeBlock*> cbs;
  extern map<string, int> num_cb;
}

namespace Memory{
  extern int num_bank;
  extern vector<MemBank> membanks;
}

extern vector<MacroNodeTemplate*> mn_temps;

extern Parameters opti_para;

extern MemoryTrack* global_sp;
#endif
