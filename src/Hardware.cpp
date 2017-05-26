#include<iostream>
#include<string>
#include<vector>
#include<array>
#include<map>
#include<list>
#include<deque>

#include "Hardware.hpp"

using namespace std;

ComputeBlock* iniCBWrapper(string name, int num, int stage, int width){
  if(name == "load"){
    return new CB_Load(name, num);
  }
  else if(name == "store"){
    return new CB_Store(name, num);
  }
  else if(name == "submul"){
    return new CB_MulAcc(false,name,num,stage);
  }
  else if(name == "accmul"){
    return new CB_MulAcc(true,name,num,stage);
  }
  else if(name == "div"){
    return new CB_Div(name,num);
  }
  else if(name == "mul"){
    return new CB_Mul(name, num);
  }
  else if(name == "copy"){
    return new CB_Copy(name,num);
  }
  else if(name == "qr_householder_accmul"){
    return new CB_QRMulAcc(name,width,num,stage);
  }
  else if(name == "qr_householder_para"){
    return new CB_QRHouseholderPara(name,num);
  }
}

CB_Load::CB_Load(string in_name, int in_num)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::LOAD;
  num = in_num;
  stage = 1;
  latency = FunctionUnitLib::LOAD.latency+1;

}

CB_Store::CB_Store(string in_name, int in_num)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::STORE;
  num = in_num;
  stage = 1;
  latency = FunctionUnitLib::STORE.latency+1;
}

CB_Copy::CB_Copy(string in_name, int in_num){
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::COPY;
  num = in_num;
  stage = 1;
  latency = FunctionUnitLib::COPY.latency;
}

CB_Div::CB_Div(string in_name, int in_num)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::DIV;
  num = in_num;
  stage = 1;
  latency = FunctionUnitLib::DIV.latency+2;
}

CB_Mul::CB_Mul(string in_name, int in_num){
  name = in_name;
  //cb_unit = &ComputeBlockUnitLib::MUL;
  num = in_num;
  stage = 1;
  latency = FunctionUnitLib::MUL.latency+2;
}

CB_DivRoot::CB_DivRoot(string in_name, int in_num)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::DIV_ROOT;
  num = in_num;
  stage = 1;
  latency = FunctionUnitLib::DIV.latency+FunctionUnitLib::ROOT.latency+2;
}

CB_MulAcc::CB_MulAcc(bool in_is_add, string in_name, int in_num, int in_stage)
{
  is_add = in_is_add;
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::MUL_ADD;
  num = in_num;
  stage = in_stage;
  latency = (FunctionUnitLib::MUL.latency+FunctionUnitLib::ADD.latency)+(stage-1)*FunctionUnitLib::ADD.latency;
  latency_rw_c = stage*FunctionUnitLib::ADD.latency;
  initial_latency = FunctionUnitLib::MUL.latency;
}

void CB_MulAcc::updateStage(int in_stage){
  stage = in_stage;
  latency = (FunctionUnitLib::MUL.latency+FunctionUnitLib::ADD.latency)+(stage-1)*FunctionUnitLib::ADD.latency;
  latency_rw_c = stage*FunctionUnitLib::MUL.latency;
}

int CB_MulAcc::LiveinReadCycle(int livein_idx){
  int read_cycle;
  if(livein_idx < 3){
    if(livein_idx%3 == 2){
      read_cycle = FunctionUnitLib::MUL.latency;
    }
    else{
      read_cycle = 0;
    }
  }
  else{
    read_cycle = ((livein_idx-3)/2+1)*FunctionUnitLib::ADD.latency;
  }
  return read_cycle;
}


CB_Acc::CB_Acc(string in_name, int in_num, int in_stage)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::ACC;
  num = in_num;
  stage = in_stage;
  latency = stage*FunctionUnitLib::ADD.latency;
}

void CB_Acc::updateStage(int in_stage){
  stage = in_stage;
  latency = 2+stage*FunctionUnitLib::ADD.latency;
}

int CB_Acc::LiveinReadCycle(int livein_idx){
  int read_cycle = 0;
  if(livein_idx < 2){
    read_cycle = 0;
  }
  else{
    read_cycle = ((livein_idx-2)+1)*FunctionUnitLib::ADD.latency;
  }
  return read_cycle;
}

CB_QRMulAcc::CB_QRMulAcc(string in_name, int in_width, int in_num, int in_stage){
  name = in_name;
  width = in_width;
  num = in_num;
  stage = in_stage;
  //only consider one stage case
  latency = 2+2*FunctionUnitLib::MUL.latency+2*FunctionUnitLib::ADD.latency;
  latency_rw_c = 2;
}

int CB_QRMulAcc::LiveinReadCycle(int livein_idx){
  int read_cycle = 0;
  if(livein_idx < 2){
    read_cycle = 0;
  }
  else if((livein_idx-2)%2 == 0){
    read_cycle = FunctionUnitLib::MUL.latency+FunctionUnitLib::ADD.latency;
  }
  else{
    read_cycle = 2*FunctionUnitLib::MUL.latency+FunctionUnitLib::ADD.latency;
  }
  return read_cycle;
}

CB_QRHouseholderPara::CB_QRHouseholderPara(string in_name, int in_num){
  name = in_name;
  num = in_num;
  latency = 2+2*FunctionUnitLib::ROOT.latency+FunctionUnitLib::ADD.latency+max(FunctionUnitLib::DIV.latency,FunctionUnitLib::MUL.latency);
}

int CB_QRHouseholderPara::LiveinReadCycle(int livein_idx){
  return 0;
}

CB_Root::CB_Root(string in_name, int in_num)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::ROOT;
  num = in_num;
  stage = 1;
  latency = 2+FunctionUnitLib::ROOT.latency;

}

CB_JacobiS::CB_JacobiS(string in_name, int in_num)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::JACOBI_CS;
  num = in_num;
  stage = 1;
  latency = 10;
}

CB_GivensS::CB_GivensS(string in_name, int in_num)
{
  name = in_name;
  cb_unit = &ComputeBlockUnitLib::GIVENS_CS;
  num = in_num;
  stage = 1;
  latency = 10;
}



