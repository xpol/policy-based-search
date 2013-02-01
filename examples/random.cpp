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
#include "search.hpp"

#include <random>
#include <sstream>
#include <unordered_set>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <string>
#include <iterator>
#include <locale>
#include <type_traits>

#include <boost/heap/d_ary_heap.hpp>

#include <unordered_map>

#ifndef NDEBUG
#include <boost/graph/graphviz.hpp>
#include <fstream>
#endif

using namespace std;
using namespace jsearch;

typedef Random::state State;
typedef Random::node Node;

Graph procedural(size_t const &N, size_t const &B, mt19937::result_type const &SEED);
string backtrace(Node const &NODE);
Graph square();

template <typename T, typename Comparator>
using PriorityQueue = boost::heap::d_ary_heap<T, boost::heap::mutable_<true>, boost::heap::arity<2>, boost::heap::compare<Comparator>>;

template <typename T>
using ClosedList = std::unordered_set<T>;


template <typename T>
using QueueSet = boost::heap::d_ary_heap<T, boost::heap::mutable_<true>, boost::heap::arity<2>>;

template <typename Key, typename Value>
using Map = std::unordered_map<Key, Value>;

int main(int argc, char **argv)
{
	size_t n(0);
	mt19937::result_type seed(chrono::high_resolution_clock::to_time_t(chrono::high_resolution_clock::now()));
	string const ARGV0(argv[0]);
	
	// TODO: Use Program Options from Boost?
	switch(argc)
	{
		case 5:
			istringstream(argv[4]) >> seed;
		case 4:
			istringstream(argv[3]) >> n;
			istringstream(argv[2]) >> e;
			istringstream(argv[1]) >> b;
			break;

		case 3:
		case 2:
		case 1:
		case 0:
			cerr << "Invocation: " << ARGV0.substr(ARGV0.find_last_of('/') + 1) << " b e n\nSuch that: n ≥ e\n";
			exit(EXIT_FAILURE);
			break;
			
		default:
			cerr << "Unexpected extra arguments.\n";
			exit(EXIT_FAILURE);
			break;
	}

	if(e > n)
	{
		cerr << "e(" << e << ") > n(" << n << ")\n";
		exit(EXIT_FAILURE);
	}
	
	cout << "branching factor: " << b << "\n";
	cout << "expanded nodes: " << e << "\n";
	cout << "graph size: " << n << "\n";
	
	G = new Graph(procedural(n, b, seed));
	// G = new Graph(square());
	weight = new WeightMap(get(boost::edge_weight, *G));
	State const INITIAL(*boost::vertices(*G).first);
	Problem<Random, Distance, Neighbours, Visit, GoalTest> const PROBLEM(INITIAL);
	
	try
	{
		auto const T0(chrono::high_resolution_clock::now());
		auto const SOLUTION(jsearch::best_first_search<PriorityQueue, Dijkstra, ClosedList, Map>(PROBLEM));
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


Graph procedural(size_t const &N, size_t const &B, mt19937::result_type const &SEED)
{
	Graph g(N);
	conditional<is_integral<Random::pathcost>::value, uniform_int_distribution<Random::pathcost>, uniform_real_distribution<Random::pathcost>>::type weight_dist(1, 500);
	cout << "seed: " << SEED << endl;
	mt19937 const engine(SEED);
	auto weight_generator(bind(weight_dist, engine));

	for(vertex_desc i(0); i < N - B; ++i)
	{
		uniform_int_distribution<vertex_desc> vertex_dist(i + 1, N - 1);
		auto vertex_generator(bind(vertex_dist, engine));
		unsigned char failures(0);
		while(boost::out_degree(i, g) < B && failures < 3)
		{
			auto const V(vertex_generator());
			if(V != i && boost::in_degree(V, g) < B)
			{	
				auto const W(weight_generator());
				auto const E(boost::add_edge(i, V, W, g));
				if(!E.second)
				{
					// ostringstream tmp;
					// cerr << "Failed to add edge " << E.first << " to vertex " << i << ".\n";
					if(++failures == B)
						cerr << "Bailing out on vertex " << i << "\n";
					// throw logic_error(tmp.str());
				}
			}
		}
		failures = 0;
		cout << '.';
		cout.flush();
	}

	cout << "Graph created.\n";

#ifndef NDEBUG
	ofstream dot("Random.dot");
	boost::write_graphviz(dot, g, boost::default_writer(), boost::make_label_writer(boost::get(boost::edge_weight, g)));
#endif

	return g;
}


string backtrace(Node const &NODE)
{
	return (NODE->parent() ? backtrace(NODE->parent()) + " => " : "") + to_string(NODE->state());
}


Graph square()
{
	Graph g(4);

	// source, target, weight, graph
	boost::add_edge(0, 1, 1, g);
	boost::add_edge(0, 2, 5, g);
	boost::add_edge(0, 3, 10, g);
	boost::add_edge(1, 2, 1, g);
	boost::add_edge(1, 3, 3, g);
	boost::add_edge(2, 3, 1, g);

#ifndef NDEBUG
	ofstream dot("Random.dot");
	boost::write_graphviz(dot, g, boost::default_writer(), boost::make_label_writer(boost::get(boost::edge_weight, g)));
#endif

	return g;
}
