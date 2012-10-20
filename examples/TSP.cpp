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
#include <sstream>

#ifndef NDEBUG
#include <boost/graph/graphviz.hpp>
#include <fstream>
#endif


using namespace std;
using namespace jsearch;

Graph Australia();
Graph procedural(size_t const &n);

/*
class TSPProblem : public jsearch::Problem<TSP, EdgeCost, HigherCostValidEdges, AppendEdge, ValidTour>
{
	typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iter;
	typedef typename boost::graph_traits<Graph>::edge_iterator edge_iter;
	typedef typename boost::graph_traits<Graph>::vertices_size_type vertices_size_type;
	typedef typename boost::graph_traits<Graph>::edges_size_type edges_size_type;

public:
	TSPProblem(const State& INITIAL, Graph const &G) : Problem(INITIAL), graph_(G), vertices_(boost::vertices(G)), edges_(boost::edge(G)), n_(boost::num_vertices(G)), N_(boost::num_edges(G)) {};
	Graph graph() const { return graph_; };
	std::pair<vertex_iter, vertex_iter> vertices() const;
	std::pair<edge_iter, edge_iter> edges() const;
	inline vertices_size_type n() const { return n_; }
	inline edges_size_type N() const { return N_; }
	
private:
	Graph const graph_;
	std::pair<vertex_iter, vertex_iter> vertices_;
	std::pair<edge_iter, edge_iter> edges_;
	vertices_size_type n_; // Size of the TSP instance (number of cities).
	edges_size_type N_; // Size of the TSP instance (number of edges).
};
*/

// Example comparator of Weighted A* with a weight of 10 (owing to the default divisor of 10).
template <typename Traits,
	template <typename State, typename PathCost> class PathCostPolicy,
	template <typename PathCost, typename State> class HeuristicPolicy>
	using W10AStar = WeightedAStar<Traits, PathCostPolicy, HeuristicPolicy, 100>;


int main(int argc, char **argv)
{
	float weight = 1.0;
	// TODO: Use Program Options from Boost to clean up this ugly mess.
	if(argc > 1)
	{
		istringstream arg(argv[1]);
		arg >> n;

		if (argc > 2)
		{
			istringstream arg(argv[2]);
			arg >> weight;
		}
	}
	else
	{
		cerr << "Missing required value for n as argument 1.\n";
		exit(EXIT_FAILURE);
	}
	
	problem.reset(new Graph(procedural(n)));
	N = problem->m_num_edges;
	pair<edge_iter, edge_iter> const EP(boost::edges(*problem));
	
	transform(EP.first, EP.second, back_inserter(COST), [&](edge_desc const &E){ return (*problem)[E]; });
	sort(begin(COST), end(COST));

	TSP::state const i;

	Problem<TSP, EdgeCost, HigherCostValidEdges, AppendEdge, ValidTour> const minimal(i);

	Evaluation<MinimalImaginableTour> const eval;
	
	try
	{
		TSP::node const solution = jsearch::best_first_search(minimal, eval);

		cout << "solution: { ";
		for_each(begin(solution.state), end(solution.state), [](vector<Index>::const_reference &E)
		{
			cout << EDGES[E] << " ";
		});
		cout << "}, " << solution.path_cost << endl;
	}
	catch (goal_not_found &ex)
	{
		cout << "Goal not found! :(\n";
	}
	
	return 0;
}


Graph procedural(size_t const &n)
{
	vector<unsigned int> WEIGHT(n * (n - 1) / 2);
	std::uniform_int_distribution<TSP::pathcost> distribution(1, 1000);
	std::mt19937 const engine;
	auto generator = bind(distribution, engine);
	generate(begin(WEIGHT), end(WEIGHT), generator);
	Graph g(n);

	EDGES.reserve(n * (n - 1) / 2);

	for(vertex_desc i = 0, k = 0; i < n - 1; ++i)
	{
		for(vertex_desc j = i + 1; j < n; ++j, ++k)
		{
			auto const E = boost::add_edge(i, j, EdgeProps(WEIGHT[k]), g);
			if(!E.second)
				cerr << "Failed to add edge " << E.first << "to the graph." << endl;
			else
				EDGES.push_back(E.first);
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
