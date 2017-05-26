#ifndef TILE
#define TILE

#include<vector>
#include<map>

#include "Util.hpp"
#include "Hardware.hpp"

using namespace std;

struct rw_info{
  int cycle;
  array<int,2> addr;
};

class Tile{
public:
	Tile(ComputeBlock* in_cb){cb=in_cb; mn=-1;};
	~Tile(){};

	ComputeBlock* cb;
	string pattern_name;
	string mn_name;

	int latency;
	vector<int> num_read;

	int mn;

	vector<int> ops;

	map<int, int> livein_ops;
	   //key: livein op, value: cycle being read
	vector<int> pred_tiles;
	vector<int> liveout_ops;
	map<int, vector<int>> post_tiles;

	//map<int,rw_info> inop;
	//map<int,rw_info> outop;

	int tile_sche_idx;
};

#endif
