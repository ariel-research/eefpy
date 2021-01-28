#pragma once
#include "../model.h"

/* Class of ILPs that compute a trading cycle that serves as a witness
 * for not having some given efficiency property
 */

class Efficiency_ILP : public ILP_Allocation_Model {
public:

	Efficiency_ILP(EEF &, EEF_Config);

	// return a TC which should be added to the ILP or a vector of size 0 if no TC is to be added
	virtual vector< vector<long> > check_allocation(vector< vector<long> > pi) = 0;
};
