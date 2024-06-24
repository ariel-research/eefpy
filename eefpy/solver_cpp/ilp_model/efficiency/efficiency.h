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
#include "../model.h"

/* Class of ILPs that compute a trading cycle that serves as a witness
 * for not having some given efficiency property
 */

class Efficiency_ILP : public ILP_Allocation_Model {
public:

	Efficiency_ILP(EEF &, EEF_Config);

	// return a TC which should be added to the ILP or a vector of size 0 if no TC is to be added
	virtual vector< vector<long> > check_allocation(vector< vector<long> > pi) = 0;
};
