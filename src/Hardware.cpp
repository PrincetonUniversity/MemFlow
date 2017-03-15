#include<iostream>
#include<string>
#include<vector>
#include<array>
#include<map>
#include<list>
#include<deque>

#include "Hardware.hpp"

using namespace std;

CB_Load::CB_Load(string in_name, int in_width)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::LOAD;
  width = in_width;
  stage = 1;
  latency = FunctionUnitLib::LOAD.latency+1;

}

CB_Store::CB_Store(string in_name, int in_width)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::STORE;
  width = in_width;
  stage = 1;
  latency = FunctionUnitLib::STORE.latency+1;
}

CB_Copy::CB_Copy(string in_name, int in_width){
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::COPY;
  width = in_width;
  stage = 1;
  latency = FunctionUnitLib::COPY.latency;
}

CB_Sub::CB_Sub(string in_name, int in_width)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::SUB;
  width = in_width;
  stage = 1;
  latency = FunctionUnitLib::SUB.latency+2;
}

CB_Div::CB_Div(string in_name, int in_width)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::DIV;
  width = in_width;
  stage = 1;
  latency = FunctionUnitLib::DIV.latency+2;
}

CB_DivRoot::CB_DivRoot(string in_name, int in_width)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::DIV_ROOT;
  width = in_width;
  stage = 1;
  latency = FunctionUnitLib::DIV.latency+FunctionUnitLib::ROOT.latency+2;
}

CB_MulAcc::CB_MulAcc(bool in_is_add, string in_name, int in_width, int in_stage)
{
  is_add = in_is_add;
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::MUL_ADD;
  width = in_width;
  stage = in_stage;
  latency = 2+(FunctionUnitLib::MUL.latency+FunctionUnitLib::ADD.latency)+(stage-1)*FunctionUnitLib::ADD.latency;
  latency_rw_c = latency-1-FunctionUnitLib::MUL.latency;
}

void CB_MulAcc::updateStage(int in_stage){
  stage = in_stage;
  latency = 2+(FunctionUnitLib::MUL.latency+FunctionUnitLib::ADD.latency)+(stage-1)*FunctionUnitLib::ADD.latency;
  latency_rw_c = latency-1-FunctionUnitLib::MUL.latency;
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


CB_Acc::CB_Acc(string in_name, int in_width, int in_stage)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::ACC;
  width = in_width;
  stage = in_stage;
  latency = stage*FunctionUnitLib::ADD.latency;
}

void CB_Acc::updateStage(int in_stage){
  stage = in_stage;
  latency = 2+stage*FunctionUnitLib::ADD.latency;
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

CB_Root::CB_Root(string in_name, int in_width)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::ROOT;
  width = in_width;
  stage = 1;
  latency = 2+FunctionUnitLib::ROOT.latency;

}

CB_JacobiS::CB_JacobiS(string in_name)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::JACOBI_CS;
  width = 1;
  stage = 1;
  latency = 10;

}

CB_GivensS::CB_GivensS(string in_name)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::GIVENS_CS;
  width = 1;
  stage = 1;
  latency = 10;

}



