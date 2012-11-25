/*
    TSP.cpp: Travelling Salesman Problem demonstration.
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

#include "TSP.h"
#include "problem.h"
#include "search.h"

#include <iostream>
#include <algorithm>
#include <sstream>
// TODO: ctime is not ideal, but chrono is incompatible with clang 3.1.
#include <ctime>
#include <locale>

#ifndef NDEBUG
#include <boost/graph/graphviz.hpp>
#include <fstream>
#endif


using namespace std;
using namespace jsearch;

Graph Australia();
Graph procedural(size_t const &n, mt19937::result_type const &SEED);


struct EdgeDescComp
{
public:
	bool operator()(edge_desc const &A, edge_desc const &B) const
	{
		return (*problem)[A] < (*problem)[B];
	}
};


template <typename Traits>
using AStarTSP = jsearch::DefaultAStar<Traits, MinimalImaginableTour>;

// Remember that weights are specified as 10 times larger.
template <typename Traits>
using WAStarTSP = jsearch::DefaultWAStar<Traits, MinimalImaginableTour, 100>;


int main(int argc, char **argv)
{
	float weight = 10.0;
	mt19937::result_type seed(time(nullptr));
	
	// TODO: Use Program Options from Boost?
	switch(argc)
	{
		case 4:
			// TODO: Make weight a run-time value.
			istringstream(argv[3]) >> weight;
			cout << "weight: " << weight << "\n";
			weight *= 10;
		case 3:
			istringstream(argv[2]) >> seed;
		case 2:
			istringstream(argv[1]) >> n;
			break;

		case 1:
		case 0:
			cerr << "Missing required value for n as argument 1.\n";
			exit(EXIT_FAILURE);
			break;

		default:
			cerr << "Unexpected extra arguments.\n";
			exit(EXIT_FAILURE);
			break;
	}

	problem.reset(new Graph(procedural(n, seed)));
	N = problem->m_num_edges;
	EDGES.reserve(N);
	pair<edge_iter, edge_iter> const EP(boost::edges(*problem));
	transform(EP.first, EP.second, back_inserter(EDGES), [&](edge_desc const &E){ return E; });
	sort(begin(EDGES), end(EDGES), EdgeDescComp());

#ifndef NDEBUG
	cout << "Sorted edge descriptors: " << jwm::to_string(EDGES) << endl;
#endif
	
	TSP::state const I;
	Problem<TSP, EdgeCost, HigherCostValidEdges, AppendEdge, ValidTour> const MINIMAL(I);
	cout.imbue(locale(""));
	
	try
	{
		// Change this to WAStarTSP to use weighted A* (and adjust the weight above if desired).
		TSP::node const SOLUTION = jsearch::best_first_search<AStarTSP>(MINIMAL);

		cout << "solution: { ";
		for_each(begin(SOLUTION.state), end(SOLUTION.state), [](vector<Index>::const_reference &E)
		{
			cout << EDGES[E] << " ";
		});
		cout << "}, " << SOLUTION.path_cost << endl;
	}
	catch (goal_not_found const &EX)
	{
		cout << "Goal not found! :(\n";
	}
	
	return 0;
}


Graph procedural(size_t const &n, mt19937::result_type const &SEED)
{
	vector<unsigned int> WEIGHT(n * (n - 1) / 2);
	uniform_int_distribution<TSP::pathcost> distribution(1, 500);
	cout << "seed: " << SEED << endl;
	mt19937 const engine(SEED);
	auto generator = bind(distribution, engine);
	generate(begin(WEIGHT), end(WEIGHT), generator);
	Graph g(n);

	for(vertex_desc i = 0, k = 0; i < n - 1; ++i)
	{
		for(vertex_desc j = i + 1; j < n; ++j, ++k)
		{
			auto const E = boost::add_edge(i, j, EdgeProps(WEIGHT[k]), g);
			if(!E.second)
				cerr << "Failed to add edge " << E.first << "to the graph." << endl;
		}
	}

#ifndef NDEBUG
	ofstream dot("TSP.dot");
	boost::write_graphviz(dot, g, boost::default_writer(), boost::make_label_writer(boost::get(&EdgeProps::cost, g)));
#endif

	pair<vertex_iter, vertex_iter> const VP = boost::vertices(g);
	cout << "vertices: ";
	for (auto vi = VP.first; vi != VP.second; ++vi)
		cout << *vi << " ";
	cout << "" << endl;

	// Verify that it worked.
	cout << "edges: ";
	edge_iter ei, ei_end;
	for (tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei)
		cout << *ei << ": "<< g[*ei].cost << "  ";
	cout << std::endl;
	
	return g;
}


Graph Australia()
{
	enum { Melbourne, Sydney, Perth, Adelaide, Darwin, Brisbane, Hobart, Canberra, n };
	const std::array<std::string, n> AUS_NAMES { { "Melbourne", "Sydney", "Perth", "Adelaide", "Darwin", "Brisbane", "Hobart", "Canberra" } };
	
	Graph g(n);

	// Add the vertices so I know what order they are in?
	boost::add_vertex(Melbourne, g);
	boost::add_vertex(Sydney, g);
	boost::add_vertex(Perth, g);
	boost::add_vertex(Adelaide, g);
	boost::add_vertex(Darwin, g);
	boost::add_vertex(Brisbane, g);
	boost::add_vertex(Hobart, g);
	boost::add_vertex(Canberra, g);
	
	typedef boost::graph_traits<Graph>::vertex_iterator vertex_iter;
	pair<vertex_iter, vertex_iter> const vp = boost::vertices(g);
	for (vertex_iter i = vp.first; i != vp.second; ++i)
	{
		g[*i].name = AUS_NAMES[i - vp.first];
	}

	boost::add_edge(Melbourne, Sydney, g);
	boost::add_edge(Melbourne, Perth, g);
	boost::add_edge(Melbourne, Adelaide, g);
	boost::add_edge(Melbourne, Darwin, g);
	boost::add_edge(Melbourne, Brisbane, g);
	boost::add_edge(Melbourne, Hobart, g);
	boost::add_edge(Melbourne, Canberra, g);
	boost::add_edge(Sydney, Perth, g);
	boost::add_edge(Sydney, Adelaide, g);
	boost::add_edge(Sydney, Darwin, g);
	boost::add_edge(Sydney, Brisbane, g);
	boost::add_edge(Sydney, Hobart, g);
	boost::add_edge(Sydney, Canberra, g);
	boost::add_edge(Perth, Adelaide, g);
	boost::add_edge(Perth, Darwin, g);
	boost::add_edge(Perth, Brisbane, g);
	boost::add_edge(Perth, Hobart, g);
	boost::add_edge(Perth, Canberra, g);
	boost::add_edge(Adelaide, Darwin, g);
	boost::add_edge(Adelaide, Brisbane, g);
	boost::add_edge(Adelaide, Hobart, g);
	boost::add_edge(Adelaide, Canberra, g);
	boost::add_edge(Darwin, Brisbane, g);
	boost::add_edge(Darwin, Hobart, g);
	boost::add_edge(Darwin, Canberra, g);
	boost::add_edge(Brisbane, Hobart, g);
	boost::add_edge(Brisbane, Canberra, g);
	boost::add_edge(Hobart, Canberra, g);

	typedef boost::graph_traits<Graph>::edge_iterator edge_iter;
	pair<edge_iter, edge_iter> const ep = boost::edges(g);
	for (edge_iter i = ep.first; i != ep.second; ++i)
	{
		// g[*i].cost = AUS_NAMES[i - ep.first];
	}
	
	return g;
}
