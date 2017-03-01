#include<iostream>
#include<fstream>
#include<algorithm>
#include<math.h>
#include<queue>
#include<list>
#include<vector>
#include<set>
#include<array>
#include<string>
#include<chrono>
#include<memory>
#include<climits>

#include "Hardware.hpp"
#include "Scheduling.hpp"
#include "Setting.hpp"

using namespace std;


TileScheduling::TileScheduling(MacroNodeTemplate &in_macro, int in_mode):mode(in_mode), macro(in_macro){
}

CBEvents::CBEvents(int in_num_stage){
  num_stage = in_num_stage;

  for(int i=0; i<num_stage; i++){
    StageEvents st;
    stage_events.push_back(st);
  }
}


void CBEvents::setStages(int in_num_stage){
  num_stage = in_num_stage;

  for(int i=0; i<num_stage; i++){
    StageEvents st;
    stage_events.push_back(st);
  }
}

void StageEvents::Enqueue(set<int> &ops){
  q_ops.push(ops);
}

void StageEvents::Dequeue(){
  ops = q_ops.front();
  q_ops.pop();
}


int TileScheduling::Executing_cbs(){
  bool finished = false;
  int cycle = -1;
  while(!finished){
    cycle++;
    cout << endl << "cycle " << cycle << endl;

    vector<bool> finish_stamp(cb_events.size(), false);
    for(int i=0; i<cb_events.size(); i++){
      cout << endl << "cb " << i << endl;
      for(int j=cb_events[i].num_stage-1; j>=0; j--){
        //take new event
	if(i == 0){
	  cout << "stage " << j << endl;
	}
	cb_events[i].stage_events[j].stall = (!cb_events[i].stage_events[j].ops.empty());
	if((j<(cb_events[i].num_stage-1)) && cb_events[i].stage_events[j+1].stall){
	  cb_events[i].stage_events[j].stall = true;
	}
	if(!cb_events[i].stage_events[j].stall){
	  if(j==0){
	    cb_events[i].stage_events[j].produced_out = false;
	    if(!cb_events[i].stage_events[j].q_ops.empty()){
	      cb_events[i].stage_events[j].Dequeue();
	    }
	  }
	  else{ 
	    cb_events[i].stage_events[j].produced_out = cb_events[i].stage_events[j-1].produced_out;
	    if(cb_events[i].stage_events[j-1].produced_out){
	      if(!cb_events[i].stage_events[j].q_ops.empty()){
		cb_events[i].stage_events[j].Dequeue();
		cb_events[i].stage_events[j].produced_out = false;
	      }
	    }
	  }
	}

	//read / write ops
	vector<int> finished_ops;
	for(auto op=cb_events[i].stage_events[j].ops.begin(); op!=cb_events[i].stage_events[j].ops.end(); op++){
	  if(i == 0){
	    cout << "op " << *op << endl;
	  }
	  int bank = macro.ioop_addr[*op][0];
	  macro.mem->ExtendLength(cycle+1);
	  if(op_read_cycles[*op].find(cycle) == op_read_cycles[*op].end()){
	    if(macro.mem->CanPortInc(bank, cycle)){
	      //update op read cycles
	      if(j != (cb_events[i].num_stage-1)){
		op_read_cycles[*op].insert(cycle);
	      }
	      macro.mem->PortInc(bank, cycle);
	      finished_ops.push_back(*op);
	    }
	  }else{
	    finished_ops.push_back(*op);
	  }
	}
	for(auto &op: finished_ops){
	  cb_events[i].stage_events[j].ops.erase(op);
	}

	if(i == 0){
	  cout << "op left" << endl;
	  for(auto op=cb_events[i].stage_events[j].ops.begin(); op!=cb_events[i].stage_events[j].ops.end(); op++){
	    cout << "op " << *op << endl;
	  }
	}

	if(!finished_ops.empty() && cb_events[i].stage_events[j].ops.empty()){
	  cb_events[i].stage_events[j].produced_out = true;
	}

	if(j == (cb_events[i].num_stage-1)){
	  if(cb_events[i].stage_events[j].ops.empty() && cb_events[i].stage_events[j].q_ops.empty()){
	    finish_stamp[i] = true;
	  }
	}
      }
      cout << "finish this cb" << endl;

    }

    cout << "finish this cycle " << endl;
    finished = true;
    for(int i=0; i<cb_events.size(); i++){
      finished = finished && finish_stamp[i];
    }
  }
  return cycle;
}


