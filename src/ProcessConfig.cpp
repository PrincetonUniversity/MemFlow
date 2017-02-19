#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>

#include "ProcessConfig.hpp"

using namespace std;

void readConfig(){

  ifstream config ("config_hw");
  if(config.is_open()){
    
    string line;
    
    /*
    string start_cb = "Compute blocks";

    getline(config,line);
    while(line.compare(0, start_cb.size(), start_cb)){
      getline(config,line);
    }

    getline(config,line);
    int idx = 1;
    while(!line.empty()){
      //line stores cb info
      stringstream ss(line);

      string name;
      getline(ss, name, ',');

      string token;
      getline(ss, token, ',');
      int depth = atoi(token.c_str());

      getline(ss, token, ',');
      int width = atoi(token.c_str());

      getline(ss, token, ',');
      int num = atoi(token.c_str());

      if(name == "load")
	ComputeBlockLib::cbs[name] = new CB_Load("load", idx, width); 
      else if(name == "store")
	ComputeBlockLib::cbs[name] = new CB_Store("store", idx, width);
      else if(name == "sub")
	ComputeBlockLib::cbs[name] = new CB_Sub("sub", idx, width);
      else if(name == "div")
	ComputeBlockLib::cbs[name] = new CB_Div("div", idx, width);
      else if(name == "div_root")
	ComputeBlockLib::cbs[name] = new CB_DivRoot("div_root", idx, width);
      else if(name == "mul_acc")
	ComputeBlockLib::cbs[name] = new CB_MulAcc("mul_acc", idx, width, depth);
      else if(name == "acc")
	ComputeBlockLib::cbs[name] = new CB_Acc("acc", idx, width, depth);
      else if(name == "root_s")
	ComputeBlockLib::cbs[name] = new CB_Root("root_s", idx, width);
      else if(name == "div_s")
	ComputeBlockLib::cbs[name] = new CB_Div("div_s", idx, width);
      else if(name == "jacobi_s")
	ComputeBlockLib::cbs[name] = new CB_JacobiS("jacobi_s", idx);
      else if(name == "givens_s")
	ComputeBlockLib::cbs[name] = new CB_GivensS("givens_s", idx);

      ComputeBlockLib::num_cb[name] = num;

      getline(config,line);
      idx++;
    }
    */


    getline(config, line);
    string start_mem = "Memory";

    while(line.compare(0, start_mem.size(), start_mem)){
      getline(config,line);
    }

    getline(config, line);
    int count = 0;
    while(!line.empty()){
      stringstream ss(line);

      string token;
      getline(ss, token, ',');
      int size = atoi(token.c_str());

      getline(ss, token, ',');
      int port = atoi(token.c_str());

      MemBank m = {size, port};
      Memory::membanks.push_back(m);

      getline(config, line);
      count++; 
    }
    Memory::num_bank = count;
  }
  else cout << "Unable to open file config_hw" << endl;
}

void freeConfig(){
  for(auto it=mn_temps.begin(); it!=mn_temps.end(); it++){
    delete *it;
  }

  for(auto it=ComputeBlockLib::cbs.begin(); it!=ComputeBlockLib::cbs.end(); it++){
    delete it->second;
  }

}
