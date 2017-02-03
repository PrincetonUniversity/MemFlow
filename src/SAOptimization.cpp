#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string>

#include <vector>
#include <set>

#include "Python.h"

#include "Hardware.hpp"
#include "Scheduling.hpp"
#include "SAOptimization.hpp"

using namespace std;

/*
SAOptimizer::SAOptimizer(vector<Operation> &ops, vector<Tile>& tiles){
	sche = new Scheduling(ops, tiles);
	cout << "before init sche worst" << endl;
	sche->InitScheduling_worst();
	//sche->PrintScheduling();

	sche->GetTileSlack();
	sche->GetSpill1();

	avg_uphill_cost = 1;
}
*/

/*
Scheduling* SAOptimizer::Neighbor_tile(Scheduling* sche, int tile, int loc_idx){
	Scheduling* new_sche = new Scheduling(*sche);
	//map<int,set<int>>::iterator i=new_sche->tile_slack.begin();
	//for(int n=0; n<idx; n++)
	//   i++;
	//int tile = i->first;

	//int it2_idx = rand()%i->second.size();
	vector<tile_sche>::iterator j = new_sche->tile_slack[tile].begin();
	for(int n=0; n<loc_idx; n++)
	   j++;

	int new_start = j->start;
	int new_end = j->end;	


	//cout << endl << "select tile " << tile << endl;
	//cout << "move to loc " << new_start << endl;

	int old_start = new_sche->tiles_sche[tile].start;
	int old_end = new_sche->tiles_sche[tile].end;	

	//update hw
	new_sche->num_cb[new_sche->tiles[tile].cb->name][old_start]--;
	new_sche->num_cb[new_sche->tiles[tile].cb->name][new_start]++;


	//update for live in
	for(map<int, int>::iterator in_op=new_sche->tiles[tile].livein_ops.begin(); in_op!=new_sche->tiles[tile].livein_ops.end(); in_op++){
	   int in_tile = new_sche->ops[in_op->first].tile;

	   int new_read_cycle = new_start+in_op->second;
	   int old_read_cycle = old_start+in_op->second;
	   
	   liverange* lr = new liverange;
	   if(new_read_cycle > new_sche->op_in_cycle[in_op->first]){
	      new_sche->GetLiverange(lr, in_op->first, new_sche->op_in_cycle[in_op->first]);
	   }
	   else{
	      new_sche->GetLiverange(lr, in_op->first, new_read_cycle);
	   }

	   new_sche->RemoveLRfromMem(lr, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	   //need to remove read
	   if((old_read_cycle <= lr->write_cycle) || (old_read_cycle > lr->last_cycle)){
	      int old_membank = new_sche->op_in_bank[in_op->first][old_read_cycle];
	      new_sche->num_read[old_membank][old_read_cycle]--;
	   }

	   new_sche->UpdateLiverange(lr, old_read_cycle, new_read_cycle);
	   
	   lr->mem_bank = j->inop_membank[in_op->first];

	   new_sche->AddLRtoMem(lr, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	   for(int c=lr->write_cycle+1; c<=lr->last_cycle; c++){
	      new_sche->op_in_bank[in_op->first][c] = j->inop_membank[in_op->first];
	   }
	   delete lr;

           int new_last_userloc = 0;
	   int old_last_userloc = 0;
	   for(vector<int>::iterator user_tile=new_sche->tiles[in_tile].post_tiles[in_op->first].begin(); user_tile!=new_sche->tiles[in_tile].post_tiles[in_op->first].end(); user_tile++){
	      if(*user_tile == tile){
	         if(new_read_cycle > new_last_userloc){
		    new_last_userloc = new_read_cycle;
	         }
		 if(old_read_cycle > old_last_userloc){
		    old_last_userloc = old_read_cycle;
	         }
	      }
	      else{
		 int user_loc = new_sche->tiles_sche[*user_tile].start+new_sche->tiles[*user_tile].livein_ops[in_op->first];
		 if(user_loc > new_last_userloc){
		    new_last_userloc = user_loc;
	         }
		 if(user_loc > old_last_userloc){
		    old_last_userloc = user_loc;
		 }
	      }
	   }
	   

	   if(new_last_userloc != old_last_userloc){
	      new_sche->op_in_cycle[in_op->first] = new_last_userloc;
	      if(new_last_userloc < old_last_userloc){
	         for(int c=new_last_userloc+1; c<=old_last_userloc; c++){
	            new_sche->op_in_bank[in_op->first][c] = -1;
	         }
		 //clear spills after it
		 for(vector<int>::iterator sp=new_sche->op_spills[in_op->first].begin(); sp!=new_sche->op_spills[in_op->first].end(); sp++){
	            if((new_sche->spills_sche[*sp].cond == 1) && (new_sche->spills_sche[*sp].start >= new_last_userloc)){
		       new_sche->spills_sche[*sp].cond = 0;
		       new_sche->spills_sche[*sp].start = 0;
		       new_sche->spills_sche[*sp].end = 0;
	            }
	         }
	      }
	   }

	}

	//update for liveout op
	for(vector<int>::iterator out_op=new_sche->tiles[tile].liveout_ops.begin(); out_op!=new_sche->tiles[tile].liveout_ops.end(); out_op++){
	   int new_write_cycle = new_end;
	   int old_write_cycle = old_end;
	
	   liverange* lr = new liverange;
	   new_sche->GetLiverange(lr, *out_op, old_write_cycle+1);	   
	   new_sche->RemoveLRfromMem(lr, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	   for(int c=lr->write_cycle+1; c<=lr->last_cycle; c++){
	      new_sche->op_in_bank[*out_op][c] = -1;
	   }
	   lr->write_cycle = new_write_cycle;
	   lr->mem_bank = j->outop_membank[*out_op];
	   new_sche->AddLRtoMem(lr, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	   for(int c=lr->write_cycle+1; c<=lr->last_cycle; c++){
	      new_sche->op_in_bank[*out_op][c] = lr->mem_bank;
	   }
	}

	new_sche->tiles_sche[tile].start = new_start;
	new_sche->tiles_sche[tile].end = new_end;
	
	//new_sche->UpdateInfo();
	new_sche->GetTileSlack();
	new_sche->GetSpill1();

	new_sche->Testing();

	return new_sche;
}

Scheduling* SAOptimizer::Neighbor_spill01(Scheduling* sche, int spill_idx, int loc_idx){
	Scheduling* new_sche = new Scheduling(*sche);
	//map<int,vector<array<int,2>>>::iterator i = new_sche->spill0_slack.begin(); 
	//for(int n=0; n<idx; n++){
	//   i++;   
	//}
	//int spill_idx = i->first;

	//int it2_idx = rand()%i->second.size();
	vector<spill_sche>::iterator j = new_sche->spill0_slack[spill_idx].begin();
	for(int n=0; n<loc_idx; n++){
	   j++;
	}
	//int new_start = j->at(0);
	//int new_end = j->at(1);


	cout << "select to turn on spill0 " << spill_idx << " start at " << j->start << " end at " << j->end << endl;

	liverange* lr = new liverange;
	new_sche->GetLiverange(lr, j->op, j->start);
	new_sche->RemoveLRfromMem(lr, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
        for(int c=lr->write_cycle+1; c<=lr->last_cycle; c++){
	   new_sche->op_in_bank[j->op][c] = -1;
	}


	liverange* lr1 = new liverange;
	lr1->op = lr->op;
	lr1->mem_bank = j->start_membank;
	lr1->write_cycle = lr->write_cycle;
	lr1->last_cycle = j->start;
	lr1->read_cycles.insert(j->start);
	for(set<int>::iterator r=lr->read_cycles.begin(); r!=lr->read_cycles.end(); r++){
	   if(*r < j->start){
	      lr1->read_cycles.insert(*r);
	   }
	}
	new_sche->AddLRtoMem(lr1, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	for(int c=lr1->write_cycle+1; c<=lr1->last_cycle; c++){
	   new_sche->op_in_bank[j->op][c] = lr1->mem_bank;
	}


	liverange* lr2 = new liverange;
	lr2->op = lr->op;
	lr2->mem_bank = j->end_membank;
	lr2->write_cycle = j->end;
	lr2->last_cycle = lr->last_cycle;
	for(set<int>::iterator r=lr->read_cycles.begin(); r!=lr->read_cycles.end(); r++){
	   if(*r > j->end){
	      lr2->read_cycles.insert(*r);
	   }
	}
	new_sche->AddLRtoMem(lr2, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	for(int c=lr2->write_cycle+1; c<=lr2->last_cycle; c++){
	   new_sche->op_in_bank[j->op][c] = lr2->mem_bank;
	}


	delete lr;
	delete lr1;
	delete lr2;

	new_sche->spills_sche[spill_idx].cond = 1;
	new_sche->spills_sche[spill_idx].start = j->start;
	new_sche->spills_sche[spill_idx].end = j->end;

	//cout << "convert spill 0 to 1" << "(tile:" << tile <<",op:" << op << ",spill:"<<spill_idx<<")" << ", loc: " << loc << endl;

	new_sche->UpdateInfo();

	new_sche->Testing();	
	
	return new_sche;
}

Scheduling* SAOptimizer::Neighbor_spill10(Scheduling* sche, int spill_idx){
	Scheduling* new_sche = new Scheduling(*sche);
	//set<int>::iterator i = new_sche->spill1.begin();
	//for(int n=0; n<idx; n++){
	//   i++;
	//}
	//int spill_idx = *i;
	//cout << "select to close spill1 " << spill_idx << endl;
	
	int op = new_sche->spills_sche[spill_idx].op;
	int tile = new_sche->ops[op].tile;	

	int old_start = new_sche->spills_sche[spill_idx].start;
	int old_end = new_sche->spills_sche[spill_idx].end;
	
	int before_mem_bank = new_sche->op_in_bank[op][old_start];
	int after_mem_bank = new_sche->op_in_bank[op][old_end+1];

	liverange* lr1 = new liverange;
	new_sche->GetLiverange(lr1, op, old_start);
	new_sche->RemoveLRfromMem(lr1, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	
	liverange* lr2 = new liverange;
	new_sche->GetLiverange(lr2, op, old_end+1);
	new_sche->RemoveLRfromMem(lr2, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);

	liverange* lr = new liverange;
	lr->op = op;
	lr->write_cycle = lr1->write_cycle;
	lr->last_cycle = lr2->last_cycle;
	lr->read_cycles.insert(lr2->last_cycle);
	for(vector<int>::iterator user_tile=new_sche->tiles[tile].post_tiles[op].begin(); user_tile!=new_sche->tiles[tile].post_tiles[op].end(); user_tile++){
	   int user_loc = new_sche->tiles_sche[*user_tile].start+new_sche->tiles[*user_tile].livein_ops[op];
	   if((user_loc > lr1->write_cycle) && (user_loc < lr2->last_cycle)){
	      if(lr->read_cycles.find(user_loc) == lr->read_cycles.end()){
	         lr->read_cycles.insert(user_loc);
	      }
	   }
	}

	lr->mem_bank = new_sche->spill1[spill_idx];
	new_sche->AddLRtoMem(lr, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	for(int c=lr->write_cycle+1; c<=lr->last_cycle; c++){
	   new_sche->op_in_bank[op][c] = lr->mem_bank;
	}

	delete lr1;
	delete lr2;
	delete lr;

	new_sche->spills_sche[spill_idx].cond = 0;
	new_sche->spills_sche[spill_idx].start = 0;
	new_sche->spills_sche[spill_idx].end = 0;
	
	//cout << "convert spill 1 to 0" << "(tile:" << tile <<",op:" << op << ",spill:"<<spill_idx<<")" << endl;
	
	//new_sche->UpdateInfo();
	new_sche->GetTileSlack();
	new_sche->GetSpill1();

	//new_sche->GetTileSlack();
	//new_sche->GetSpill0Slack();
	//new_sche->GetSpill1();
	//new_sche->GetSpill1Slack();

	new_sche->Testing();
	
	return new_sche;
}


Scheduling* SAOptimizer::Neighbor_spill1(Scheduling* sche, int spill_idx, int loc_idx){
	Scheduling* new_sche = new Scheduling(*sche);
	//map<string, set<int>>::iterator i=new_sche->spill1_slack.begin();
	//for(int n=0; n<idx; n++){
	//   i++;
	//}
	//int under = i->first.find('_');
	//int under = spill_startend.find('_');
	//string spill;
	//string start_end;
	//for(int n=0; n<under; n++){
	//   spill += spill_startend[n];
	//}
	//int spill_idx = stoi(spill);
	//for(int n=under+1; n<spill_startend.size(); n++){
	//   start_end += spill_startend[n];
	//}

	//int it2_idx = rand()%(i->second.size());
	vector<spill_sche>::iterator j=new_sche->spill1_slack[spill_idx].begin();
	for(int n=0; n<loc_idx; n++){
	   j++;
	}
	//int loc = *k;

	cout << "select move spill1 " << spill_idx << " start at " << j->start << " end at " << j->end << endl;

	int old_start = new_sche->spills_sche[spill_idx].start;
	int old_end = new_sche->spills_sche[spill_idx].end;
	
	int op = new_sche->spills_sche[spill_idx].op;
	int tile = new_sche->ops[op].tile;
	int store_mem_bank = new_sche->op_in_bank[op][old_start];
	int load_mem_bank = new_sche->op_in_bank[op][old_end+1];	

	liverange* lr1 = new liverange;
	new_sche->GetLiverange(lr1, op, old_start);
	new_sche->RemoveLRfromMem(lr1, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	for(int c=lr1->write_cycle+1; c<=lr1->last_cycle; c++){
	   new_sche->op_in_bank[op][c] = -1;
	}

	liverange* lr2 = new liverange;
	new_sche->GetLiverange(lr2, op, old_end+1);

	new_sche->RemoveLRfromMem(lr2, &new_sche->num_read, &new_sche->num_write,&new_sche->num_live);

	for(int c=lr2->write_cycle+1; c<=lr2->last_cycle; c++){
	   new_sche->op_in_bank[op][c] = -1;
	}

	liverange* lr1p = new liverange;
	lr1p->op = lr1->op;
	lr1p->write_cycle = lr1->write_cycle;
	lr1p->last_cycle = j->start;
	for(vector<int>::iterator user_tile=new_sche->tiles[tile].post_tiles[op].begin(); user_tile!=new_sche->tiles[tile].post_tiles[op].end(); user_tile++){
	   int user_loc = new_sche->tiles_sche[*user_tile].start+new_sche->tiles[*user_tile].livein_ops[op];
	   if((user_loc>lr1p->write_cycle) && (user_loc<=lr1p->last_cycle)){
	      lr1p->read_cycles.insert(user_loc);
	   }
	}
	lr1p->mem_bank = j->start_membank;
	new_sche->AddLRtoMem(lr1p, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	for(int c=lr1p->write_cycle+1; c<=lr1p->last_cycle; c++){
	   new_sche->op_in_bank[op][c] = lr1p->mem_bank;
	}
	
	liverange* lr2p = new liverange;
	lr2p->op = lr2->op;
	lr2p->write_cycle = j->end;
	lr2p->last_cycle = lr2->last_cycle;
	lr2p->read_cycles = lr2->read_cycles;
	lr2p->mem_bank = j->end_membank;
	new_sche->AddLRtoMem(lr2p, &new_sche->num_read, &new_sche->num_write, &new_sche->num_live);
	for(int c=lr2p->write_cycle+1; c<=lr2p->last_cycle; c++){
	   new_sche->op_in_bank[op][c] = lr2p->mem_bank;
	}
	
	new_sche->spills_sche[spill_idx].start = j->start;
	new_sche->spills_sche[spill_idx].end = j->end;

	new_sche->UpdateInfo();

	new_sche->Testing();	

	return new_sche;	
}


Scheduling* SAOptimizer::Neighbor(Scheduling* sche){
	Scheduling* new_sche;

	int tileslack = sche->tile_slack.size();
	int spill0slack = sche->spill0_slack.size();
	int spill1 = sche->spill1.size();
	int spill1slack = sche->spill1_slack.size();

	int total_size = tileslack+spill0slack+spill1+spill1slack;
	
	int select_loc = rand()%total_size;
	   
	if(select_loc < sche->tile_slack.size()){
	   map<int,vector<tile_sche>>::iterator i = sche->tile_slack.begin();
	   for(int n=0; n<select_loc; n++){
	      i++;
	   }
	   int tile = i->first;
	   int size = i->second.size();
	   int loc_idx = rand()%size;

	   new_sche = Neighbor_tile(sche, tile, loc_idx);
	}
	else{
	   select_loc -= sche->tile_slack.size();
	   if(select_loc < sche->spill0_slack.size()){
	      map<int, vector<spill_sche>>::iterator i=sche->spill0_slack.begin();
	      for(int n=0; n<select_loc; n++){
	         i++;
	      }
	      int spill_idx = i->first;
	      int loc_idx = rand()%i->second.size();

	      new_sche = Neighbor_spill01(sche, spill_idx, loc_idx);
	   }
	   else{
	      select_loc -= sche->spill0_slack.size();
	      if(select_loc < sche->spill1.size()){
		 map<int, int>::iterator i=sche->spill1.begin();
	         for(int n=0; n<select_loc; n++){
		    i++;
		 }
		 int spill_idx = i->first;
	         
	         new_sche = Neighbor_spill10(sche, spill_idx);
	      }
	      else{
		 select_loc -= sche->spill1.size();
	         if(select_loc < sche->spill1_slack.size()){
		    map<int, vector<spill_sche>>::iterator i=sche->spill1_slack.begin();
		    for(int n=0; n<select_loc; n++){
	               i++;
	            }
		    int spill_idx = i->first;
		    int loc_idx = rand()%i->second.size();

	            new_sche = Neighbor_spill1(sche, spill_idx, loc_idx);
	         }
	      }
	   }
	}

	return new_sche;
}

*/

