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


TileScheduling::TileScheduling(MacroNodeTemplate* in_macro, int in_mode):mode(in_mode), macro(in_macro){
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

void StageEvents::Enqueue(set<event> &ops){
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
    
    finished = true;
    for(auto &cb_ctg: cb_events){
      for(auto cb_it=cb_ctg.second.begin(); cb_it!=cb_ctg.second.end(); cb_it++){
	int cb_idx = cb_it-cb_ctg.second.begin();
	CBEvents& cb = *cb_it;
	//cb_event: cb
	//cout << endl << "cb " << cb_ctg.first << endl;

	for(int j=cb.num_stage-1; j>=0; j--){
	  if(cb_idx == 0){
	    cout << "stage " << j << endl;
	  }
	  
	  //stage j
	  //reason 1 to stall: still read or write
	  cb.stage_events[j].stall = (!cb.stage_events[j].ops.empty());
	  if(cb.stage_events[j].stall){
	    if(cb_idx == 0){
	      cout << "  stage " << j << " is stalled" << endl;
	    }
	  }

	  //reason 2 to stall: next stage is stalled
	  if((j<(cb.num_stage-1)) && (cb.stage_events[j+1].stall)){
	    cb.stage_events[j].stall = true;
	  }
	  if(cb.stage_events[j].stall){
	    if(cb_idx == 0){
	      cout << "  stage " << j << " is stalled" << endl;
	    }
	  }

	  //cout << "ops on queue front " << endl;
	  //if(!cb.stage_events[j].q_ops.empty()){
	    //for(auto op=cb.stage_events[j].q_ops.front().begin(); op!=cb.stage_events[j].q_ops.front().end(); op++){
	      //cout << op->op << ",";
	    //}
	    //cout << endl;
	  //}

	  //reason 3 to stall: data is not ready
	  if(!cb.stage_events[j].stall){
	    if(!cb.stage_events[j].q_ops.empty()){
	      set<event> ops_candi = cb.stage_events[j].q_ops.front();
	      for(auto &e: ops_candi){
	        if(cb_idx == 0){
		  cout << "candi op " << e.op << endl;
		}
		if((e.r_or_w == 'r') && !macro->p_cg->ops[e.op].has_produced){
		  if(cb_idx == 0){
		    cout << "is not produced " << endl;
		  }
		  cb.stage_events[j].stall = true;
		  cb.stage_events[j].produced_out = false;
		  if(cb_idx == 0){
		    cout << "  stage " << j << " is stalled " << endl;
		  }
		  //cout << "has dep op not produced yet" << endl;
		  break;
		}
	      }
	    }
	  }

	  //not stall: fetch new data
	  if(!cb.stage_events[j].stall){
	    if(j == 0){
	        if(!cb.stage_events[j].q_ops.empty()){
		  cb.stage_events[j].Dequeue();
		  cb.stage_events[j].produced_out = false;
		}
	    }
	    else{
	      if(cb.stage_events[j-1].produced_out){
	        if(!cb.stage_events[j].q_ops.empty()){
		  cb.stage_events[j].Dequeue();
		  cb.stage_events[j].produced_out = false;
		}
		else{
		  cb.stage_events[j].produced_out = true;
		}
	      }
	      else{
	        cb.stage_events[j].produced_out = false;
	      }
	    }
	  }

	  //cout << "ops: ";
	  //for(auto op=cb.stage_events[j].ops.begin(); op!=cb.stage_events[j].ops.end(); op++){
	    //cout << op->op << ",";
	  //}
	  //cout << endl;

	  vector<event> finished_events;
	  for(auto e=cb.stage_events[j].ops.begin(); e!=cb.stage_events[j].ops.end(); e++){
	    int bank = macro->p_cg->ops[e->op].sp_addr[0];
	    macro->mem->ExtendLength(cycle+1);
	    //cout << "extend mem length " << endl;
	    if(op_read_cycles[e->op].find(cycle) == op_read_cycles[e->op].end()){
	      if(macro->mem->CanPortInc(bank,cycle)){
	        if(e->r_or_w == 'r'){
		  op_read_cycles[e->op].insert(cycle);
		}
		macro->mem->PortInc(bank,cycle);
		finished_events.push_back(*e);
	      }
	    }
	    else{
	      finished_events.push_back(*e);
	    }
	  }

	  for(auto &e: finished_events){
	    //cout << "finished op " << e.op << endl;
	    if(e.r_or_w == 'w'){
	      macro->p_cg->ops[e.op].has_produced = true;
	    }
	    cb.stage_events[j].ops.erase(e);
	  }

	  if(!finished_events.empty() && cb.stage_events[j].ops.empty()){
	    cb.stage_events[j].produced_out = true;
	    //if(cb_idx == 0){
	      //cout << "produced out true " << endl;
	    //}
	  }

	  if(j == (cb.num_stage-1)){
	    if(cb.stage_events[j].ops.empty() && cb.stage_events[j].q_ops.empty()){
	      cb.finished = true;
	    }
	  }
	}
	finished = finished && cb.finished;
	//cout << "finish cb" << endl;
      }
    }
  }
  return cycle;
}

