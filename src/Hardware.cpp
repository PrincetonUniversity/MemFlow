#include<iostream>
#include<string>
#include<vector>
#include<array>
#include<map>
#include<list>
#include<deque>

#include "Hardware.hpp"

using namespace std;

CB_Load::CB_Load(string in_name, int in_idx, int in_width)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::LOAD;
  width = in_width;
  max_depth = 1;
  latency = FunctionUnitLib::LOAD.latency+1;

}

CB_Store::CB_Store(string in_name, int in_idx, int in_width)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::STORE;
  width = in_width;
  max_depth = 1;
  latency = FunctionUnitLib::STORE.latency+1;
}

CB_Sub::CB_Sub(string in_name, int in_idx, int in_width)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::SUB;
  width = in_width;
  max_depth = 1;
  latency = FunctionUnitLib::SUB.latency+2;
}

CB_Div::CB_Div(string in_name, int in_idx, int in_width)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::DIV;
  width = in_width;
  max_depth = 1;
  latency = FunctionUnitLib::DIV.latency+2;
}

CB_DivRoot::CB_DivRoot(string in_name, int in_idx, int in_width)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::DIV_ROOT;
  width = in_width;
  max_depth = 1;
  latency = FunctionUnitLib::DIV.latency+FunctionUnitLib::ROOT.latency+2;
}


CB_MulAcc::CB_MulAcc(string in_name, int in_idx, int in_width, int in_depth)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::MUL_ADD;
  width = in_width;
  max_depth = in_depth;
  latency = 2+(FunctionUnitLib::MUL.latency+FunctionUnitLib::ADD.latency)+(in_depth-1)*FunctionUnitLib::ADD.latency;
}

int CB_MulAcc::GetRealLatency(int in_realdepth){
  return 2+(FunctionUnitLib::MUL.latency+FunctionUnitLib::ADD.latency)+(in_realdepth-1)*FunctionUnitLib::ADD.latency;
}

int CB_MulAcc::LiveinReadCycle(int livein_idx){
  int read_cycle;
  if(livein_idx < 3*width){
    if(livein_idx%3 == 2){
      read_cycle = FunctionUnitLib::MUL.latency;
    }
    else{
      read_cycle = 0;
    }
  }
  else{
    read_cycle = ((livein_idx-3*width)/(2*width)+1)*FunctionUnitLib::ADD.latency;
  }
  return read_cycle;
}


CB_Acc::CB_Acc(string in_name, int in_idx, int in_width, int in_depth)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::ACC;
  width = in_width;
  max_depth = in_depth;

  latency = in_depth*FunctionUnitLib::ADD.latency;
}

int CB_Acc::GetRealLatency(int in_realdepth){
  return 2+in_realdepth*FunctionUnitLib::ADD.latency;
}

int CB_Acc::LiveinReadCycle(int livein_idx){
  int read_cycle = 0;
  if(livein_idx < 2*width){
    read_cycle = 0;
  }
  else{
    read_cycle = ((livein_idx-2*width)/width+1)*FunctionUnitLib::ADD.latency;
  }
  return read_cycle;
}

CB_Root::CB_Root(string in_name, int in_idx, int in_width)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::ROOT;
  width = in_width;
  max_depth = 1;
  latency = 2+FunctionUnitLib::ROOT.latency;

}

CB_JacobiS::CB_JacobiS(string in_name, int in_idx)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::JACOBI_CS;
  width = 1;
  max_depth = 1;
  latency = 10;

}

CB_GivensS::CB_GivensS(string in_name, int in_idx)
{
  name = in_name;
  idx = in_idx;
  cb_unit = &ComputeBlockUnitLib::GIVENS_CS;
  width = 1;
  max_depth = 1;
  latency = 10;

}



