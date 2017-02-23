#include <vector>

#include "DataBlock.hpp"

using namespace std;

DataBlock::DataBlock(int in_idx, string in_name, int in_m, int in_n, int in_basei, int in_basej){
  idx = in_idx;
  matrix_name = in_name;
  size_m = in_m;
  size_n = in_n;
  base_i = in_basei;
  base_j = in_basej;
}
