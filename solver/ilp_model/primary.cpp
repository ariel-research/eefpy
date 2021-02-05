#include "primary.h"

#define DISABLE_TRASH_AGENT() {if (cfg.trash_agent) {n--;}}
#define ENABLE_TRASH_AGENT() {if (cfg.trash_agent) {n++;}}

IloObjective Primary_ILP::get_objective_from_enum(Objective ilp_objective) {
	IloObjective ilo_objective(env);

	if (ilp_objective == Objective::NONE) {
		// do nothing
	}
	if (ilp_objective == Objective::MIN_MAX_ALPHA) {
		// placeholder; actual optimization done in solver loop through bisection
	}
	if (ilp_objective == Objective::MAX_SWF) {
		IloExpr *SWF_util = new IloExpr(env);
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++) {
				*SWF_util += x[i][j] * u[i][j];
			}
		}
		ilo_objective = IloMaximize(env, *SWF_util);
	}
	if (ilp_objective == Objective::MIN_SWF) {
		IloExpr *SWF_util = new IloExpr(env);
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++) {
				*SWF_util += x[i][j] * u[i][j];
			}
		}
		ilo_objective = IloMinimize(env, *SWF_util);
	}
	if (ilp_objective == Objective::MIN_MAX_ABS_ENVY) {
		IloNumVar abs_alpha = IloNumVar(env, -IloInfinity, IloInfinity, "abs_alpha");

		for (long i = 0; i < n; i++) {
			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;

				IloExpr *exp = new IloExpr(env);
				for (long k = 0; k < m; k++) {
					(*exp) += u[i][k] * x[i][k];
					(*exp) -= u[i][k] * x[j][k];
				}

				model.add((*exp) + abs_alpha >= 0);
			}
		}

		ilo_objective = IloMinimize(env, abs_alpha);
	}
	if (ilp_objective == Objective::MIN_MAX_ABS_ENVY_OLD) {
		IloExprArray *abs_envies = new IloExprArray(env, 0);

		/* abs envy */
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;

				IloExpr *exp_l = new IloExpr(env);
				IloExpr *exp_r = new IloExpr(env);
				for (long k = 0; k < m; k++) {
					(*exp_l) += u[i][k] * x[i][k];
					(*exp_r) += u[i][k] * x[j][k];
				}

				(*abs_envies).add((*exp_r) - (*exp_l));
			}
		}
		ilo_objective = IloMinimize(env, IloMax(*abs_envies));
	}
	if (ilp_objective == Objective::MIN_MAX_ALPHA_QP) {
		IloNumVarArray *alphas_qp = new IloNumVarArray(env, 0);

		/* abs envy */
		for (long i = 0; i < n; i++) {
			(*alphas_qp).add(alpha[i]);
		}

		ilo_objective = IloMinimize(env, IloMax(*alphas_qp));
	}
	if (ilp_objective == Objective::MIN_TRASHED_ITEMS) {
		IloExpr *num_trashed = new IloExpr(env);
		for (long j = 0; j < m; j++) {
			(*num_trashed) += x[n-1][j];
		}

		ilo_objective = IloMinimize(env, *num_trashed);
	}
	if (ilp_objective == Objective::MIN_TRASHED_UTIL) {
		IloExpr *util_trashed = new IloExpr(env);
		for (long j = 0; j < m; j++) {
			(*util_trashed) += -u[n-1][j]*x[n-1][j];
		}

		ilo_objective = IloMinimize(env, *util_trashed);
	}
	if (ilp_objective == Objective::MIN_EMPTY_AGENTS) {
		IloExpr *empty_agents = new IloExpr(env);
		for (long i = 0; i < n; i++) {
			IloExpr *util = new IloExpr(env);

			for (long j = 0; j < m; j++) {
				(*util) += u[i][j] * x[i][j];
			}

			(*empty_agents) += (*util) == 0;
		}

		ilo_objective = IloMinimize(env, *empty_agents);
	}

	return ilo_objective;
}

void Primary_ILP::set_objectives(vector<Objective> ilp_objectives) {
	// Use CPLEX's new way of defining a hierarchy of objectives
	assert(!objectives_set);
	objectives_set = true;

	for (int i = 0; i < ilp_objectives.size(); i++) {
		Objective ilp_objective = ilp_objectives[i];
		int prio = ilp_objectives.size() - i;

		IloObjective tmp_obj = get_objective_from_enum(ilp_objective);
		float weight = tmp_obj.getSense() == IloObjective::Maximize ? 1.0 : -1.0;

		obj_expressions.add(tmp_obj.getExpr());
		obj_weights.add(weight);
		obj_priorities.add(prio);
		obj_abs_tols.add(0.0); // no tolerance
		obj_rel_tols.add(0.0); // no tolerance
	}

	// set labels
	obj_labels = ilp_objectives;

	// create hierarchical objective
	objective = IloMaximize(env, IloStaticLex(env, obj_expressions, obj_weights, obj_priorities, obj_abs_tols, obj_rel_tols));
	model.add(objective);
}

