#ifndef SETTING
#define SETTING

#include <map>
#include <vector>
#include <string>
#include "Hardware.hpp"
//#include "MacroNodeTemplate.hpp"
#include "Data.hpp"
#include "../DRAMSim2/DRAMSim.h"
#include "Memory.hpp"

namespace ComputeBlockLib{
  extern map<string, map<string, ComputeBlock*>> cbs;
}

namespace Memory{
  extern int num_bank;
  extern vector<MemBank> membanks;
}

//extern vector<MacroNodeTemplate*> mn_temps;

extern Parameters opti_para;

extern MemoryTrack* global_sp;

extern DRAMSim::MultiChannelMemorySystem* dram;

extern map<string, DataArray*> data_arrays;

extern string app;

extern map<string, int> fus;

#endif
