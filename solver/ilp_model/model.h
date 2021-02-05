/****************************************************************************
*****************************************************************************
*HiMEFAF: High-Multiplicity Efficient Fair Allocations Finder 
*Copyright (C) 2020 Aleksander Figiel
*****************************************************************************
*This file is a part of HiMEFAF: High-Multiplicity Efficient Fair Allocations
*Finder 
*
*HiMEFAF is free software: you can redistribute it
*and/or modify it under the terms of the GNU General Public
*License as published by the Free Software Foundation, either
*version 3 of the License, or (at your option) any later
*version.
*
*This program is distributed in the hope that it will be
*useful, but WITHOUT ANY WARRANTY; without even the implied
*warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*PURPOSE.  See the GNU General Public License for more
*details.
*
*You should have received a copy of the GNU General Public
*License along with this program.  If not, see
*<https://www.gnu.org/licenses/>
*****************************************************************************
****************************************************************************/
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

