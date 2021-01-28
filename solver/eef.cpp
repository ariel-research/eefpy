#include <cassert>
#include <vector>
#include <string>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <cmath>

#include <ilcplex/ilocplex.h>

#include "eef.h"
#include "trading_cycle.h"

#include "ilp_model/primary.h"
#include "ilp_model/efficiency/pareto.h"
#include "ilp_model/efficiency/mintc.h"
#include "ilp_model/efficiency/none.h"

using namespace std;

static float sec_timediff(chrono::steady_clock::time_point start, chrono::steady_clock::time_point end) {
		return ((float) chrono::duration_cast<chrono::milliseconds>(end - start).count()) / 1000 ;
}

vector< vector<long> > EEF::solve(EEF_Config cfg) {
	vector< vector<long> > pi;
	

	try {
		pi = solve_cplex(cfg);
	}
	catch (IloException &e) {
		cerr << "Concert exception caught: " << e.getMessage() << endl;
	}
	catch (...) {
		cerr << "Unknown exception caught" << endl;
	}
	
	return pi;
}

#define DISABLE_TRASH_AGENT() {if (cfg.trash_agent) {n--;}}
#define ENABLE_TRASH_AGENT() {if (cfg.trash_agent) {n++;}}

#define ALPHA_ACCURACY 0.001

vector<float> EEF::find_alpha_range(vector<bool> locked) {
	float alpha_min = 0;
	float alpha_max = 0; // highest non-infinity alpha value that's feasible
	bool set_min = false;
	bool set_max = false;

	// compute alpha_min

	for (long i = 0; i < n; i++) {
		if (locked[i])
			continue;

		/* pick all positive items for yourself and all negatives for the other */
		float self = 0;
		float other = 0;
		for (long j = 0; j < m; j++) {
			if (u[i][j] > 0)
				self += u[i][j];
			if (u[i][j] < 0)
				other += u[i][j];
		}

		float ratio = other  / self;
		if (!set_min) {
			alpha_min = ratio;
			set_min = true;
		}
		else
			alpha_min = max(alpha_min, ratio);


		/* pick smallest utility item for yourself, and all others for the other */
		self = 0;
		other = 0;
		long picked = -1;

		for (long j = 0; j < m; j++) {
			if (u[i][j] > 0) {
				if (picked == -1) {
					self = u[i][j];
					picked = j;
				}
				else if (self > u[i][j]) {
					self = u[i][j];
					picked = j;
				}
			}
		}

		assert(picked != -1 /* please make sure in your instances each item has at least one positive utility item */);
		for (long j = 0; j < m; j++) {
			if (u[i][j] > 0 && j != picked) {
				other += u[i][j];
			}
		}

		if (other == 0) {
			// alpha_min = alpha_max for this agent
			alpha_max = max(alpha_max, ratio);
			continue;
		}

		ratio = other / self;
		if (!set_max) {
			alpha_max = ratio;
			set_max = true;
		}
		else
			alpha_max = max(alpha_max, ratio);
	}

	vector<float> r{alpha_min, alpha_max};
	return r;
}

