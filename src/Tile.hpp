#ifndef TILE
#define TILE

#include<vector>
#include<map>

#include "Util.hpp"
#include "Hardware.hpp"

using namespace std;

class Tile{
public:
	Tile(ComputeBlock* in_cb){cb=in_cb; mn=-1;};
	~Tile(){};

	ComputeBlock* cb;
	string pattern_name;
	string cpsection_name;

	int latency;
	vector<int> num_read;

	int mn;

	vector<int> ops;

	map<int, int> livein_ops;
	   //key: livein op, value: cycle being read
	vector<int> pred_tiles;
	vector<int> liveout_ops;
	map<int, vector<int>> post_tiles;
};

#endif
