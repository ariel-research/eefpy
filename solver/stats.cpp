#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <stdlib.h>
#include <limits>

#include "eef.h"

using namespace std;

double EEF::swf_util(vector< vector<long> > allocation) {
	double swf = 0;
	for (long i = 0; i < n; i++) {
		long util = 0;
		for (long k = 0; k < m; k++) {
			util += u[i][k] * allocation[i][k];
		}
		swf += util;
	}

	return swf;
}

double EEF::geo_swf_nash(vector< vector<long> > allocation) {
	double swf = 1;
	double rt = 1.0 / n;
	for (long i = 0; i < n; i++) {
		long util = 0;
		for (long k = 0; k < m; k++) {
			util += u[i][k] * allocation[i][k];
		}
		swf *= pow(util, rt);
	}

	return swf;
}

double EEF::abs_envy(vector< vector<long> > allocation) {
	double envy = -numeric_limits<double>::infinity();
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < n; j++) {
			if (j == i)
				continue;
			long util = 0;
			long util2 = 0;
			for (long k = 0; k < m; k++) {
				util += u[i][k] * allocation[i][k];
				util2 += u[i][k] * allocation[j][k];
			}

			envy = max(envy, (double) (util2 - util));
		}
	}

	return envy;
}

double EEF::rel_envy(vector< vector<long> > allocation) {
	double envy = -numeric_limits<double>::infinity();
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < n; j++) {
			if (j == i)
				continue;
			long util = 0;
			long util2 = 0;
			for (long k = 0; k < m; k++) {
				util += u[i][k] * allocation[i][k];
				util2 += u[i][k] * allocation[j][k];
			}

			envy = max(envy, ((double) (util2 - util)) / ((double) util));
		}
	}

	return envy;
}

bool EEF::alloc_is_ef1(vector< vector<long> > allocation) {
	bool is_ef1 = true;
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < n; j++) {
			// does i envy j?
			long u_i = 0;
			long u_j = 0;

			for (long l = 0; l < m; l++) {
				u_i += u[i][l] * allocation[i][l];
				u_j += u[i][l] * allocation[j][l];
			}

			long best = 0;
			for (long l = 0; l < m; l++)
				if (allocation[j][l] > 0)
					best = u[i][l];
			for (long l = 0; l < m; l++)
				if (allocation[j][l] > 0)
					best = max(best, u[i][l]);

			if (u_i < u_j - best) {
				is_ef1 = false;
				break;
			}
		}
	}

	return is_ef1;
}

bool EEF::alloc_is_efx(vector< vector<long> > allocation) {
	bool is_efx = true;
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < n; j++) {
			// does i envy j?
			long u_i = 0;
			long u_j = 0;

			for (long l = 0; l < m; l++) {
				u_i += u[i][l] * allocation[i][l];
				u_j += u[i][l] * allocation[j][l];
			}

			long worst = 0;

			for (long l = 0; l < m; l++)
				if (allocation[j][l] > 0)
					worst = u[i][l];
			for (long l = 0; l < m; l++)
				if (allocation[j][l] > 0)
					worst = min(worst, u[i][l]);

			if (u_i < u_j - worst) {
				is_efx = false;
				break;
			}
		}
	}
	return is_efx;
}

bool EEF::alloc_is_ef(vector< vector<long> > allocation) {
	bool is_ef = true;
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < n; j++) {
			// does i envy j?
			long u_i = 0;
			long u_j = 0;

			for (long l = 0; l < m; l++) {
				u_i += u[i][l] * allocation[i][l];
				u_j += u[i][l] * allocation[j][l];
			}

			if (u_i < u_j) {
				is_ef = false;
				break;
			}
		}
	}
	return is_ef;
}

void EEF::print_allocation(vector< vector<long> > allocation) {
	if (allocation.size() == 0) {
		cout << "No allocation" << endl;
		return;
	}

	vector<size_t> max_col_width;
	max_col_width.resize(m);

	for (long i = 0; i < m; i++) 
		max_col_width[i] = 0;
	for (long i = 0; i < n; i++) {
		for (long j = 0; j < m; j++) {
			string to_print;
			to_print = to_string(allocation[i][j]);
			max_col_width[j] = max(max_col_width[j], to_print.length());
		}
	}


	cout << "Allocation: " << endl;
	for (long i = 0; i < n; i++) {
		//cout << i << ": ";
		for (long j = 0; j < m; j++) {
			string to_print;
			to_print = to_string(allocation[i][j]);
			size_t diff = max_col_width[j] - to_print.length();
			assert(diff < 100);

			for (size_t k = 0; k < diff; k++)
				cout << " ";

			cout << to_print << " ";
		}
		cout << endl;
	}

	cout << endl;
	cout << "# Allocation stats" << endl;
	cout << "#     Agents evaluation of allocation: " << endl;
	
	max_col_width.resize(n);
	for (long i = 0; i < n; i++) 
		max_col_width[i] = 0;

	for (long i = 0; i < n; i++) {
		for (long j = 0; j < n; j++) {
			long util = 0;
			for (long k = 0; k < m; k++) {
				util += u[i][k] * allocation[j][k];
			}
			string to_print = to_string(util);

			max_col_width[j] = max(max_col_width[j], to_print.length());
		}
	}

	for (long i = 0; i < n; i++) {
		cout << "#     ";
		for (long j = 0; j < n; j++) {
			long util = 0;
			for (long k = 0; k < m; k++) {
				util += u[i][k] * allocation[j][k];
			}

			string to_print;
			to_print = to_string(util);

			size_t diff = max_col_width[j] - to_print.length();
			assert(diff < 100);

			for (size_t k = 0; k < diff; k++)
				cout << " ";

			cout << to_print << " ";
		}
		cout << endl;
	}



	cout << "#" << endl;
	cout << "#     SWF_util = " << swf_util(allocation) << endl;
	cout << "#     mean SFW_util = " << swf_util(allocation) / n  << endl;
	cout << "#     geo util (norm. SWF_nash) = " << geo_swf_nash(allocation)  << endl;
	cout << "#     max_abs_envy = " << abs_envy(allocation) << endl;
	cout << "#     max_rel_envy = " << rel_envy(allocation) << endl;
	cout << "#     alpha = " << 1.0 + rel_envy(allocation) << endl;
	cout << "#     Allocation is EF = "  << (alloc_is_ef(allocation)  ? "true" : "false") << endl;
	cout << "#     Allocation is EFX = " << (alloc_is_efx(allocation) ? "true" : "false") << endl;
	cout << "#     Allocation is EF1 = " << (alloc_is_ef1(allocation) ? "true" : "false") << endl;
	cout << "#     Allocation is pareto efficient = " << (alloc_is_pareto_efficient(allocation) ? "true" : "false") << endl;

}