vector< vector<long> > EEF::solve_cplex(EEF_Config cfg) {
	// stats
	float total_sec = 0;
	float setup_sec = 0;
	float primary_sec = 0;
	float secondary_sec = 0;

	chrono::steady_clock::time_point total_start, total_end;
	chrono::steady_clock::time_point setup_start, setup_end;
	chrono::steady_clock::time_point primary_start, primary_end;
	chrono::steady_clock::time_point secondary_start, secondary_end;

	total_start = chrono::steady_clock::now();

	setup_start = chrono::steady_clock::now();



	/*************************************************************************************************************
	 *************************************************************************************************************
				Prebuilt LPs - configuration is done by the constructors
	 *************************************************************************************************************
	 *************************************************************************************************************/

	// PRIMARY LP used for finding nice allocations
	
	Primary_ILP ilp(*this, cfg);
	// load any precached trading cycles
	for (size_t i = 0; i < cached_tcs.size(); i++) {
		// TODO: fix endless loop due to add_tc adding the tc to the cache
		assert(false);
		ilp.add_tc(cached_tcs[i]);
	}
	ilp.cplex.extract(ilp.model);

	// SECONDARY LPs used for checking efficiency of allocations

	// if item trashing is enabled then we ignore the trash agent in efficiency considerations
	DISABLE_TRASH_AGENT();

	Pareto_Efficiency_ILP pareto_eff(*this, cfg);
	pareto_eff.cplex.extract(pareto_eff.model);

	MINTC_Efficiency_ILP mintc_eff(*this, cfg);
	mintc_eff.cplex.extract(mintc_eff.model);

	None_Efficiency_ILP none_eff(*this, cfg);
	none_eff.cplex.extract(none_eff.model);

	// we will only be using one of these efficiency ILPs
	Efficiency_ILP *all_eff_ilps[] = {&none_eff, &pareto_eff, &mintc_eff};
	size_t tmp = 0;
	if (cfg.efficiency == Efficiency_notion::PARETO) 
		tmp = 1;
	if (cfg.efficiency == Efficiency_notion::DYNAMIC_MINTC) 
		tmp = 2;

	// ugly hack to get a reference to the correct ILP
	Efficiency_ILP &eff_ilp = *all_eff_ilps[tmp];
	

	ENABLE_TRASH_AGENT();

	/*************************************************************************************************************
	 *************************************************************************************************************
				SOlVER LOGIC
	 *************************************************************************************************************
	 *************************************************************************************************************/

	srand(time(NULL));

	/* here we will store an allocation */
	vector< vector<long> > pi;
	pi.resize(n);
	for (long i = 0; i < n; i++)
		pi[i].resize(m);

	long m_max = 0;
	for (long mult: mu)
		m_max = max(m_max, mult);

	size_t num_iter = 0;
	bool needs_rerun = false;

	bool debug = true;
	bool found_alloc = true;
	bool print_debug_on_fail = true;

	setup_end = chrono::steady_clock::now();
	setup_sec = sec_timediff(setup_start, setup_end);

	/* Setup for EF_alpha minimization */
	size_t alpha_iter = 0;
	size_t alpha_phase = 0;

	bool alpha_opt_enabled = false;
	bool alpha_opt_running = false;
	bool alpha_accuracy_reached = false;

	for (Objective obj: ilp.obj_labels)
		if (obj == Objective::MIN_MAX_ALPHA)
			alpha_opt_enabled = true;


	/////////////////////
	DISABLE_TRASH_AGENT();
	/////////////////////

	// define search space for optimal alpha value in EF_alpha minimization mode
	// Invariant in all but the first two iterations:
	//     - there is no solution with alpha = alpha_min
	//     - there is a solution with alpha = alpha_max
	// ~> Determine alpha using the bisection method (up to some accuracy)
	// After this is done some a "bottleneck" agent determines alpha
	// for this agent we lock their alpha value and try to minimize alpha
	// for the remaining agents untill all agents are "locked"

	float infty = numeric_limits<float>::infinity();
	vector<float> alpha_current;
	vector<bool> alpha_locked;
	for (long i = 0; i < n; i++) {
		alpha_current.push_back(infty); // infty = alpha constraint not in model
		alpha_locked.push_back(false);
	}

	float alpha_min = 0;
	float alpha_max = 0; // highest non-infinity alpha value that's feasible
	float alpha = cfg.alpha;

	if (cfg.envy == Envy_notion::EF_alpha || alpha_opt_enabled) {
		if (has_negatives) {
			cout << "#     Warning: EF-Alpha range may be wrong due to negative utilities" << endl;
		}

		vector<float> range = find_alpha_range(alpha_locked);
		alpha_min = range[0];
		alpha_max = range[1] + ALPHA_ACCURACY;

		// start looking for a solution with this alpha
		if (alpha_opt_enabled)
			alpha = alpha_min;
	}

	/////////////////////
	ENABLE_TRASH_AGENT();
	/////////////////////


	if (alpha_opt_enabled && ilp.obj_labels[0] == Objective::MIN_MAX_ALPHA) {
		alpha_opt_running = true;
	}
	if (alpha_opt_enabled)
		print_debug_on_fail = false;

	if (true) {
		cout << "#     alpha_min = " << alpha_min << endl;
		cout << "#     alpha_max = " << alpha_max << endl;
		if (cfg.envy == Envy_notion::EF_alpha)
			cout << "#     alpha = " << alpha << endl;
		if (cfg.debug) {
			cout << "#     locked = ";
			for (long i = 0; i < n; i++)
				cout << alpha_locked[i];
			cout << endl;
		}
	}

alpha_rerun:

	if (alpha_opt_running) {
		alpha_iter++;
		alpha_phase++;
	}
		
	/////////////////////
	DISABLE_TRASH_AGENT();
	/////////////////////

	if (cfg.envy == Envy_notion::EF_alpha || alpha_opt_running) {
		// adapt alpha values of agents to the current alpha
		for (long i = 0; i < n; i++) {
			if (alpha_locked[i])
				continue;

			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;
				if (alpha_current[i] == infty && alpha == infty)
					continue;

				if (alpha_current[i] == infty)
					ilp.model.add(ilp.efa_constraints[i][j]);
				if (alpha == infty)
					ilp.model.remove(ilp.efa_constraints[i][j]);
				else
					for (long k = 0; k < m; k++)
						ilp.efa_constraints[i][j].setLinearCoef(ilp.x[i][k], alpha * u[i][k]);
			}

			alpha_current[i] = alpha;
		}
	} 

	/////////////////////
	ENABLE_TRASH_AGENT();
	/////////////////////

	needs_rerun = true;
	found_alloc = true; // it's a lie

	while (needs_rerun) {
		needs_rerun = false;
		num_iter++;

#if 0
		if (num_iter == 1) {
			cplex.setParam(IloCplex::Param::MIP::Limits::Populate, 100);
			cplex.setParam(IloCplex::Param::MIP::Pool::Capacity, 100);
		}
		else {
			cplex.setParam(IloCplex::Param::MIP::Limits::Populate, 1);
			cplex.setParam(IloCplex::Param::MIP::Pool::Capacity, 1);
		}
#endif

		primary_start = chrono::steady_clock::now();
//		ilp.cplex.populate();
		ilp.cplex.solve();

		primary_end = chrono::steady_clock::now();
		primary_sec += sec_timediff(primary_start, primary_end);

		int numsol = ilp.cplex.getSolnPoolNsolns();

		auto status = ilp.cplex.getStatus();
		if (numsol == 0) {
			if (print_debug_on_fail) {
				ilp.env.error() << "Failed to solve LP" << endl;
				ilp.env.error() << "Status = " << status << endl;
				ilp.env.error() << "Numsol = " << numsol << endl;
			}
			found_alloc = false;
			break;
		}

		//int rand_sol = rand() % numsol;
		//if (num_iter == 1) {
		//	cout << "Found " << numsol << " initial solutions" << endl;
		//	cout << "Picking solution " << rand_sol << endl;
		//}


		/* iterate over solutions that have been found */
//		for (int sol = 0; sol < numsol; sol++) {
		for (int sol = 0; sol < 1; sol++) {
		//	if (sol != rand_sol)
		//		continue;
			

			/* read allocation found by the main LP */
			for (long i = 0; i < n; i++) {	
				for (long j = 0; j < m; j++) {
					//double val = ilp.cplex.getValue(ilp.x[i][j], sol);
					double val = ilp.cplex.getValue(ilp.x[i][j]);
					long val_l = lround(val);

					assert( abs(val - (float) val_l) < 0.1);

					assert(val_l >= 0);
					pi[i][j] = val_l;
				}
			}



			/////////////////////
			DISABLE_TRASH_AGENT();
			/////////////////////
			
			// sanity checks for envyness notions
			if (cfg.envy == Envy_notion::EF)
				assert(alloc_is_ef(pi));
			if (cfg.envy == Envy_notion::EFX)
				assert(alloc_is_efx(pi));
			if (cfg.envy == Envy_notion::EF1)
				assert(alloc_is_ef1(pi));


			secondary_start = chrono::steady_clock::now();

			/* check if solution is efficient */
			// adapt item multiplicities
			if (cfg.trash_agent) {
				for (long j = 0; j < m; j++) {
					long mult = mu[j] - pi[n][j];
					eff_ilp.mult[j].setLB(mult);
					eff_ilp.mult[j].setUB(mult);
				}
			}

			auto tc = eff_ilp.check_allocation(pi);
			if (tc.size() > 0) {
				needs_rerun = true;
				ilp.add_tc(tc);
			}

			secondary_end = chrono::steady_clock::now(); 
			secondary_sec += sec_timediff(secondary_start, secondary_end);

			/////////////////////
			ENABLE_TRASH_AGENT();
			/////////////////////
		}
	}


	////////////////////////////////////////////////////
	// Done with main loop
	
	if (found_alloc && alpha_opt_enabled && !alpha_opt_running) {
		// add values of optimization goals before MIN_MAX_ALPHA as constraints and turn them off

		for (int i = 0; i < ilp.obj_labels.size(); i++) {
			int prio = ilp.obj_labels.size() - i;
			Objective ilp_objective = ilp.obj_labels[i];

			if (ilp_objective == Objective::MIN_MAX_ALPHA)
				break;

			ilp.obj_weights[i] = 0.0;
			float value = ilp.cplex.getValue(ilp.obj_expressions[i]);
			ilp.model.add(ilp.obj_expressions[i] == value);
		}

		ilp.update_objectives();
		alpha_opt_running = true;
		goto alpha_rerun;
	}