/*
void SAOptimizer::GetAvgUphillCost()
{
	Scheduling* cur_sche = new Scheduling(*sche);
	cur_sche->InitScheduling_best();
	cur_sche->PrintScheduling();
	cur_sche->Cost();
	cur_sche->PrintCost();

	int cur_cost = cur_sche->Cost();

	Scheduling* new_sche;
	int new_cost;

	int total_cost = 0;
	int count_uphill = 0;
	for(int i=0; i<1000; i++){
	   cout << "cur_cost: " << cur_cost << endl;

	   new_sche = Neighbor(cur_sche);
	   new_cost = new_sche->Cost();
	   cout << "new_cost: " << new_cost << endl;

	   if(new_cost > cur_cost){
	      count_uphill++;
	      total_cost += (new_cost-cur_cost);
	   }	   

	   delete cur_sche;
	   cur_sche = new_sche;
	   cur_cost = new_cost;

	}

	cout << "count_uphill: " << count_uphill << endl;
	cout << "total_cost: " << total_cost << endl;
	
	//avg_uphill_cost = total_cost/count_uphill;
	//cout << "Avg uphill cost: " << avg_uphill_cost << endl;
}
*/


/*
void SAOptimizer::Solver(){
	Scheduling* new_sche;
	new_sche = Neighbor_tile(sche, 286, 23);
	//for(map<int,vector<tile_sche>>::iterator i=sche->tile_slack.begin(); i!=sche->tile_slack.end(); i++){
        //   for(vector<tile_sche>::iterator j=i->second.begin(); j!=i->second.end(); j++){
	//      int tile = i->first;
	//      int loc_idx = j-i->second.begin();
	//      new_sche = Neighbor_tile(sche, tile, loc_idx);
	//   }
	//}

	//for(map<int, vector<spill_sche>>::iterator i=sche->spill0_slack.begin(); i!=sche->spill0_slack.end(); i++){
	//   int spill_idx = i->first;
	//   for(vector<spill_sche>::iterator j=i->second.begin(); j!=i->second.end(); j++){
	//      int loc_idx = j-i->second.begin();
	//      new_sche = Neighbor_spill01(sche, spill_idx, loc_idx);
	//   }
	//}

	//for(map<int,int>::iterator i=sche->spill1.begin(); i!=sche->spill1.end(); i++){
	//   int spill_idx = i->first; 
	//   new_sche = Neighbor_spill10(sche, spill_idx);
	//}

	//for(map<int, vector<spill_sche>>::iterator i=sche->spill1_slack.begin(); i!=sche->spill1_slack.end(); i++){
	//   int spill_idx = i->first;
	//   for(vector<spill_sche>::iterator j=i->second.begin(); j!=i->second.end(); j++){
	//      int loc = j-i->second.begin();
	//      new_sche = Neighbor_spill1(sche, spill_idx, loc);
	//   }
	//}
}
*/
/*
void SAOptimizer::Solver(){
	cout << "Before optimization: " << endl;
	sche->GetNumCBMax();
	sche->PrintNumCBMax();
	sche->GetCost();
	sche->PrintCost();
	sche->PrintPerf();


	while((!sche->tile_slack.empty()) || (!sche->spill1.empty())){
	   Scheduling* new_sche;
	   if(!sche->tile_slack.empty()){
	      int tile = sche->tile_slack.begin()->first;
	      new_sche = Neighbor_tile(sche, tile, 0);
	   }
	   else{
	      int spill_idx = sche->spill1.begin()->first;
	      new_sche = Neighbor_spill10(sche, spill_idx);
	   }
	   sche = new_sche;
	}

	cout << endl <<"After optimization: " << endl;
	sche->GetNumCBMax();
	sche->PrintNumCBMax();
	sche->GetCost();
	sche->PrintCost();
	sche->PrintPerf();
}
*/

