#include "model.h"

ILP_Allocation_Model::ILP_Allocation_Model(EEF &tmp_I, EEF_Config tmp_cfg)
	: model(env), cplex(env), I(tmp_I), cfg(tmp_cfg), n(I.n), m(I.m), mu(I.mu), u(I.u)
{
	/* setup variables */

	x.resize(n);
	for (long i = 0; i < n; i++) {
		x[i].resize(m);
		for (long j = 0; j < m; j++) {
			// the allocation vector consists of positive integer values
			x[i][j] = IloIntVar(env, 0, mu[j], ("x_" + to_string(i) + "_" + to_string(j)).c_str());
		}
	}

	/* setup basic constraints */

	/* all items are assigned */
	for (long j = 0; j < m; j++) {
		IloExpr *exp = new IloExpr(env);
		for (long i = 0; i < n; i++) {
			(*exp) += x[i][j];
		}

		mult.push_back((*exp) == mu[j]);
		model.add(mult[j]);
	}

//	cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap , 0.0);
//	cplex.setParam(IloCplex::Param::MIP::Tolerances::AbsMIPGap , 0.0);
//	cplex.setParam(IloCplex::Param::Emphasis::Numerical, true);
//	cplex.setParam(IloCplex::Param::Emphasis::MIP, 2); // emphasis on optimality
//	ofstream log_file("cplex_log.txt");
//	cplex.setOut(log_file);

	cplex.setOut(env.getNullStream());
	cplex.setWarning(env.getNullStream());
}

void ILP_Allocation_Model::add_tc_paper_version(vector< vector<long> > tc) {
	long m_max = 0;
	for (long mult: mu)
		m_max = max(m_max, mult);
		
	vector< vector<IloIntVar *> > y;
	y.resize(n);
	for (long i = 0; i < n; i++) {
		y[i].resize(m);
		for (long j = 0; j < m; j++) {
			y[i][j] = new IloBoolVar(env, "");
		}
	}

	IloExpr *sum_of_ys = new IloExpr(env);
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < m; j++) {
			(*sum_of_ys) += (*y[i][j]);
		}
	}

	model.add((*sum_of_ys) >= 1);


	for (long i = 0; i < n; i++) {
		for (long j = 0; j < m; j++) {
			if (!cfg.tc_only_negatives || tc[i][j] < 0) {
				model.add(x[i][j] + tc[i][j] <= -1 + (1 - (*y[i][j]))*(2*m_max+1));
			}
			else {
				y[i][j]->setUB(0);
			}
		}
	}
}

void ILP_Allocation_Model::add_tc_lazy_cplex_version(vector< vector<long> > tc) {
	IloExpr *cannot_apply_tc = new IloExpr(env);

	for (long i = 0; i < n; i++) {
		for (long j = 0; j < m; j++) {
			if (!cfg.tc_only_negatives || tc[i][j] < 0) {
				(*cannot_apply_tc) += (x[i][j] + tc[i][j] <= -1);
			}
		}
	}

	model.add(*cannot_apply_tc >= 1);
}

void ILP_Allocation_Model::add_tc(vector< vector<long> > tc) {
	assert(tc.size() == n);
	//add_tc_lazy_cplex_version(tc);
	add_tc_paper_version(tc);
	
	num_tcs++;
}