#if 1
	if (alpha_opt_running && !alpha_accuracy_reached) {
		//cout << "Found alloc = " << found_alloc << endl;

		// the actual alpha value for this allocation (disregarding locked agents)
		float real_alpha = -infty;

		if (found_alloc) {
			for (long i = 0; i < n; i++) {
				if (alpha_locked[i])
					continue;

				for (long j = 0; j < n; j++) {
					if (j == i)
						continue;
					long util = 0;
					long util2 = 0;
					for (long k = 0; k < m; k++) {
						util += u[i][k] * pi[i][k];
						util2 += u[i][k] * pi[j][k];
					}

					real_alpha = max(real_alpha, ((float) util2) / ((float) util));
				}
			}
		}


		//if (found_alloc)
		//	cout << "Set alpha = " << alpha << " got " << real_alpha << endl;
		//else
		//	cout << "Set alpha = " << alpha << " got no alloc" << endl;
		if (alpha_phase == 1) {
			if (found_alloc == false) {
				alpha = alpha_max;
				goto alpha_rerun;
			}
			else {
				// we are super happy because we found a solution with alpha = alpha_min
			}
		}
		else if (alpha_phase == 2) {
			if (found_alloc == false) {
				// only alpha = inf is possible
				alpha = infty;
				alpha_accuracy_reached = true;
				goto alpha_rerun;
			}
			else {
				alpha_max = real_alpha;
				alpha = (alpha_min + alpha_max) / 2.0;
				goto alpha_rerun;

			}
		}
		else {
			if (found_alloc) {
				alpha = real_alpha;
				if (alpha == alpha_max) {
					alpha_min = alpha_max;
					alpha_accuracy_reached = true;
				}
				alpha_max = alpha;
			}
			else {
				alpha_min = alpha;
			}

			if (fabs(alpha_min - alpha_max) < ALPHA_ACCURACY) {
				// we have enough accuracy now
				alpha_accuracy_reached = true;
			//	cout << "Accuracy reached, setting alpha = " << alpha_max << endl;
				alpha = alpha_max + ALPHA_ACCURACY/2.0;
				// now go find the allocation again
			}
			else {
				alpha = (alpha_min + alpha_max) / 2.0;
			}

			goto alpha_rerun;
		}
	}
