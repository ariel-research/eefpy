#include <cassert>
#include <vector>
#include <string>
#include <chrono>
#include <stdlib.h>
#include <time.h>

#include <ilcplex/ilocplex.h>

#include "eef.h"

using namespace std;


vector< vector< vector<long> > > EEF::all_trading_cycles() {
	//TODO? currently we only count trading cycles up to ~10M

	IloEnv env;
	IloModel model(env);
	vector< vector<IloIntVar> > x; // x[i][j] = number of item j that agent i has
	x.resize(n);

	for (long i = 0; i < n; i++) {
		x[i].resize(m);
		for (long j = 0; j < m; j++) {
			// the trading cycle vector consists of integer values
			x[i][j] = IloIntVar(env, IloIntMin, IloIntMax, ("x_" + to_string(i) + "^" + to_string(j)).c_str());
		}
	}
	/* setup constraints */

	/* conservation of items */
	for (long j = 0; j < m; j++) {
		IloExpr *exp = new IloExpr(env);
		IloExpr *exp2 = new IloExpr(env);
		for (long i = 0; i < n; i++) {
			(*exp) += x[i][j];
			(*exp2) += IloAbs(x[i][j]);
		}

		model.add((*exp) == 0);
		model.add((*exp2) <= 2*mu[j]);
	}
	vector<IloRange> noworseoff_constraints;
	noworseoff_constraints.reserve(n);

	/* each agent is not worse off */
	for (long i = 0; i < n; i++) {
		IloExpr *exp = new IloExpr(env);
		for (long j = 0; j < m; j++) {
			(*exp) += u[i][j] * x[i][j];
		}

		noworseoff_constraints.push_back((*exp) >= 0);
		

		model.add(noworseoff_constraints[i]);
	}

	/* sum of utilities has to increase by at least 1*/
	IloExpr domination_exp(env);
	IloRange domination_constraint;
	{
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++) {
				domination_exp += u[i][j] * x[i][j];
			}
		}
		domination_constraint = domination_exp >= 1;
	}
	model.add(domination_constraint);


	IloCplex cplex(model);
//	cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap , 0.0);
//	cplex.setParam(IloCplex::Param::MIP::Tolerances::AbsMIPGap , 0.0);
//	cplex.setParam(IloCplex::Param::Emphasis::Numerical, true);

	cplex.setParam(IloCplex::Param::MIP::Pool::Intensity, 4);
	cplex.setParam(IloCplex::Param::MIP::Limits::Populate, 1e7);
//	cplex.setParam(IloCplex::Param::MIP::Pool::Capacity, 100);
	cplex.setOut(env.getNullStream());
	cplex.setWarning(env.getNullStream());

	cplex.populate();
	long numsol = cplex.getSolnPoolNsolns();

	cout << "# Found " << numsol << " trading cycles" << endl;


	vector< vector< vector<long> > > hi;
	return hi;
}


vector< vector< vector<long> > > EEF::minimal_trading_cycles(EEF_Config cfg) {
	IloEnv env;
	IloModel model(env);
	vector< vector<IloIntVar> > x; // x[i][j] = number of item j that agent i has
	x.resize(n);

	for (long i = 0; i < n; i++) {
		x[i].resize(m);
		for (long j = 0; j < m; j++) {
			// the trading cycle vector consists of integer values
			x[i][j] = IloIntVar(env, IloIntMin, IloIntMax, ("x_" + to_string(i) + "^" + to_string(j)).c_str());
		}
	}
	/* setup constraints */

	/* conservation of items */
	for (long j = 0; j < m; j++) {
		IloExpr *exp = new IloExpr(env);
		IloExpr *exp2 = new IloExpr(env);
		for (long i = 0; i < n; i++) {
			(*exp) += x[i][j];
			(*exp2) += IloAbs(x[i][j]);
		}

		model.add((*exp) == 0);
		model.add((*exp2) <= 2*mu[j]);
	}
	vector<IloRange> noworseoff_constraints;
	noworseoff_constraints.reserve(n);

	/* each agent is not worse off */
	for (long i = 0; i < n; i++) {
		IloExpr *exp = new IloExpr(env);
		for (long j = 0; j < m; j++) {
			(*exp) += u[i][j] * x[i][j];
		}

		noworseoff_constraints.push_back((*exp) >= 0);
		

		model.add(noworseoff_constraints[i]);
	}

	/* sum of utilities has to increase by at least 1*/
	IloExpr domination_exp(env);
	IloRange domination_constraint;
	{
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++) {
				domination_exp += u[i][j] * x[i][j];
			}
		}
		domination_constraint = domination_exp >= 1;
	}
	model.add(domination_constraint);

	/* objective: minimum trading cycle */
	IloExpr num_shuffle(env);
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < m; j++) {
			num_shuffle += IloAbs(x[i][j]);
		}
	}
	IloObjective objective = IloMinimize(env, num_shuffle);
	model.add(objective);



	IloCplex cplex(model);
