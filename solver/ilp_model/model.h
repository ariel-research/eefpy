#pragma once

#include <vector>

#include <ilcplex/ilocplex.h>

#include "../eef.h"
#include "../config.h"


using namespace std;

/* Base class for all types of ILPs we will be using
 * Shared properties:
 *    - ILP variables x[i][j] representing the number of items of type j that agent i receives
 *    - constraint to find only complete allocations
 *            (for incomplete allocations add a trash agent, and change mult LB and UB)
 *    - option to add constraints based on trading cycle
 *            (exclude allocations in which the trading cycle would be applicable)
 */

class ILP_Allocation_Model {
public:
	IloEnv env;
	IloModel model;
	IloCplex cplex;

	vector< vector<IloIntVar> > x; // ILP variables for the allocation
	vector<IloRange> mult;	       // constraint for enforcing how much of a given item type must be allocated

	EEF &I; // reference to the original EEF instance (utility matrix etc.)
	EEF_Config cfg;	
	// some shortcuts/references to avoid writing I.n and I.u all the time

	// you get your own copy of n and m so we can hide the existance of the trash agent if we want to
	long n; // number of agents
	long m; // number of item groups

	vector<long> &mu; // item multiplicities
	vector< vector<long> > &u; // utilities matrix u[i][j] = utility of item j to agent i
	



	ILP_Allocation_Model(EEF &, EEF_Config);
	virtual void add_tc(vector< vector<long> > tc);
	long num_tcs = 0;
private:
	void add_tc_paper_version(vector< vector<long> > tc);
	void add_tc_lazy_cplex_version(vector< vector<long> > tc);
};

