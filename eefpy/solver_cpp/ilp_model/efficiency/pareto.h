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
#pragma once
#include "efficiency.h"

class Pareto_Efficiency_ILP : public Efficiency_ILP {
public:

	Pareto_Efficiency_ILP(EEF &, EEF_Config);

	// return a TC which serves as a a witness to the reference allocation not being pareto efficient
	vector< vector<long> > check_allocation(vector< vector<long> > pi);

private:
	vector<IloRange> noworseoff;
	IloRange dom_cstr;
};
