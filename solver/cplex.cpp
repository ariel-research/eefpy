#include <ilcplex/ilocplex.h>
#include <fstream>

#include "graph.h"
#include "read_sol.h"

static void dfs_constraint(Vertex *v, vector<Vertex *> &P, IloEnv &env, IloModel &model) {
	if (v->visited)
		return;

	// check that adding v does not break the induced path
	size_t count = 0;
	for (auto edge: v->edges)  {
		if (edge.first->visited)
			count++;
	}

	if (P.size() != 0 && count != 1)
		return;

	P.push_back(v);
	v->visited = true;

	if (P.size() == 4) {
		// prevent duplicate constraints
		if (P[0]->id > P[3]->id)
			goto end;
		IloExpr *exp = new IloExpr(env);

		for (size_t i = 0; i < 4; i++) {
			(*exp) += *(P[i]->cvar);
		}

		for (auto edge: P[0]->edges) {
			Vertex *b = edge.first;
			assert(!b->marked);
			b->marked = true;
		}

		for (auto edge: P[3]->edges) {
			Vertex *b = edge.first;

			if (b->marked) {
				*exp += 1.0 - *b->cvar;
			}
		}

		for (auto edge: P[0]->edges) {
			Vertex *b = edge.first;
			b->marked = false;
		}

		IloRange *constraint = new IloRange(env, 1.0, *exp, +IloInfinity);
		model.add(*constraint);
	}
	else {
		for (auto edge: v->edges)  {
			Vertex *a = edge.first;
			dfs_constraint(a, P, env, model);
		}
	}

end:
	P.pop_back();
	v->visited = false;
}

void cplex_solve_helper(Graph &G) {
	IloEnv env;
	IloModel model(env);

	/* objective expression */
	IloExpr objective_expr(env);

	/* create a variable for each vertex */
	for (Vertex *v: G.V) {
		v->cvar = new IloBoolVar(env, "");
		objective_expr += *v->cvar;
	}

	/* minimize objective expression */
	model.add(IloMinimize(env, objective_expr));

	/* create restircted P4 constraints */
	for (Vertex *a: G.V) {
		vector<Vertex *> P;
		assert(!a->visited);
		dfs_constraint(a, P, env, model);
	}
	
	ofstream log_file("cplex_log.txt");
	IloCplex cplex(model);
	cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap , 0.0);
	cplex.setParam(IloCplex::Param::MIP::Tolerances::AbsMIPGap , 0.0);
	cplex.setParam(IloCplex::Param::Emphasis::Numerical, true);
//	cplex.setParam(IloCplex::Param::Emphasis::MIP, 2); // emphasis on optimality

	cplex.setOut(log_file);
	//cplex.exportModel("cplex_file.lp");
	cout << "# Done building model" << endl;

	if (!cplex.solve()) {
		env.error() << "Failed to solve LP" << endl;
		throw(-1);
	}

	/* query results */
	list<Vertex *> listV(G.V);
	for (Vertex *v: listV) {
		IloBoolVar v_var = *v->cvar;
		double val = cplex.getValue(v_var);

		if (val > 0.9) {
			add_to_vds(G, v);
		}
		else assert(val < 0.1);
	}
		

	env.end();
}

void cplex_solve(Graph &G) {
	list<Vertex *> solution;

	try {
		cplex_solve_helper(G);
	}
	catch (IloException &e) {
		cerr << "Concert expection caught: " << e << endl;
	}
	catch (...) {
		cerr << "Unknown exception caught" << endl;
	}

	read_sol(G, solution);

	cout << "# VDS size = " << solution.size() << endl;

	for (Vertex *v: solution) {
		cout << v->name << endl;
	}

}
