#pragma once

#include <vector>

// stuff for divisible items
#define DIV_MULT 100
#define PRINT_PRECISION 2

using namespace std;

enum class Objective {
	NONE,
	MAX_SWF,
	MIN_SWF,
	MIN_MAX_ABS_ENVY,
	MIN_MAX_ABS_ENVY_OLD,
	MIN_EMPTY_AGENTS,
	MIN_TRASHED_ITEMS,
	MIN_TRASHED_UTIL,
	MIN_DOMTC_WEIGHT, // used internally for minimizing number/weight of dominating trading cycles
	MIN_MAX_ALPHA,
	MIN_MAX_ALPHA_QP
};

enum class Envy_notion {
	NONE,
	EF,
	EF1,
	EFX,
	EF_alpha,
	EF_alpha_qp
};

enum class Efficiency_notion {
	NONE,
	PARETO,
	DYNAMIC_MINTC_CRUDE_EQ0, // never used in the solver internally, only in the config (later switched to DYNAMIC_MINTC)
	DYNAMIC_MINTC,
	STATIC_MINTC
};
enum class Mintc_mode {
	CONST,
	NUM_AGENTS,
	NUM_POSITIVE_AGENTS,
	MIN_SET_OF_AGENTS,
	SUM_ENTRIES,
};

class EEF_Config {
public:
	Envy_notion envy = Envy_notion::EF;
	float alpha = 0.0; // alpha value for EF_alpha
	bool iterated_alpha_locking = false;
	Efficiency_notion efficiency = Efficiency_notion::PARETO;
	vector<Objective> objectives;

	
	struct {
		bool crude = false;
		bool eq0 = false;
		Mintc_mode mode = Mintc_mode::CONST;
	} mintc;

	bool trash_agent = false;
	bool tc_only_negatives = true;

	bool debug = false;
};


class EEF {
public:

	long n; // number of agents
	long m; // number of item groups

	vector<long> mu; // item multiplicities
	vector< vector<long> > u; // utilities matrix u[i][j] = utility of item j to agent i
	bool has_negatives = false;

	/* additional stuff for divisible items */
	// WARNING: we modify u and mu in order to support divisible items
	bool enable_div = false;
	vector<int> divisible;

	vector< vector<long> > solve(EEF_Config cfg);
	vector< vector< vector<long> > > cached_tcs;


	vector<float> find_alpha_range(vector<bool> locked);

	/* trading cycle analysis */
	vector< vector< vector<long> > > minimal_trading_cycles(EEF_Config cfg);
	vector< vector< vector<long> > > all_trading_cycles();



	/* stats */
	void print_allocation(vector< vector<long> > allocation);

	double swf_util(vector< vector<long> > allocation);
	double geo_swf_nash(vector< vector<long> > allocation);

	double abs_envy(vector< vector<long> > allocation);
	double rel_envy(vector< vector<long> > allocation);
	
	bool alloc_is_pareto_efficient(vector< vector<long> > pi);
	bool alloc_is_ef1(vector< vector<long> > pi);
	bool alloc_is_efx(vector< vector<long> > pi);
	bool alloc_is_ef(vector< vector<long> > pi);

	long tc_weight(vector< vector<long> > tc, EEF_Config cfg);
private:
	vector< vector<long> > solve_cplex(EEF_Config cfg);
};
