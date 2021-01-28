#pragma once
#include "model.h"

class Primary_ILP : public ILP_Allocation_Model {
public:

	vector< vector <IloRange> > ef_constraints;
	vector< vector <IloRange> > efa_constraints;
	vector< vector <IloRange> > efaqp_constraints;
	vector< vector< vector<IloBoolVar> > > xefx;
	vector< vector< vector<IloBoolVar> > > xef1;

	vector<IloNumVar> alpha; // ILP variables for the allocation

	IloObjective objective;

	void set_objectives(vector<Objective> ilp_objectives);
	void update_objectives();

	IloObjective get_objective_from_enum(Objective ilp_objective);

	vector<Objective> obj_labels;
	IloNumExprArray obj_expressions;
	IloNumArray obj_weights;
	IloIntArray obj_priorities;
	IloNumArray obj_abs_tols;
	IloNumArray obj_rel_tols;

	Primary_ILP(EEF &, EEF_Config);

	void add_tc(vector< vector<long> > tc) override;
private:
	bool objectives_set = false;
	IloExpr *dynmintc_sum_weights = nullptr;
};
