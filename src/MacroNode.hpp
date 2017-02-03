#ifndef MACRONODE
#define MACRONODE

#include<vector>
#include<string>

#include "Tile.hpp"
#include "MacroNodeTemplate.hpp"

using namespace std;

class MacroNode{
  public:
    MacroNode(MacroNodeTemplate* in_mn_temp);
    MacroNode(MacroNodeTemplate* in_mn_temp, int in_idx);
    MacroNode(int in_idx);
    ~MacroNode(){};

    void BuildOpMap(vector<Tile> &real_tiles, vector<Operation> &real_ops);

    MacroNodeTemplate* mn_temp;

    map<int, int> tile_map;
    //key: real tile idx; value: tile idx in mn
    map<int, int> op_map;
    //key: op idx in mn; value: real op idx

    set<int> pred_mns;
    set<int> post_mns;

    vector<int> pred_dblks;
    vector<int> post_dblks;

    string name;
    int idx;

};

#endif
