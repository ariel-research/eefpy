#pragma once
#include "efficiency.h"

class None_Efficiency_ILP : public Efficiency_ILP {
public:

	None_Efficiency_ILP(EEF &, EEF_Config);

	// return a TC which serves as a a witness to the reference allocation not being pareto efficient
	vector< vector<long> > check_allocation(vector< vector<long> > pi);
};
