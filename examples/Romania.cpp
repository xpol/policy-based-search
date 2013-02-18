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

#include "Romania.hpp"
#include "bestfirstsearch.hpp"

#include <iostream>
#include <algorithm>
#include <boost/heap/d_ary_heap.hpp>
#include <unordered_set>
#include <functional>

using namespace jsearch;
using namespace std;

typedef Romania::state State;
typedef Romania::node Node;

template <class charT, class traits>
basic_ostream<charT, traits>& print(Node const &NODE, basic_ostream<charT, traits>& os);


// Create template aliases that specify node evaluation.
template <typename Traits>
using CostFunction = AStar<Traits, EuclideanDistance>;

template <typename Traits>
using TieBreaker = LowH<Traits, EuclideanDistance>;

// Specify the heap used for the frontier, its comparator and the map used for fast look-up.
template <typename T>
using PriorityQueue = boost::heap::d_ary_heap<T, boost::heap::mutable_<true>, boost::heap::arity<2>>;

template <typename Traits, template <typename Traits> class CostFunction>
using Comparator = TiebreakingComparator<Traits, CostFunction, TieBreaker>;


int main(int, char **)
{
	State const INITIAL("Arad");
	Problem<Romania, Distance, Neighbours, Visit, GoalTest> const BUCHAREST(INITIAL); // The problem is to get to Bucharest.

	try
	{
		auto const SOLUTION(jsearch::recursive_best_first_search<CostFunction, TieBreaker, PriorityQueue>(BUCHAREST));
		
		print(SOLUTION, cout);
		cout << ": " << SOLUTION->path_cost() << "\n";
	}
	catch (goal_not_found const &ex)
	{
		cout << "No path from " << INITIAL << " to the goal could be found!\n";
	}
}


template <class charT, class traits>
basic_ostream<charT, traits>& print(Node const &NODE, basic_ostream<charT, traits>& os)
{
	
	if(NODE->parent())
	{
		print(NODE->parent(), os);
		os << " => ";
	}
	
	os << NODE->state();
	
	return os;
}