//	cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap , 0.0);
//	cplex.setParam(IloCplex::Param::MIP::Tolerances::AbsMIPGap , 0.0);
//	cplex.setParam(IloCplex::Param::Emphasis::Numerical, true);

//	cplex.setParam(IloCplex::Param::MIP::Pool::Intensity, 4);
//	cplex.setParam(IloCplex::Param::MIP::Limits::Populate, 1e7);
//	cplex.setParam(IloCplex::Param::MIP::Pool::Capacity, 100);
	cplex.setOut(env.getNullStream());
	cplex.setWarning(env.getNullStream());


	bool needs_rerun = true;
	vector< vector< vector<long> > > T; // set of trading cycles
	
	chrono::steady_clock::time_point begin;
	chrono::steady_clock::time_point end;

	begin = chrono::steady_clock::now();

	while (needs_rerun) {
		needs_rerun = false;

		if (cplex.solve()) {
			needs_rerun = true;
			// read trading cycle
			vector< vector<long> > tc;
			tc.resize(n);
			for (long i = 0; i < n; i++)
				tc[i].resize(m);
			for (long i = 0; i < n; i++) {	
				for (long j = 0; j < m; j++) {
					double val = cplex.getValue(x[i][j]);
					long val_l = lround(val);
					assert( abs(val - (float) val_l) < 0.1);
					tc[i][j] = val_l;
				}
			}

#if 0
			cout << "Found MINTC: " << endl;
			for (long i = 0; i < n; i++) {	
				for (long j = 0; j < m; j++) {
					cout <<  tc[i][j] << " ";
				}
				cout << endl;
			}
#endif


			T.push_back(tc);
	//		cout << "# Num TC's: " << T.size() << endl;

			// add constraints
			IloExpr *expr = new IloExpr(env);
			for (long i = 0; i < n; i++) {
				for (long j = 0; j < m; j++) {
					if (tc[i][j] > 0 && !cfg.mintc.crude) {
						(*expr) += (x[i][j] <= tc[i][j] - 1);
					}
			//		if (tc[i][j] == 0) {
			//			(*expr) += (x[i][j] != 0);
			//		}
					if (tc[i][j] < 0) {
						(*expr) += (x[i][j] >= tc[i][j] + 1);
					}
				}
			}
			model.add( (*expr) >= 1);
		}
	}

	end = chrono::steady_clock::now();
	cout << "# MINTCs: " << T.size() << endl;
	cout << "# MINTC search: " << ((float) chrono::duration_cast<chrono::milliseconds>(end - begin).count()) / 1000 << " s" << std::endl;

	return T;
}

long EEF::tc_weight(vector< vector<long> > tc, EEF_Config cfg) {
	if (cfg.mintc.mode == Mintc_mode::CONST) {
		return 1;
	}
	if (cfg.mintc.mode == Mintc_mode::NUM_AGENTS) {
		long agents = 0;
		for (long i = 0; i < n; i++) {
			bool involved = false;
			for (long j = 0; j < m; j++) {
				if (tc[i][j] != 0) {
					involved = true;
					break;
				}
			}
			if (involved)
				agents++;
		}

		// minimize the number of uninvolved agents, which is almost the same as maximizing
		// the number of involved agents except in the case when there's no applicable TC's
		// which we would rather have
		return n - agents;
	}
	if (cfg.mintc.mode == Mintc_mode::NUM_POSITIVE_AGENTS) {
		long agents = 0;
		for (long i = 0; i < n; i++) {
			long util = 0;
			for (long j = 0; j < m; j++) {
				util += u[i][j] * tc[i][j];
			}
			if (util > 0)
				agents++;
		}

		return agents;
	}
	else {
		cout << "Unsupported MINTC mode" << endl;
		exit(1);
	}
}