/*
void SAOptimizer::Solver(){
	ofstream file;
	file.open("iteration_results.txt");	

	cout << endl << "Start scheduling: " << endl;
	sche->GetCost();

	cout << endl << "Start cost: " << endl;
	sche->PrintCost();

	//sche->PrintSpill1();
	//sche->PrintTileSlack();
	//sche->PrintSpill0Slack();
	//sche->PrintSpill1Slack();


	
	Scheduling* new_sche;

	Scheduling* best_sche = new Scheduling(*sche);
        int best_cost = best_sche->GetCost();
	//best_sche->PrintScheduling_graph();	

	int cost;
	int new_cost;

	double p_begin = 0.999;
	double p_end = 0.001;
	double T_begin = -avg_uphill_cost/log(p_begin);
	double T_end = -avg_uphill_cost/log(p_end);
	//double T_begin = 1000;

	int num_t_dec = 200;
	//int num_t_dec = 1;
	double f = pow(T_end/T_begin, 1.0/num_t_dec);
	//double f = 0.96;
	cout << "f: " << f << endl;

	int num_to_equi = 10;
	//int num_to_equi = sche->tiles.size();
	//simulated annealing flow
	double cost_start_t;

	clock_t t;
	t = clock();

	double T = T_begin;
	int uphill_cost;
	double p;


	vector<int> uphill_list;
	for(int i=0; i<num_t_dec; i++){
	   cout << "t dec: " << i << endl << endl;
	   map<int,int> e_count;

	   for(int j=0; j<num_to_equi; j++){
	      cout << "inner it: " << j << endl;
	      cost = sche->GetCost();
	      //sche->PrintCost();


	      if(j==0)
	         cost_start_t = cost;

	      new_sche = Neighbor(sche);
	      new_cost = new_sche->GetCost();
		
	      cout << "Get neighbor, its cost " << new_cost << endl;

	      //if(i == num_t_dec-1){
	         //new_sche->PrintCost();      
	         //new_sche->PrintSpill1();
	         //new_sche->PrintTileSlack();
	         //new_sche->PrintSpill0Slack();
	         //new_sche->PrintSpill1Slack();
	         //cout << endl;
	      //}

	      if(new_cost <= best_cost){
	         delete best_sche;
	         best_sche = new Scheduling(*new_sche);
	         best_cost = new_cost;
	      }
	
	      if(new_cost < cost){
	         cout << "replace old sche" << endl;
	         //replace the current scheduling
	         delete sche;
	         sche = new_sche;
	         cost = new_cost;
	         //cout << "replace! " << endl;
	      }
	      else if(new_cost > cost){
	         uphill_cost = (new_cost-cost);
	         //cout << "K: " << K << endl;
	         p = 1/(exp(uphill_cost/T));

		 cout << "p " << p << endl; 

	         uphill_list.push_back(uphill_cost);

	         //cout << "p: " << p << endl;
	         double rd = (double)rand()/(double)RAND_MAX;
	         if(rd < p){
		    cout << "replace old sche" << endl;
	            delete sche;
	            sche = new_sche;
	 	    cost = new_cost;
		    //cout << "replace! " << endl;
	         }
	         else{
 	            delete new_sche;
	         }
	      }

	      file << cost << " ";
	   }
	   file << endl;
	   cout << "T: " << T << endl;
	   T = f*T;
	}

	cout << "Finish optimization" << endl;
	
	t = clock() - t;
	cout << "Time used: " << (float)t/CLOCKS_PER_SEC << " seconds" << endl;

	//PlotDis();
	//PlotCost();
	//cout << endl;
	
	cout << endl << "Best scheduling: " << endl;
	best_sche->PrintScheduling();
	//best_sche->PrintTileSlack();
	//best_sche->PrintSpill0Slack();
	//best_sche->PrintSpill1();
	//best_sche->PrintSpill1Slack();

	best_sche->GetNumCBMax();
	best_sche->PrintNumCBMax();

	cout << endl << "Best cost: " << endl;
	best_sche->GetCost();
	best_sche->PrintCost();

	cout << endl << "Best performance: " << endl;
	best_sche->PrintPerf();

	//for(int i=0; i<uphill_list.size(); i++){
	//   cout << uphill_list[i] << ", ";
	//}
	//cout << endl;
}
*/


