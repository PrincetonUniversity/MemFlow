#ifndef SAOPTI
#define SAOPTI

#include<vector>

#include "Hardware.hpp"
#include "Scheduling.hpp"
#include "Tile.hpp"

using namespace std;

class SAOptimizer{
	public:
	   SAOptimizer(vector<Operation> &ops, vector<Tile> &tiles);
	   ~SAOptimizer(){};
	   Scheduling* Neighbor_tile(Scheduling* sche, int tile, int loc_idx);
	   Scheduling* Neighbor_spill01(Scheduling* sche, int spill_idx, int loc_idx);
	   Scheduling* Neighbor_spill10(Scheduling* sche, int spill_idx);
	   Scheduling* Neighbor_spill1(Scheduling* sche, int spill_idx, int loc_idx);
	   Scheduling* Neighbor(Scheduling* sche);
	   void GetAvgUphillCost();
	   void Solver();
	   void PlotDis();
	   void PlotCost();
	private:
	   Scheduling* sche;
	   int avg_uphill_cost;
	   vector<double> cost_time;   
	   vector<double> cost_start;
	   vector<map<double,double>> cost_p;
};


#endif

