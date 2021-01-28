#pragma once
#include "efficiency.h"

class MINTC_Efficiency_ILP : public Efficiency_ILP {
public:

	MINTC_Efficiency_ILP(EEF &, EEF_Config);

	// return a TC which serves as a a witness to the reference allocation not being pareto efficient
	// 	Notice: check_allocation also adds this TC to its own model!
	vector< vector<long> > check_allocation(vector< vector<long> > pi);

private:

	vector< vector<IloIntVar> > x_ref; // x_ref[i][j] = number of item j that agent i has in the reference allocation
					   // it's an ILP variable but we will force it to be a constant
	vector<IloRange> noworseoff;
	IloObjective objective;
};