/*
void TileScheduling::Scheduling_pipe(){
  //initialize op_read_cycles
  for(auto t=macro->p_cg->tiles.begin(); t!=macro->p_cg->tiles.end(); t++){
    for(auto op=t->livein_ops.begin(); op!=t->livein_ops.end(); op++){
      set<int> t;
      op_read_cycles[op->first] = t;
    }
  }

  //int num_cbs_used = subblk_dimi_tile*subblk_dimj_tile;
  //cout << "mul acc latency: " << ComputeBlockLib::cbs["mul_acc"]->latency << endl;
  //cout << "num of cb: " << opti_para.num_cb << endl;
  for(int i=0; i<opti_para.num_cb["mul_acc"]; i++){
    CBEvents ce(ComputeBlockLib::cbs["mul_acc"]->latency);
    cb_events["mul_acc"].push_back(ce);
  }

  subblk_m = opti_para.blk_dim["i"]/opti_para.subblk_dim["i"];
  subblk_n = opti_para.blk_dim["j"]/opti_para.subblk_dim["j"];
  subblk_k = opti_para.blk_dim["l"]/opti_para.subblk_dim["l"];

  for(auto t=macro->p_cg->tiles.begin(); t!=macro->p_cg->tiles.end(); t++){
    int tile = t-macro->p_cg->tiles.begin();
    for(auto op=macro->p_cg->tiles[tile].liveout_ops.begin(); op!=macro->p_cg->tiles[tile].liveout_ops.end(); op++){
      if(macro->ioop_addr.find(*op) == macro->ioop_addr.end()){
	int num_cout = static_cast<MN_mtxmul*>(macro)->Cout.size() * static_cast<MN_mtxmul*>(macro)->Cout[0].size();
	int c_idx = tile%num_cout;
	int cout_op = static_cast<MN_mtxmul*>(macro)->Cout[c_idx/static_cast<MN_mtxmul*>(macro)->Cout[0].size()][c_idx%static_cast<MN_mtxmul*>(macro)->Cout[0].size()];
	macro->ioop_addr[*op] = macro->ioop_addr[cout_op];

	//intermediate ops
	//read need to be after write
	intermediate_ops[*op] = INT_MAX;
      }
    }
  }

  //cout << "add events to queue" << endl;
  for(int subblk_l=0; subblk_l<subblk_k; subblk_l++){
    for(int subblk_i=0; subblk_i<subblk_m; subblk_i++){
      for(int subblk_j=0; subblk_j<subblk_n; subblk_j++){
	//cout << endl;
	//cout << "subblk i " << subblk_i << endl;
	//cout << "subblk j " << subblk_j << endl;
	//cout << "subblk l " << subblk_l << endl;
	//cout << endl;
        //read out all input data
	//allocate all cbs
	int cb_idx = 0;
	for(int i=(subblk_i)*opti_para.subblk_dim["i"]; i<(subblk_i+1)*opti_para.subblk_dim["i"]; i++){
	  for(int j=(subblk_j)*opti_para.subblk_dim["j"]; j<(subblk_j+1)*opti_para.subblk_dim["j"]; j++){
	      //tile idx
	      int tile = subblk_l*(opti_para.blk_dim["i"]*opti_para.blk_dim["j"])+i*opti_para.blk_dim["j"]+j;

	      //cb to use cb_idx
	      map<int, set<int>> sorted_livein_ops;
	      for(auto &i: macro->p_cg->tiles[tile].livein_ops){
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
	        cb_events["mul_acc"][cb_idx].stage_events[livein.first].Enqueue(livein.second);
	      }
		
	      set<int> liveout_ops;
	      for(auto &op: macro->p_cg->tiles[tile].liveout_ops){
	        liveout_ops.insert(op);
	      }
	      cb_events["mul_acc"][cb_idx].stage_events.rbegin()->Enqueue(liveout_ops);

	      cb_idx++;
	  }
	}
      }
    }
  }

  //executing cbs
  last_cycle = Executing_cbs();
  macro->cycle_length = last_cycle+1;
  cout << "#cycle of mn template scheduling " << macro->cycle_length << endl;
}
*/


