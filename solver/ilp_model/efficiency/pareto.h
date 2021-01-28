#pragma once
#include "efficiency.h"

class Pareto_Efficiency_ILP : public Efficiency_ILP {
public:

	Pareto_Efficiency_ILP(EEF &, EEF_Config);

	// return a TC which serves as a a witness to the reference allocation not being pareto efficient
	vector< vector<long> > check_allocation(vector< vector<long> > pi);

private:
	vector<IloRange> noworseoff;
	IloRange dom_cstr;
};
