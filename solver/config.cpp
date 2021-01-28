#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <string>

#include "eef.h"
#include "config.h"

using namespace std;

enum {
	OPT_EFFICIENCY = 424242,
	OPT_ENVY,
	OPT_ALPHA,
	OPT_ITERALPHALOCK,
	OPT_OBJECTIVE,
	OPT_DEBUG,
	OPT_DIVISIBLES,
	OPT_CONFIG,
	OPT_MINTC_CRUDE,
	OPT_MINTC_EQ0,
	OPT_INCOMPLETE,
	OPT_MINTC_MODE,
	OPT_ANALYZE
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static void read_config_from_file(Arguments *arguments, char *file);

// options we understand
static struct argp_option options[] = {
	{"envy",       OPT_ENVY,       "NOTION", 0, "Specify envyness notion to use (ef, efx, ef1, ef-alpha)"},
	{"env",        OPT_ENVY,       "NOTION", OPTION_ALIAS, 0},
	{"alpha",      OPT_ALPHA,       "VALUE", 0, "Specify alpha value for EF-alpha"},
	{"iter-alpha-lock",  OPT_ITERALPHALOCK, 0, 0, "Minimize the alpha in EF-alpha, find bottleneck agent and lock their alpha"},
	{"efficiency", OPT_EFFICIENCY, "NOTION", 0, "Specify efficiency notion to use (pareto, mintc, none)"},
	{"eff",        OPT_EFFICIENCY, "NOTION", OPTION_ALIAS, 0},
	{"mintc_crude",OPT_MINTC_CRUDE,       0, 0, "Use a crude, but faster version of MINTCs"},
	{"mintc_eq0",  OPT_MINTC_EQ0,         0, 0, "Use MINTCs to find strictly pareto efficient allocations"},
	{"mintc_mode", OPT_MINTC_MODE,   "MODE", 0, "Specify in what way to use MINTCs"},
	{"objective",  OPT_OBJECTIVE,  "NOTION", 0, "Specify objective (none, max_swf, min_swf, min_max_abs_envy)"},
	{"obj",        OPT_OBJECTIVE,  "NOTION", OPTION_ALIAS, 0},
	{"debug",      OPT_DEBUG,             0, 0, "Enable debug output"},
	{"divisibles", OPT_DIVISIBLES,        0, 0, "Enable simulation of divisible items. TODO: probably broken right now"},
	{"incomplete", OPT_INCOMPLETE,        0, 0, "Enable incomplete allocations through the introduction of a trash agent"},
	{"config",     OPT_CONFIG,       "FILE", 0, "Read configuration from file. Not all options are supported"},
	{"analyse",    OPT_ANALYZE,      "FILE", OPTION_ARG_OPTIONAL, "Read allocation from file or stdin if no file is given"},
	{"analyze",    OPT_ANALYZE ,     "FILE", OPTION_ALIAS | OPTION_ARG_OPTIONAL | OPTION_HIDDEN, 0},
	{ 0 }
};
static struct argp argp = {options, parse_opt, 0, 0};


static Envy_notion str_to_envy(string str) {
	if (false)
		;
	else if (str == "none") {
		return Envy_notion::NONE;
	}
	else if (str == "ef") {
		return Envy_notion::EF;
	}
	else if (str == "ef1") {
		return Envy_notion::EF1;
	}
	else if (str == "efx") {
		return Envy_notion::EFX;
	}
	else if (str == "ef-alpha") {
		return Envy_notion::EF_alpha;
	}
	else if (str == "ef-alpha-qp") {
		return Envy_notion::EF_alpha_qp;
	}

	cout << "Unknown envy notion: " << str << endl;
	exit(1);
}

static Objective str_to_objective(string str) {
	if (false)
		;
	else if (str == "none") {
		return Objective::NONE;
	}
	else if (str == "max_swf") {
		return Objective::MAX_SWF;
	}
	else if (str == "min_swf") {
		return Objective::MIN_SWF;
	}
	else if (str == "min_max_abs_envy") {
		return Objective::MIN_MAX_ABS_ENVY;
	}
	else if (str == "min_max_abs_envy_old") {
		return Objective::MIN_MAX_ABS_ENVY_OLD;
	}
	else if (str == "min_empty_agents") {
		return Objective::MIN_EMPTY_AGENTS;
	}
	else if (str == "min_max_alpha") {
		return Objective::MIN_MAX_ALPHA;
	}
	else if (str == "min_max_alpha_qp") {
		return Objective::MIN_MAX_ALPHA_QP;
	}
	else if (str == "min_trashed_items") {
		return Objective::MIN_TRASHED_ITEMS;
	}
	else if (str == "min_trashed_util") {
		return Objective::MIN_TRASHED_UTIL;
	}

	cout << "Unknown objective: " << str << endl;
	exit(1);
}

static Mintc_mode str_to_mintcmode(string str) {
	if (false)
		;
	else if (str == "const") {
		return Mintc_mode::CONST;
	}
	else if (str == "num_agents") {
		return Mintc_mode::NUM_AGENTS;
	}
	else if (str == "num_positive_agents") {
		return Mintc_mode::NUM_POSITIVE_AGENTS;
	}
	else if (str == "min_set_of_agents") {
		return Mintc_mode::MIN_SET_OF_AGENTS;
	}
	else if (str == "sum_entries") {
		return Mintc_mode::SUM_ENTRIES;
	}

	cout << "Unknown MINTC mode: " << str << endl;
	exit(1);
}

static Efficiency_notion str_to_efficiency(string str) {
	if (false)
		;
	else if (str == "none") {
		return Efficiency_notion::NONE;
	}
	else if (str == "pareto") {
		return Efficiency_notion::PARETO;
	}
	else if (str == "mintc") {
		return Efficiency_notion::DYNAMIC_MINTC;
	}
	else if (str == "mintc0") {
		return Efficiency_notion::DYNAMIC_MINTC_CRUDE_EQ0;
	}
	else if (str == "dynamic_mintc") {
		return Efficiency_notion::DYNAMIC_MINTC;
	}
	else if (str == "static_mintc") {
		return Efficiency_notion::STATIC_MINTC;
	}

	cout << "Unknown efficiency notion: " << str << endl;
	exit(1);
}

static error_t parse_opt (int key, char *arg, struct argp_state *state){
	/* Get the input argument from argp_parse, which we
	 *      know is a pointer to our arguments structure. */
	Arguments *arguments = (Arguments *) state->input;

	switch (key)
	{
		case OPT_ENVY:
			arguments->eef_cfg.envy = str_to_envy(arg);
			cout << "# Using notion: " << arg << endl;
			break;
		case OPT_ALPHA:
			arguments->eef_cfg.alpha = stof(arg);
			break;
		case OPT_ITERALPHALOCK:
			arguments->eef_cfg.iterated_alpha_locking = true;
			break;
		case OPT_OBJECTIVE: 
			arguments->eef_cfg.objectives.push_back(str_to_objective(arg));
			cout << "# Using notion: " << arg << endl;
			break;
		case OPT_EFFICIENCY: 
			arguments->eef_cfg.efficiency = str_to_efficiency(arg);
			cout << "# Using notion: " << arg << endl;
			if (arguments->eef_cfg.efficiency == Efficiency_notion::DYNAMIC_MINTC_CRUDE_EQ0) {
				arguments->eef_cfg.efficiency = Efficiency_notion::DYNAMIC_MINTC;
				arguments->eef_cfg.mintc.eq0 = true;
				arguments->eef_cfg.mintc.crude = true;
			}
			break;
		case OPT_MINTC_CRUDE:
			arguments->eef_cfg.mintc.crude = true;
			break;
		case OPT_MINTC_EQ0:
			arguments->eef_cfg.mintc.eq0 = true;
			break;
		case OPT_MINTC_MODE:
			arguments->eef_cfg.mintc.mode = str_to_mintcmode(arg);
			break;
		case OPT_DIVISIBLES:
			arguments->divisibles = strcmp("true", arg) == 0;
			break;
		case OPT_INCOMPLETE:
			arguments->eef_cfg.trash_agent = true;
			break;
		case OPT_CONFIG:
			read_config_from_file(arguments, arg);
			break;
		case OPT_ANALYZE:
			arguments->analyze = true;
			if (arg != NULL) {
				arguments->alloc_file = string(arg);
			}
			break;
		case OPT_DEBUG:
			arguments->eef_cfg.debug = true;
			break;
		case ARGP_KEY_ARG:
			if (state->arg_num >= 1)
				/* Too many arguments. */
				argp_usage (state);

			break;
		case ARGP_KEY_END:
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

Arguments read_arguments(int argc, char**argv) {
	Arguments arguments;

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	return arguments;
}

static vector<string> split(const string& str, const string& delim)
{
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos-prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	}
	while (pos < str.length() && prev < str.length());
	return tokens;
}


static void read_config_from_file(Arguments *arguments, char *file) {
	ifstream fin;
	fin.open(file);
	
	string line;
	while (getline(fin, line)) {
		if (line[0] == '#' || line.size() == 0)
			continue;

		// for some reason "\r\n" is not handled correctly
		if (!line.empty() && line[line.size() - 1] == '\r')
		    line.erase(line.size() - 1);

		vector<string> config = split(line, " ");

		if (config.size() == 0)
			continue;
		if (false)
			;

		else if (config[0] == "objective") {
			arguments->eef_cfg.objectives.push_back(str_to_objective(config[1]));
		}

		else if (config[0] == "envy_notion") {
			arguments->eef_cfg.envy = str_to_envy(config[1]);
		}

		else if (config[0] == "efficiency_notion") {
			arguments->eef_cfg.efficiency = str_to_efficiency(config[1]);
		}

		else {
			cout << "Unknown option: " << config[0] << endl;
			exit(1);
		}
	}
}