// call this if you changed for example the objective expressions in obj_expressions
void Primary_ILP::update_objectives() {
	model.remove(objective);
	objective.end();

	objective = IloMaximize(env, IloStaticLex(env, obj_expressions, obj_weights, obj_priorities, obj_abs_tols, obj_rel_tols));
	model.add(objective);
}

Primary_ILP::Primary_ILP(EEF &tmp_I, EEF_Config tmp_cfg)
	: ILP_Allocation_Model(tmp_I, tmp_cfg),
	 obj_expressions(env), obj_weights(env), obj_priorities(env), obj_abs_tols(env), obj_rel_tols(env)
{
	/*************** setup envyness constraints ***************/
	
	/////////////////////
	DISABLE_TRASH_AGENT();
	/////////////////////

	// EF

	if (cfg.envy == Envy_notion::EF) {
		ef_constraints.resize(n);
		for (long i = 0; i < n; i++) {
			ef_constraints[i].resize(n);
			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;

				IloExpr *exp = new IloExpr(env);
				for (long k = 0; k < m; k++) {
					(*exp) += u[i][k] * x[i][k];
					(*exp) -= u[i][k] * x[j][k];
				}

				ef_constraints[i][j] = ((*exp) >= 0);
				model.add(ef_constraints[i][j]);
			}
		}
	}

	// EF-alpha
	bool alpha_opt_enabled = false;
	for (Objective obj: cfg.objectives)
		if (obj == Objective::MIN_MAX_ALPHA)
			alpha_opt_enabled = true;

	if (cfg.envy == Envy_notion::EF_alpha || alpha_opt_enabled) {
		// we'll add the alpha term before starting the main loop

		efa_constraints.resize(n);
		for (long i = 0; i < n; i++) {
			efa_constraints[i].resize(n);
			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;

				IloExpr *exp = new IloExpr(env);
				for (long k = 0; k < m; k++) {
					(*exp) += u[i][k] * x[i][k];
					(*exp) -= u[i][k] * x[j][k];
				}

				efa_constraints[i][j] = ((*exp) >= 0);
			}
		}
	}

	if (cfg.envy == Envy_notion::EF_alpha_qp) {
		alpha.resize(n);
		for (long i = 0; i < n; i++) {
			// the allocation vector consists of positive integer values
			alpha[i] = IloNumVar(env);
		}

		efaqp_constraints.resize(n);

		for (long i = 0; i < n; i++) {
			efaqp_constraints[i].resize(n);
			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;

				IloExpr *exp = new IloExpr(env);
				for (long k = 0; k < m; k++) {
					(*exp) += alpha[i] * u[i][k] * x[i][k];
					(*exp) -= u[i][k] * x[j][k];
				}

				efaqp_constraints[i][j] = ((*exp) >= 0);
				model.add(efaqp_constraints[i][j]);
			}
		}
	}

	// EFX

	if (cfg.envy == Envy_notion::EFX) {
		long u_max = u[0][0];
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++)
				u_max = max(u_max, u[i][j]);
		}
		long m_sum = 0;
		for (long j = 0; j < m; j++)
			m_sum += mu[j];	


		xefx.resize(n);
		for (long i = 0; i < n; i++) {
			xefx[i].resize(n);

			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;
				xefx[i][j].resize(2 * u_max + 1);

				for (long k = -u_max; k <= u_max; k++) {
					xefx[i][j][k + u_max] = IloBoolVar(env);
				}
			}
		}
	
		
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;

				for (long k = -u_max; k <= u_max; k++) {
					// constraint 10 from page 14
					
					vector<long> lek; // I_{a,<=k}
					vector<long> eqk; // I_{a,=k}

					for (long l = 0; l < m; l++) {
						if (u[i][l] == k)
							eqk.push_back(l);
						if (u[i][l] <= k)
							lek.push_back(l);
					}

					IloExpr *exp_l = new IloExpr(env);
					IloExpr *exp_r = new IloExpr(env);

					for (long l: lek)
						(*exp_l) += x[j][l];
					for (long w = -u_max; w <= k; w++)
						(*exp_r) += xefx[i][j][w + u_max];

					model.add( (*exp_l) <= m_sum * (*exp_r));

					// constraint 11 from page 15
					IloExpr *exp = new IloExpr(env);

					for (long l: eqk) {
						(*exp) += x[j][l];
					}
					model.add( xefx[i][j][k + u_max]  <= (*exp));
				}

				// constraint 12 page 15
				IloExpr *exp12 = new IloExpr(env);
				for (long k = -u_max; k <= u_max; k++)
					(*exp12) += xefx[i][j][k + u_max];

				model.add( (*exp12) <= 1);

				// constraint 13 page 15
				IloExpr *exp_l = new IloExpr(env);
				IloExpr *exp_m = new IloExpr(env);
				IloExpr *exp_r = new IloExpr(env);

				for (long k = 0; k < m; k++) {
					(*exp_l) += u[i][k] * x[i][k];
					(*exp_m) += u[i][k] * x[j][k];
				}
				for (long k = -u_max; k <= u_max; k++) {
					(*exp_r) += k * xefx[i][j][k + u_max];
				}

				model.add( (*exp_l) >= (*exp_m) - (*exp_r) );
			}
		}
	}

	// EF1

	if (cfg.envy == Envy_notion::EF1) {
		long u_max = u[0][0];
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++)
				u_max = max(u_max, u[i][j]);
		}
		long m_sum = 0;
		for (long j = 0; j < m; j++)
			m_sum += mu[j];	


		xef1.resize(n);
		for (long i = 0; i < n; i++) {
			xef1[i].resize(n);

			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;
				xef1[i][j].resize(2 * u_max + 1);

				for (long k = -u_max; k <= u_max; k++) {
					xef1[i][j][k + u_max] = IloBoolVar(env);
				}
			}
		}
	
		
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < n; j++) {
				if (i == j)
					continue;

				// make sure that agent i does not envy agent j (EF1)

				for (long k = -u_max; k <= u_max; k++) {
					// constraint 10 from page 14
					
					vector<long> gek; // I_{a,>=k}
					vector<long> eqk; // I_{a,=k}

					for (long l = 0; l < m; l++) {
						if (u[i][l] == k)
							eqk.push_back(l);
						if (u[i][l] >= k)
							gek.push_back(l);
					}

					IloExpr *exp_l = new IloExpr(env);
					IloExpr *exp_r = new IloExpr(env);

					for (long l: gek)
						(*exp_l) += x[j][l];
					for (long w = k; w <= u_max; w++)
						(*exp_r) += xef1[i][j][w + u_max];

					model.add( (*exp_l) <= m_sum * (*exp_r));

					// constraint 11 from page 15
					IloExpr *exp = new IloExpr(env);

					for (long l: eqk) {
						(*exp) += x[j][l];
					}
					model.add( xef1[i][j][k + u_max]  <= (*exp));
				}

				// constraint 12 page 15
				IloExpr *exp12 = new IloExpr(env);
				for (long k = -u_max; k <= u_max; k++)
					(*exp12) += xef1[i][j][k + u_max];

				model.add( (*exp12) <= 1);

				// constraint 13 page 15
				IloExpr *exp_l = new IloExpr(env);
				IloExpr *exp_m = new IloExpr(env);
				IloExpr *exp_r = new IloExpr(env);

				for (long k = 0; k < m; k++) {
					(*exp_l) += u[i][k] * x[i][k];
					(*exp_m) += u[i][k] * x[j][k];
				}
				for (long k = -u_max; k <= u_max; k++) {
					(*exp_r) += k * xef1[i][j][k + u_max];
				}

				model.add( (*exp_l) >= (*exp_m) - (*exp_r) );
			}
		}
	}

	/////////////////////
	ENABLE_TRASH_AGENT();
	/////////////////////

	/* set objective */

	if ((cfg.efficiency == Efficiency_notion::STATIC_MINTC || cfg.efficiency == Efficiency_notion::DYNAMIC_MINTC) && !cfg.mintc.eq0) {
		cfg.objectives.insert(cfg.objectives.begin(), Objective::MIN_DOMTC_WEIGHT);
	}

	if (cfg.objectives.size() > 0) {
		set_objectives(cfg.objectives);
	}

	// STATIC MINTC

	// make sure we are not using the "aliased" / placeholder efficiency notion
	assert(cfg.efficiency != Efficiency_notion::DYNAMIC_MINTC_CRUDE_EQ0);

	if (cfg.efficiency == Efficiency_notion::STATIC_MINTC) {
		vector< vector< vector<long> > > tcs = I.minimal_trading_cycles(cfg);
		for (auto tc: tcs)
			add_tc(tc);
	}


}

void Primary_ILP::add_tc(vector< vector<long> > tc) {
	/////////////////////
	DISABLE_TRASH_AGENT();
	/////////////////////

	if (cfg.efficiency != Efficiency_notion::DYNAMIC_MINTC || cfg.efficiency != Efficiency_notion::STATIC_MINTC || cfg.mintc.eq0 ) {
		ILP_Allocation_Model::add_tc(tc);
	}
	else {
		IloExpr *cannot_apply_tc = new IloExpr(env);

		assert(tc.size() == n);

		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++) {
				if (!cfg.tc_only_negatives || tc[i][j] < 0) {
					(*cannot_apply_tc) += (x[i][j] + tc[i][j] <= -1);
				}
			}
		}


		/* update LP objective */
		long weight = I.tc_weight(tc, cfg);
		assert(cfg.objectives.size() > 0 && cfg.objectives[0] == Objective::MIN_DOMTC_WEIGHT);

		obj_expressions[0] += weight * ((*cannot_apply_tc) == 0);
		update_objectives();

		num_tcs++;
	}

	//I.cached_tcs.push_back(tc);

	/////////////////////
	ENABLE_TRASH_AGENT();
	/////////////////////
}
