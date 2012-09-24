/*
    TSP.cpp: Travelling Salesman Problem demonstration.
    Copyright (C) 2012  Jeremy Murphy <jeremy.william.murphy@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "TSP.h"
#include "problem.h"
#include "search.h"

#include <iostream>
#include <algorithm>

using namespace std;
using namespace jsearch;

int main(int , char **)
{
	TSP::state const i;

	Problem<TSP, EdgeCost, HigherCostValidEdges, AppendEdge, ValidTour> const mona_lisa(i);
	Evaluation<MinimalFeasibleTour> const eval;
	
	TSP::node const solution = jsearch::search(mona_lisa, eval, true);

	cout << "( ";
	for_each(begin(solution.state), end(solution.state), [](vector<index>::const_reference &E)
	{
		cout << E << " ";
	});
	cout << ")" << endl;

	return 0;
}
