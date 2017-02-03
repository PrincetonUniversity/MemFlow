#include<iostream>
#include<vector>

#include "MacroNode.hpp"

using namespace std;

MacroNode::MacroNode(MacroNodeTemplate* in_mn_temp):mn_temp(in_mn_temp){
  name = mn_temp->name;
}

MacroNode::MacroNode(MacroNodeTemplate* in_mn_temp, int in_idx):mn_temp(in_mn_temp), idx(in_idx){
  name = mn_temp->name;
}

MacroNode::MacroNode(int in_idx):idx(in_idx){
}

void MacroNode::BuildOpMap(vector<Tile> &real_tiles, vector<Operation> &real_ops){
  //cout << endl << "mn " << idx << endl;
  for(map<int,int>::iterator t=tile_map.begin(); t!=tile_map.end(); t++){
    //cout << "real tile " << t->first << ": " << " tile in mn " << t->second << endl;
    vector<int>::iterator op2 = mn_temp->tiles[t->second].ops.begin();
    for(vector<int>::iterator op1 = real_tiles[t->first].ops.begin(); op1!=real_tiles[t->first].ops.end(); op1++){
      //cout << "op in real tile " << *op1 << endl;
      //cout << "op in mn tile " << *op2 << endl;
      vector<int>::iterator inop2 = mn_temp->ops[*op2].in.begin();
      for(vector<int>::iterator inop1 = real_ops[*op1].in.begin(); inop1!=real_ops[*op1].in.end(); inop1++){
	//cout <<"inop in real tile " << *inop1 << endl;
	//cout <<"inop in mn tile " << *inop2 << endl;
	if(real_tiles[t->first].livein_ops.find(*inop1) != real_tiles[t->first].livein_ops.end()){
	  op_map[*inop2] = *inop1;
	}
	inop2++;
      }
      op2++;
    }

    vector<int>::iterator out_op1 = real_tiles[t->first].liveout_ops.begin();
    for(vector<int>::iterator out_op2= mn_temp->tiles[t->second].liveout_ops.begin(); out_op2!= mn_temp->tiles[t->second].liveout_ops.end(); out_op2++){
      if(op_map.find(*out_op2) == op_map.end()){
	op_map[*out_op2] = *out_op1;
      }
      out_op1++;
    }
  }
}