void TileScheduling::Scheduling_pipe(){
  //initialize op_read_cycles
  for(auto t=macro.tiles.begin(); t!=macro.tiles.end(); t++){
    for(auto op=t->livein_ops.begin(); op!=t->livein_ops.end(); op++){
      set<int> t;
      op_read_cycles[op->first] = t;
    }
  }

  //int num_cbs_used = subblk_dimi_tile*subblk_dimj_tile;
  for(int i=0; i<opti_para.num_cb; i++){
    CBEvents ce(ComputeBlockLib::cbs["mul_acc"]->latency);
    cb_events.push_back(ce);
  }

  subblk_m = opti_para.blk_dimi/opti_para.subblk_dimi;
  subblk_n = opti_para.blk_dimj/opti_para.subblk_dimj;
  subblk_k = opti_para.blk_diml/opti_para.subblk_diml;

  for(auto t=macro.tiles.begin(); t!=macro.tiles.end(); t++){
    int tile = t-macro.tiles.begin();
    for(auto op=macro.tiles[tile].liveout_ops.begin(); op!=macro.tiles[tile].liveout_ops.end(); op++){
      if(macro.ioop_addr.find(*op) == macro.ioop_addr.end()){
	int num_cout = macro.Cout.size() * macro.Cout[0].size();
	int c_idx = tile%num_cout;
	int cout_op = macro.Cout[c_idx/macro.Cout[0].size()][c_idx%macro.Cout[0].size()];
	macro.ioop_addr[*op] = macro.ioop_addr[cout_op];
      }
    }
  }

  for(int subblk_l=0; subblk_l<subblk_k; subblk_l++){
    for(int subblk_i=0; subblk_i<subblk_m; subblk_i++){
      for(int subblk_j=0; subblk_j<subblk_n; subblk_j++){
	cout << endl;
	cout << "subblk i " << subblk_i << endl;
	cout << "subblk j " << subblk_j << endl;
	cout << "subblk l " << subblk_l << endl;

        //read out all input data
	//allocate all cbs
	int cb_idx = 0;
	for(int i=(subblk_i)*opti_para.subblk_dimi; i<(subblk_i+1)*opti_para.subblk_dimi; i++){
	  for(int j=(subblk_j)*opti_para.subblk_dimj; j<(subblk_j+1)*opti_para.subblk_dimj; j++){
	      //tile idx
	      int tile = i*(subblk_k*opti_para.blk_dimj)+j*subblk_k+subblk_l;
	      cout << "tile " << tile << endl;

	      //cb to use cb_idx
	      map<int, set<int>> sorted_livein_ops;
	      for(auto &i: macro.tiles[tile].livein_ops){
	        if(sorted_livein_ops.find(i.second) == sorted_livein_ops.end()){
	          set<int> s = {i.first};
	          sorted_livein_ops[i.second] = s;
	        }
	        else{
	          if(sorted_livein_ops[i.second].find(i.first) == sorted_livein_ops[i.second].end()){
	            sorted_livein_ops[i.second].insert(i.first);
	          }
	        }
	      }
	      
	      for(auto &livein: sorted_livein_ops){
		cout << livein.first << ":";
		for(auto &i: livein.second){
		  cout << i << ",";
		}
		cout << endl;
	        cb_events[cb_idx].stage_events[livein.first].Enqueue(livein.second);
	      }
		
	      set<int> liveout_ops;
	      for(auto &op: macro.tiles[tile].liveout_ops){
	        liveout_ops.insert(op);
	      }
	      cb_events[cb_idx].stage_events.rbegin()->Enqueue(liveout_ops);

	      cb_idx++;
	  }
	}
      }
    }
  }

  //executing cbs
  last_cycle = Executing_cbs();
  cout << "Last Cycle of mn template scheduling " << last_cycle << endl;
  macro.cycle_length = last_cycle+1;
}