#endif

	bool should_rerun = false;
	if (alpha_opt_running && cfg.iterated_alpha_locking) {
		// find bottleneck agent and lock their alpha value
		assert(found_alloc);

		/////////////////////
		DISABLE_TRASH_AGENT();
		/////////////////////

		bool some_agent_locked = false;
		for (long i = 0; i < n; i++) {
			if (alpha_locked[i])
				continue;

			bool to_be_locked = false;

			for (long j = 0; j < n; j++) {
				if (j == i)
					continue;

				float ui = 0;
				float uj = 0;

				for (long k = 0; k < m; k++) {
					ui += u[i][k] * pi[i][k];
					uj += u[i][k] * pi[j][k];
				}

				if (ui == 0) {
					if (uj > 0) {
						to_be_locked = true;
					}
				}
				else {
					if ((alpha - ALPHA_ACCURACY) * ui < uj)
						to_be_locked = true;
				}

				if (to_be_locked) {
				//	cout << "Locked agent " << i << " wrt " << j << endl;
				//	cout << "Alpha = " << alpha << " agent alpha = " << uj / ui << endl; 
					alpha_locked[i] = true;
					some_agent_locked = true;
					break;
				}
			}
			// lock only one agent
			if (some_agent_locked)
				break;
		}

		// make sure we don't go into an endless loop
		assert(some_agent_locked);

		// check if there are unlocked agents
		for (long i = 0; i < n; i++) {
			if (!alpha_locked[i])
				should_rerun = true;
		}

		if (should_rerun) {
			// do some cleanup
			alpha_phase = 0;
			alpha_accuracy_reached = false;


	//		// for alpha = inf we had to delete constraints, now add them back
	//		if (alpha == numeric_limits<float>::infinity()) {
	//			for (long i = 0; i < n; i++) {
	//				if (alpha_locked[i])
	//					continue;

	//				for (long j = 0; j < n; j++) {
	//					if (i == j)
	//						continue;
	//					ilp.model.add(ilp.efa_constraints[i][j]);
	//				}
	//			}
	//		}

			// find new range
			vector<float> range = find_alpha_range(alpha_locked);
			alpha_min = range[0];
			alpha_max = range[1] + ALPHA_ACCURACY;
			if (alpha < alpha_max)
				alpha_max = alpha;
			alpha = alpha_min;
		}

		/////////////////////
		DISABLE_TRASH_AGENT();
		/////////////////////
	}

	if (should_rerun) {
		goto alpha_rerun;
	}




	total_end = chrono::steady_clock::now();
	total_sec = sec_timediff(total_start, total_end);
	cout << "# Total time: " << total_sec << " s" << endl;
	cout << "# Setup time: " << setup_sec << " s (" << 100*setup_sec/total_sec << "%)" << endl; 
	cout << "# Primary LP time: " << primary_sec << " s (" << 100*primary_sec/total_sec << "%)" << endl; 
	cout << "# Secondary LP time: " << secondary_sec << " s (" << 100*secondary_sec/total_sec << "%)" << endl; 
		
	cout << "# Main LP iterations: " << num_iter << endl;
	cout << "# Alpha iterations: " << alpha_iter << endl;
	cout << "# Considered TCs: " << ilp.num_tcs << endl;
	double objval = ilp.cplex.getObjValue();
	cout << "# Objective value = " << objval << endl;

	long trashed_items = 0;
	if (cfg.trash_agent) {
		for (long tr: pi[n-1])
			trashed_items += tr;
	}
	cout << "# Trashed items: " << trashed_items << endl;

	vector<vector<long>> fail_alloc;
	if (!found_alloc)
		return fail_alloc;
	return pi;
}

// check if an allocation is pareto efficient
// (Only for external use. Internally we have a prebuild ILP)
bool EEF::alloc_is_pareto_efficient(vector< vector<long> > pi)
{
	EEF_Config cfg;

	Pareto_Efficiency_ILP pareto_eff(*this, cfg);
	pareto_eff.cplex.extract(pareto_eff.model);

	auto tc =  pareto_eff.check_allocation(pi);

	return tc.size() == 0;
}
