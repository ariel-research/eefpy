/****************************************************************************
*****************************************************************************
*HiMEFAF: High-Multiplicity Efficient Fair Allocations Finder 
*Copyright (C) 2020 Aleksander Figiel
*****************************************************************************
*This file is a part of HiMEFAF: High-Multiplicity Efficient Fair Allocations
*Finder 
*
*HiMEFAF is free software: you can redistribute it
*and/or modify it under the terms of the GNU General Public
*License as published by the Free Software Foundation, either
*version 3 of the License, or (at your option) any later
*version.
*
*This program is distributed in the hope that it will be
*useful, but WITHOUT ANY WARRANTY; without even the implied
*warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*PURPOSE.  See the GNU General Public License for more
*details.
*
*You should have received a copy of the GNU General Public
*License along with this program.  If not, see
*<https://www.gnu.org/licenses/>
*****************************************************************************
****************************************************************************/
#include "pareto.h"

Pareto_Efficiency_ILP::Pareto_Efficiency_ILP(EEF &tmp_I, EEF_Config tmp_cfg)
	: Efficiency_ILP(tmp_I, tmp_cfg)
{
	noworseoff.reserve(n);

	/* each agent is not worse off */
	for (long i = 0; i < n; i++) {
		IloExpr *exp = new IloExpr(env);
		long to_beat = 0; // this will be changed for each solution that we want to check
		for (long j = 0; j < m; j++) {
			(*exp) += u[i][j] * x[i][j];
		}

		noworseoff.push_back((*exp) >= to_beat);

		model.add(noworseoff[i]);
	}

	/* sum of utilities has to increase by at least 1 */
	IloExpr domination_exp(env);
	{
		long to_beat = 0; // this will be changed for each solution that we want to check
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++) {
				domination_exp += u[i][j] * x[i][j];
			}
		}
		dom_cstr = domination_exp >= to_beat;
	}
	model.add(dom_cstr);
}

vector< vector<long> > Pareto_Efficiency_ILP::check_allocation(vector< vector<long> > pi) {
	/* adapt model to reference allocation pi */

	// each agent is not worse off
	for (long i = 0; i < n; i++) {
		long to_beat = 0;
		for (long j = 0; j < m; j++) {
			to_beat += u[i][j] * pi[i][j];
		}

		noworseoff[i].setLB(to_beat);
	}

	// sum of utilities has to increase by at least 1*/
	{
		long to_beat = 1;
		for (long i = 0; i < n; i++) {
			for (long j = 0; j < m; j++) {
				to_beat += u[i][j] * pi[i][j];
			}
		}
		dom_cstr.setLB(to_beat);
	}
	

	vector< vector<long> > tc;

	if (cplex.solve()) {
		/* tcinating allocation found */

		tc.resize(n);
		for (long i = 0; i < n; i++)
			tc[i].resize(m);
		for (long i = 0; i < n; i++) {	
			for (long j = 0; j < m; j++) {
				double val = cplex.getValue(x[i][j]);
				long val_l = lround(val);
				assert(val_l >= 0);
				assert( abs(val - (float) val_l) < 0.1);
				tc[i][j] = val_l - pi[i][j];
			}
		}
	}

	return tc;
}
