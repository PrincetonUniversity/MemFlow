#ifndef UTIL
#define UTIL

#include <vector>
#include <set>

using namespace std;
/*
namespace ComputeBlockTopo
{
	string REC = "rectangle";
	string BINTREE = "binary tree";
}
*/

struct FunctionUnit{
	string name;
	int latency;
};

namespace FunctionUnitLib
{
	const struct FunctionUnit LOAD = {"load", 4};
	const struct FunctionUnit STORE = {"store", 4};
	const struct FunctionUnit ADD = {"add", 1};
	const struct FunctionUnit SUB = {"sub", 1};
	const struct FunctionUnit MUL = {"mul", 3};
	const struct FunctionUnit DIV = {"divide", 3};
	const struct FunctionUnit ROOT = {"root", 3};
};

struct ComputeBlockUnit{
	string name;
	vector<FunctionUnit> fus;
};

namespace ComputeBlockUnitLib{
	const struct ComputeBlockUnit LOAD = {"load", {FunctionUnitLib::LOAD}};
	const struct ComputeBlockUnit STORE = {"store", {FunctionUnitLib::STORE}};
	const struct ComputeBlockUnit SUB = {"sub", {FunctionUnitLib::SUB}};
	const struct ComputeBlockUnit DIV = {"div", {FunctionUnitLib::DIV}};
	const struct ComputeBlockUnit DIV_ROOT = {"div_root", {FunctionUnitLib::DIV, FunctionUnitLib::ROOT}};
	const struct ComputeBlockUnit MUL_ADD = {"mul_add", {FunctionUnitLib::MUL, FunctionUnitLib::ADD}};
	const struct ComputeBlockUnit ACC = {"acc", {FunctionUnitLib::ADD}};
	//scalar
	const struct ComputeBlockUnit ROOT = {"root", {FunctionUnitLib::ROOT}};
	const struct ComputeBlockUnit JACOBI_CS = {"jacobi_cs", {FunctionUnitLib::SUB, FunctionUnitLib::DIV, FunctionUnitLib::MUL, FunctionUnitLib::ADD, FunctionUnitLib::ADD, FunctionUnitLib::ROOT, FunctionUnitLib::DIV, FunctionUnitLib::MUL}};
	const struct ComputeBlockUnit GIVENS_CS = {"givens_cs", {FunctionUnitLib::MUL, FunctionUnitLib::MUL, FunctionUnitLib::ADD, FunctionUnitLib::ROOT, FunctionUnitLib::DIV, FunctionUnitLib::DIV}};
};

/*
struct ComputeBlock{
	int idx;
	ComputeBlockUnit cb_unit;
	int width;
	int depth;
	int latency;
};

namespace ComputeBlockLib{
	const struct ComputeBlock LOAD = {0, ComputeBlockUnitLib::LOAD, 4, 1, 5};
	const struct ComputeBlock STORE = {1, ComputeBlockUnitLib::STORE, 4, 1, 5};
	const struct ComputeBlock SUB = {2, ComputeBlockUnitLib::SUB, 4, 1, 1};
	const struct ComputeBlock DIV = {3, ComputeBlockUnitLib::DIV, 4, 1, 2};
	const struct ComputeBlock DIV_ROOT = {4, ComputeBlockUnitLib::DIV_ROOT, 2, 1, 4};
	const struct ComputeBlock MUL_ACC = {5, ComputeBlockUnitLib::MUL_ADD, 2, 2, 4};
	const struct ComputeBlock ACC = {6, ComputeBlockUnitLib::ACC, 2, 2, 2};
	const struct ComputeBlock ROOT_S = {7, ComputeBlockUnitLib::ROOT, 1, 1, 2};
	const struct ComputeBlock JACOBI_S = {8, ComputeBlockUnitLib::JACOBI_CS, 1, 1, 10};
	const struct ComputeBlock GIVENS_S = {9, ComputeBlockUnitLib::GIVENS_CS, 1, 1, 10};
	const struct ComputeBlock DIV_S = {10, ComputeBlockUnitLib::DIV, 1, 1, 2};
}
*/

struct Operation{
	int idx;
	vector<int> in;
	set<int> out;
	FunctionUnit fu;
	int tile = -1;
};


enum DblkSPAddrIdx{
  ZERO_IDX,
  ROW_IDX,
  COL_IDX,
  ROWMAJOR_IDX,
  COLMAJOR_IDX
};

#endif