void TileScheduling::Scheduling_pipe_debug(){
  //initialize op_read_cycles
  for(auto t=macro->p_cg->tiles.begin(); t!=macro->p_cg->tiles.end(); t++){
    for(auto op=t->livein_ops.begin(); op!=t->livein_ops.end(); op++){
      set<int> t;
      op_read_cycles[op->first] = t;
    }
  }

  for(auto &cb: opti_para.cbs[macro->name]){
    for(int i=0; i<cb.second.num_cb; i++){
      CBEvents ce(ComputeBlockLib::cbs[macro->name][cb.first]->latency);
      cb_events[cb.first].push_back(ce);
    }
  }

  //sweep each pattern
  for(auto p=macro->p_cg->patterns.begin(); p!=macro->p_cg->patterns.end(); p++){
    //cb name: p->cb->name
    if(((*p)->name != "load") && ((*p)->name != "store")){
      int num_cb = opti_para.cbs[macro->name][(*p)->name].num_cb;
      int num_subblk = (*p)->tiles_idx.size()/num_cb;

      for(int i=0; i<num_subblk; i++){
	cout << endl << "subblk " << i << endl;
	for(int cb=0; cb<num_cb; cb++){
	  cout << endl << "cb " << cb << endl;
	  int tile_idx = i*num_cb+cb;
	  
	  if(tile_idx < (*p)->tiles_idx.size()){
	    int tile = (*p)->tiles_idx[tile_idx];
		
	    map<int, set<event>> sorted_ops;
	    for(auto &i: macro->p_cg->tiles[tile].livein_ops){
	      if(sorted_ops.find(i.second) == sorted_ops.end()){
		set<event> s = {event(i.first,'r')};
		sorted_ops[i.second] = s;
	      }
	      else{
		if(sorted_ops[i.second].find(event(i.first,'r')) == sorted_ops[i.second].end()){
		  sorted_ops[i.second].insert(event(i.first,'r'));
		}
	      }
	    }

	    for(auto &op: macro->p_cg->tiles[tile].liveout_ops){
	      sorted_ops[(*p)->cb->latency-1].insert(event(op, 'w'));
	    }

	    for(auto &e: sorted_ops){
	      cb_events[(*p)->name][cb].stage_events[e.first].Enqueue(e.second);
	      cout << "stage " << e.first << " ";
	      for(auto it=e.second.begin(); it!=e.second.end(); it++){
		cout << it->r_or_w << " " << it->op << " ";
	      }
	      cout << endl;
	    }

	  }
	}
      }
    }
  }
  //executing cbs
  last_cycle = Executing_cbs();
  macro->cycle_length = last_cycle+1;
  cout << "#cycle of mn template scheduling " << macro->cycle_length << endl;
}




