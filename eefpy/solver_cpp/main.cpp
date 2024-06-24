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
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cassert>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <iomanip>

#include "eef.h"
#include "config.h"

using namespace std;

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

static vector<string> get_split_line(istream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line[0] == '#' || line.size() == 0)
			continue;

		// for some reason "\r\n" is not handled correctly
		if (!line.empty() && line[line.size() - 1] == '\r')
		    line.erase(line.size() - 1);

		return split(line, " ");
	}
	vector<string> empty;
	return empty;
}

static vector<long> get_row(istream &stream) {
	vector<string> tmp = get_split_line(stream);
	vector<long> row;
	row.resize(tmp.size());

	for (size_t i = 0; i < tmp.size(); i++) {
		row[i] = stol(tmp[i]);
	}

	return row;
}

static void read_instance(EEF &I, Arguments &args) {
	vector<long> row = get_row(cin);
	if (row.size() != 2) {
		cout << "Expected number of agents followed by number of item types" << endl;
		exit(1);
	}

	I.n = row[0];
	I.m = row[1];

	if (I.n == 0 || I.m == 0)
		return;


	I.u.resize(I.n);
	for (long i = 0; i < I.n; i++) {
		I.u[i] = get_row(cin);
		if (I.u[i].size() != I.m) {
			cout << "Insufficient number of utilities provided for agent " << i + 1 << endl;
			exit(1);
		}
	}

	I.mu = get_row(cin);
	if (I.mu.size() != I.m) {
		cout << "Insufficient number of item multiplicities provided" << endl;
		exit(1);
	}

	if (args.eef_cfg.trash_agent) {
		cout << "# Adding trash agent" << endl;
		I.n++;
		I.u.resize(I.n);
		I.u[I.n-1].resize(I.m);
		for (long j = 0; j < I.m; j++) {
			long max_util = 1;
			for (long i = 0; i < I.n; i++) {
				max_util = max(I.u[i][j], max_util);
			}

			I.u[I.n-1][j] = -max_util;
		}
	}

	vector<int> divisible;
	if (args.divisibles) {
		divisible.resize(I.m);
		for (long j = 0; j < I.m; j++) {
			cin >> divisible[j];

			if (divisible[j] == 1) {
				I.mu[j] *= DIV_MULT;
			}
			else {
				for (long i = 0; i < I.n; i++)
					I.u[i][j] *= DIV_MULT;
			}
		}
	}
}

vector< vector<long> > get_allocation(long n, long m, istream &stream) {
	vector< vector<long> > pi;
	pi.resize(n);

	vector<string> header = get_split_line(stream);
	if (header.size() == 0 || header[0] != "Allocation:") {
		cout << "Missing allocation header" << endl;
		exit(1);
	}

	for (long i = 0; i < n; i++) {
		pi[i] = get_row(stream);
		if (pi[i].size() != m) {
			cout << "Insufficient allocation length for agent " << i + 1 << endl;
			exit(1);
		}
	}

	return pi;
}

int main(int argc, char **argv) {
	// start measuring time
	//TIME_start = chrono::steady_clock::now(); 

	Arguments args = read_arguments(argc, argv);

	if (args.divisibles)
		cout << "Enabling simulation of divisible items by splitting them into " << DIV_MULT << " indivisible items" << endl;

	// little performance boost for iostream
	std::ios::sync_with_stdio(false);


	// parse the input
	EEF I;
	read_instance(I, args);

	cout << "# num_agents: " << I.n << endl;
	cout << "# num_types: " << I.m << endl;
	long total_items = 0;
	for (long i = 0; i < I.m; i++) {
		total_items += I.mu[i];
	}
	cout << "# total_items: " << total_items << endl;
	cout << "# avg_items: " << total_items / I.m << endl;

	I.has_negatives = false;
	for (long i = 0; i < I.n; i++) {
		for (long j = 0; j < I.m; j++) {
			if (I.u[i][j] < 0) {
				I.has_negatives = true;
				break;
			}
		}
	}
	if (I.has_negatives) {
		cout << "#     Warning: Utility matrix has negative utilities!" << endl;
		cout << "#              The notions of EFX, EF1 and relative envy may not function as desired" << endl;
	}


	if (args.analyze == false) {
		vector< vector<long> > allocation = I.solve(args.eef_cfg);
		if (args.eef_cfg.trash_agent) {
			I.n--;
		}
		I.print_allocation(allocation);
	}
	else {
		if (args.alloc_file.length() != 0) {
			filebuf fb;
			if (fb.open(args.alloc_file.c_str(), ios::in)) {
				istream alloc_stream(&fb);

				vector< vector<long> > allocation = get_allocation(I.n, I.m, alloc_stream);
				I.print_allocation(allocation);
			}
			else {
				cout << "Failed to open alloc file" << endl;
				exit(1);
			}
		}
		else {
			vector< vector<long> > allocation = get_allocation(I.n, I.m, cin);
			I.print_allocation(allocation);
		}
	}

	return 0;
}

