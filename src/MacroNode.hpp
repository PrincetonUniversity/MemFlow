#ifndef MACRONODE
#define MACRONODE

#include<vector>
#include<string>
#include<queue>

#include "DataBlock.hpp"
#include "Tile.hpp"
#include "MacroNodeTemplate.hpp"
#include "Memory.hpp"
#include "DRAMAccess.hpp"

using namespace std;

class LoadStoreDblk{
  public:
    LoadStoreDblk();
    ~LoadStoreDblk();
    void genTransactions(DataBlock* dblk, queue<uint64_t>& trans_addr);
    int loadDblk(DataBlock* dblk);
    int storeDblk(DataBlock* dblk);
    void setIO();
    void freeIO();

    Callback_t* read_cb;
    Callback_t* write_cb;
    TransactionReceiver transactionReceiver;

    string matrix_name;
    int blk_i;
    int blk_j;

    DblkAddr dblk_addr;
};

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

    //set<int> pred_mns;
    //set<int> post_mns;

    vector<int> pred_dblks;
    vector<int> post_dblks;

    string name;
    int idx;

    DblkAddr a_sp_addr;
    DblkAddr b_sp_addr;
    DblkAddr c_sp_addr;

};

#endif
