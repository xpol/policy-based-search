/*
    Romania.cpp: Loader.
    Copyright (C) 2012  Jeremy W. Murphy <jeremy.william.murphy@gmail.com>

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

#include "Romania.h"
#include "search.h"

#include <iostream>
#include <algorithm>
#include <boost/heap/pairing_heap.hpp>

using namespace jsearch;
using namespace std;

typedef Romania::state State;
typedef Romania::node Node;


template <class charT, class traits>
basic_ostream<charT, traits>& print(Node const &NODE, basic_ostream<charT, traits>& os);


template <typename Traits>
using AStarRomania = DefaultAStar<Traits, EuclideanDistance>;

template <typename T>
using PriorityQueue = boost::heap::pairing_heap<T, boost::heap::compare<AStarRomania<Romania>>>;


int main(int, char **)
{
	State const INITIAL("Arad");
	Problem<Romania, Distance, Neighbours, Visit, GoalTest> const BUCHAREST(INITIAL); // The problem is to get to Bucharest.

	try
	{
		auto const SOLUTION = jsearch::best_first_search<PriorityQueue>(BUCHAREST);
		
		print(SOLUTION, cout);
		cout << ": " << SOLUTION.path_cost << "\n";
	}
	catch (goal_not_found const &ex)
	{
		cout << "No path from " << INITIAL << " to the goal could be found!\n";
	}
}


template <class charT, class traits>
basic_ostream<charT, traits>& print(Node const &NODE, basic_ostream<charT, traits>& os)
{
	
	if(NODE.parent)
	{
		print(*NODE.parent, os);
		os << " => ";
	}
	
	os << NODE.state;
	
	return os;
}
