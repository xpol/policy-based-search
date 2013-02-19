/*
    random.cpp: Random loader.
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

#include "random.hpp"
#include "problem.hpp"
#include "bestfirstsearch.hpp"
#include "gg.hpp"

#include <sstream>
#include <unordered_set>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <string>
#include <iterator>
#include <locale>
#include <type_traits>
#include <cassert>
#include <cmath>
#include <boost/graph/graphviz.hpp>
#include <boost/heap/d_ary_heap.hpp>
#include <unordered_map>
#include <fstream>

using namespace std;
using namespace jsearch;

typedef Random::state State;
typedef Random::node Node;

string backtrace(Node const &NODE);
void init(int argc, char **argv);


// Create template aliases that specify node evaluation.
template <typename Traits>
using CostFunction = Dijkstra<Traits>;

template <typename T>
using PriorityQueue = boost::heap::d_ary_heap<T, boost::heap::mutable_<true>, boost::heap::arity<2>>;

template <typename T>
using ClosedList = std::unordered_set<T>;

template <typename Traits>
using Comparator = SimpleComparator<Traits, CostFunction>;

template <typename Key, typename Value>
using Map = std::unordered_map<Key, Value>;

int main(int argc, char **argv)
{
	init(argc, argv);
	State const INITIAL(*boost::vertices(G).first);
	Problem<Random, Distance, Neighbours, Visit, GoalTest> const PROBLEM(INITIAL);
	
	try
	{
		auto const T0(chrono::high_resolution_clock::now());
		auto const SOLUTION(jsearch::recursive_best_first_search<CostFunction, FalseTiePolicy, PriorityQueue>(PROBLEM));
		// auto const SOLUTION(jsearch::best_first_search<PriorityQueue, Comparator, ClosedList, Map>(PROBLEM));
		auto const ELAPSED(chrono::high_resolution_clock::now() - T0);
		cout.imbue(locale(""));
		cout << "Done: " << std::chrono::duration_cast<std::chrono::microseconds>(ELAPSED).count() << " µs\n";
		cout << backtrace(SOLUTION) << ": " << SOLUTION->path_cost() << "\n";

#ifdef STATISTICS
		cerr << "**** STATISTICS ****\n";
		cerr << "pushed: " << stats.pushed << "\n";
		cerr << "popped: " << stats.popped << "\n";
		cerr << "decreased: " << stats.decreased << "\n";
		cerr << "discarded: " << stats.discarded << "\n";
#endif
	}
	catch (goal_not_found const &ex)
	{
		cout << "No path from " << INITIAL << " to the goal could be found!\n";
	}
}


void init(int argc, char **argv)
{
	string const ARGV0(argv[0]);
	unsigned b, n, s(chrono::high_resolution_clock::to_time_t(chrono::high_resolution_clock::now()));

	// TODO: Use Program Options from Boost?
	switch(argc)
	{
		case 5:
			istringstream(argv[4]) >> s;
		case 4:
			istringstream(argv[3]) >> b;
			istringstream(argv[2]) >> n;
			istringstream(argv[1]) >> expanded;
			break;

		case 1:
		case 0:
			cerr << "Invocation: " << ARGV0.substr(ARGV0.find_last_of('/') + 1) << " <(e)panded nodes> <(n)odes> <(b)ranching factor> [seed]\nSuch that: e < n and b < n and n > 2\n";
			exit(EXIT_FAILURE);
			break;
			
		default:
			cerr << ARGV0 + ": Unexpected extra arguments.\n";
			exit(EXIT_FAILURE);
			break;
	}
	
	if(expanded > n)
	{
		cerr << "e(" << expanded << ") > n(" << n << ")\n";
		exit(EXIT_FAILURE);
	}

	G = Graph(n);
	gg::generate_graph(G, boost::num_vertices(G), b, mt19937(s));
	// weight = new WeightMap(get(boost::edge_weight, G));	
}


string backtrace(Node const &NODE)
{
	return (NODE->parent() ? backtrace(NODE->parent()) + " => " : "") + to_string(NODE->state());
}