/*
void SAOptimizer::PlotCost(){
	Py_Initialize();
	
	PyObject *sys = PyImport_ImportModule("sys");
	PyObject *path = PyObject_GetAttrString(sys, "path");
	PyList_Append(path, PyString_FromString("."));

	PyObject* pModuleName = PyString_FromString("./py/py_cost");
	PyObject* pModule = PyImport_Import(pModuleName);
	if(pModule == NULL){
	   cout << "no module" <<endl;
	}
	//pDict = PyModule_GetDict(pModule);
	cout << "module name: " << PyModule_GetName(pModule) << endl;
	PyObject* pFunc = PyObject_GetAttrString(pModule, "cost_time");
	if(!pFunc){
	   cout << "no funciton"<<endl;
	}

	PyObject* pXVec;
	pXVec = PyTuple_New(cost_time.size());

	int idx=0;
	for(vector<double>::iterator i=cost_time.begin(); i!=cost_time.end(); i++){
	   PyObject* pValue = PyFloat_FromDouble(*i);
	   PyTuple_SetItem(pXVec,idx,pValue);
	   idx++;
	}

	PyObject* pArg;
	pArg = PyTuple_New(1);
	PyTuple_SetItem(pArg, 0, pXVec);

	PyObject_CallObject(pFunc, pArg);
	cout << "called python function" << endl;

	Py_Finalize();
}


void SAOptimizer::PlotDis(){
	PyObject *pModuleName, *pModule, *pDict, *pFuncName, *pFunc, *pArg;

	Py_Initialize();

	PyObject *sys = PyImport_ImportModule("sys");
	PyObject *path = PyObject_GetAttrString(sys, "path");
	PyList_Append(path, PyString_FromString("."));

	pModuleName = PyString_FromString("./py/py_cost");
	pModule = PyImport_Import(pModuleName);
	if(pModule == NULL){
	   cout << "no module" <<endl;
	}
	//pDict = PyModule_GetDict(pModule);
	cout << "test" << endl;
	cout << "module name: " << PyModule_GetName(pModule) << endl;
	pFunc = PyObject_GetAttrString(pModule, "costdis");
	if(!pFunc){
	   cout << "no funciton"<<endl;
	}
	
	PyObject* pXVec;
	PyObject* pYVec;
	PyObject* pXVec_t;
	PyObject* pYVec_t;
	PyObject* pXValue;
	PyObject* pYValue;
	
	pXVec = PyTuple_New(cost_p.size());
	pYVec = PyTuple_New(cost_p.size());

	int idx = 0;
	for(vector<map<double,double>>::iterator i=cost_p.begin(); i!=cost_p.end(); i++){
	   pXVec_t = PyTuple_New(i->size());
	   pYVec_t = PyTuple_New(i->size());
	   int idx_j = 0;
	   for(map<double,double>::iterator j=i->begin(); j!=i->end(); j++){
	      pXValue = PyFloat_FromDouble(j->first);
	      pYValue = PyFloat_FromDouble(j->second);
	      PyTuple_SetItem(pXVec_t, idx_j, pXValue);
	      PyTuple_SetItem(pYVec_t, idx_j, pYValue);
	      idx_j++;
	   }
	   PyTuple_SetItem(pXVec, idx, pXVec_t);
	   PyTuple_SetItem(pYVec, idx, pYVec_t);
	   idx++;
	}

	PyObject* pXStartVec;
	PyObject* pYStartVec;

	pXStartVec = PyTuple_New(cost_start.size());
	pYStartVec = PyTuple_New(cost_start.size());
	idx = 0;
	for(vector<double>::iterator i=cost_start.begin(); i!=cost_start.end(); i++){
	   pXValue = PyFloat_FromDouble(*i);	   
	   pYValue = PyFloat_FromDouble(cost_p[idx][*i]);
	   PyTuple_SetItem(pXStartVec, idx, pXValue);	   
	   PyTuple_SetItem(pYStartVec, idx, pYValue);	   
	   idx ++;
	}
	
	pArg = PyTuple_New(4);
	PyTuple_SetItem(pArg, 0, pXVec);
	PyTuple_SetItem(pArg, 1, pYVec);
	PyTuple_SetItem(pArg, 2, pXStartVec);
	PyTuple_SetItem(pArg, 3, pYStartVec);
	
	PyObject_CallObject(pFunc, pArg);
	Py_Finalize();
	
}

*/
