#include "mintc.h"

MINTC_Efficiency_ILP::MINTC_Efficiency_ILP(EEF &tmp_I, EEF_Config tmp_cfg)
	: Efficiency_ILP(tmp_I, tmp_cfg)
{
	noworseoff.reserve(n);

	x_ref.resize(n);

	for (long i = 0; i < n; i++) {
		x_ref[i].resize(m);
		for (long j = 0; j < m; j++) {
			// the allocation vector consists of positive integer values
			x_ref[i][j] = IloIntVar(env, 0, IloIntMax, ("x_" + to_string(i) + "^" + to_string(j)).c_str());
		}
	}

	/* each agent is not worse off */
	vector<IloRange> noworseoff;
	noworseoff.reserve(n);
	for (long i = 0; i < n; i++) {
		IloExpr *exp = new IloExpr(env);
		for (long j = 0; j < m; j++) {
			(*exp) += u[i][j] * (x[i][j] - x_ref[i][j]);
		}

		noworseoff.push_back((*exp) >= 0);
		model.add(noworseoff[i]);
	}

	/* sum of utilities has to increase by at least 1 */
	IloExpr *domination_exp = new IloExpr(env);

	for (long i = 0; i < n; i++) {
		for (long j = 0; j < m; j++) {
			*domination_exp += u[i][j] * (x[i][j] - x_ref[i][j]);
		}
	}

	model.add(*domination_exp >= 1);

	/* minimize the difference to the reference allocation - this way we find MINTCs */

	IloExpr *mintc_tc_diff = new IloExpr(env);
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < m; j++) {
			*mintc_tc_diff += IloAbs(x[i][j] - x_ref[i][j]);
		}
	}
	objective = IloMinimize(env, *mintc_tc_diff);
	model.add(objective);
}

vector< vector<long> > MINTC_Efficiency_ILP::check_allocation(vector< vector<long> > pi) {
	// store the current allocation in the x_ref variables
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < m; j++) {
			x_ref[i][j].setLB(pi[i][j]);
			x_ref[i][j].setUB(pi[i][j]);
		}
	}

	vector< vector<long> > tc;

	if (cplex.solve()) {
		IloExpr *mintc_not_dom = new IloExpr(env);

		tc.resize(n);
		for (long i = 0; i < n; i++)
			tc[i].resize(m);
		for (long i = 0; i < n; i++) {	
			for (long j = 0; j < m; j++) {
				double val = cplex.getValue(x[i][j]);
				long val_l = lround(val);
				assert( abs(val - (float) val_l) < 0.1);
				tc[i][j] = val_l - pi[i][j];
			}
		}

		// we do not need to add constraints to this model, as the primary ILP
		// will exclude applicability of any dominated TC
		if (cfg.mintc.eq0)
			return tc;

		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++) {
				long delta = tc[i][j];

				if (delta > 0 && !cfg.mintc.crude) {
					(*mintc_not_dom) += (x[i][j] - x_ref[i][j] <= delta - 1);
				}
				if (delta < 0) {
					(*mintc_not_dom) += (x[i][j] - x_ref[i][j] >= delta + 1);
				}
			}
		}
		
		/* make sure we do not later find TC's that are not minimal */
		model.add( (*mintc_not_dom) >= 1);
	}

	return tc;
}
