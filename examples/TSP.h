/*
    TSP.h: Travelling Salesman Problem cast as an implicit graph search.
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

#ifndef TSP_H
#define TSP_H

#include "problem.h"

#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <array>
#include <string>
#include <memory>
#include <unordered_set>
#include <stdexcept>
#include <iostream>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/edge_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/adjacency_list.hpp>

using boost::adjacency_matrix;


//Bundled properties.
struct VertexProps
{
	VertexProps() {}
	VertexProps(std::string const &NAME) : name(NAME) {}
	
	std::string name;
};


struct EdgeProps
{
	EdgeProps() {}
	EdgeProps(unsigned int const &W) : cost(W) {}

	unsigned int cost;

	bool operator<(EdgeProps const &OTHER) const
	{
		return cost < OTHER.cost;
	}


	bool operator==(EdgeProps const &OTHER) const
	{
		return cost == OTHER.cost;
	}
};

typedef adjacency_matrix<boost::undirectedS, VertexProps, EdgeProps> Graph;

typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iter;
typedef typename boost::graph_traits<Graph>::edge_iterator edge_iter;
typedef typename boost::graph_traits<Graph>::vertices_size_type vertices_size_type;
typedef typename boost::graph_traits<Graph>::edges_size_type edges_size_type;
typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_desc;
typedef typename boost::graph_traits<Graph>::edge_descriptor edge_desc;

// subgraph is used for testing whether a given combination of edges form a valid subsection of a tour.
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> subgraph;
vertices_size_type n; // Size of the TSP instance (number of cities).
edges_size_type N; // Size of the TSP instance (number of edges).


typedef unsigned short Index;
typedef char City;
// typedef char Edge;

class TSP;
// Problem definition
class TSP
{
public:
	typedef std::vector<Index> state; // Why vector again?  Remind me?  Why not set?  Do I really need back()?
	typedef Index action;
	typedef unsigned int pathcost;
	typedef jsearch::DefaultNode<TSP> node;
	static bool const combinatorial = true; // TODO: Hmmm... is this actually usable at compile time?
};

std::unique_ptr<Graph> problem;
// Could these two be combined into a std::map<TSP::action, EdgeProps> without sacrificing complexity?
std::vector<EdgeProps> COST;
std::vector<edge_desc> EDGES;
////////////////////////////////////////////////////////////////////////

/*
template <typename Traits,
			template <typename State, typename PathCost> class PathCostPolicy,
			template <typename PathCost, typename State> class HeuristicPolicy>
using WAStar = jsearch::WAStarNodeComparator<Traits, PathCostPolicy, HeuristicPolicy, 5>;
*/

// TSP heuristic: shortest imaginable tour including these edges.
template <typename PathCost, class State>
class MinimalImaginableTour
{
protected:
	PathCost h(State const &STATE) const
	{
		// Expects edge costs to be ordered.
		auto START = std::begin(COST);
		bool const EMPTY = STATE.empty();
		auto const BACK = STATE.back();
		auto const OFFSET = EMPTY ? 0 : BACK + 1;
		START += OFFSET;
		auto const END = START + n - STATE.size();

		PathCost const RESULT = std::accumulate(START, END, 0, [](PathCost const &A, EdgeProps const &B)
		{
			return A + B.cost;
		});
		
		return RESULT;
	}
};


template <typename PathCost, typename State, typename Action>
class EdgeCost
{
protected:
	PathCost step_cost(State const &, Action const &ACTION) const
	{
		PathCost const RESULT = COST[ACTION].cost;
		return RESULT;
	}
};


// This actually has to do the bulk of the work in the algorithm.
template <typename State, typename Action>
class HigherCostValidEdges
{
protected:
	class found_cycle : public std::exception
	{
	public:
		found_cycle(subgraph::edge_descriptor const &EDGE) : edge(EDGE) {};
		subgraph::edge_descriptor const edge;
	};
	
	// Cycle-detector.
	struct cycle_detector : public boost::dfs_visitor<>
	{
		template <class Edge, class Graph>
		void back_edge(Edge const &E, Graph&)
		{
			// Throw an exception so that the algorithm stops searching.
			if(predecessors.find(E) == std::end(predecessors))
				throw found_cycle(E);
		}

		template <class Edge, class Graph>
		void tree_edge(Edge const &E, Graph &)
		{
			predecessors.insert(E);
		}
		
	protected:
		// TODO: Find a way to make this a hash/unordered set.
		std::set<subgraph::edge_descriptor> predecessors;
	};
	
	
	// I thought about returning a pair of iterators for a while until I realized that, derr, I could be
	// returning any arbitray subset of the available actions, not a contiguous one.
	std::vector<Action> actions(State const &STATE) const
	{
		std::vector<Action> result;
		Action const START = STATE.empty() ? 0 : STATE.back() + 1,
					END = N - n + STATE.size() + 1;
#ifndef NDEBUG
		std::cerr << "Generating actions for " << to_string(STATE) << "\n";
#endif
		if(STATE.size() > 1)
		{
			// Create a graph containing all the edges in STATE.
			subgraph subproblem;
			
			std::for_each(std::begin(STATE), std::end(STATE), [&](typename State::const_reference &E)
			{
				auto const SOURCE = boost::source(EDGES[E], *problem),
						   TARGET = boost::target(EDGES[E], *problem);
				boost::add_edge(SOURCE, TARGET, subproblem); // Use the vertices from the main problem in the subproblem.
			});
			
			
			for(Action a = START; a < END; ++a)
			{
				// Add candidate action to the subgraph.
				auto const SOURCE = boost::source(EDGES[a], *problem),
							TARGET = boost::target(EDGES[a], *problem);
				auto const add_edge_result = boost::add_edge(SOURCE, TARGET, subproblem);
				if(!add_edge_result.second)
					std::cout << "ERROR: Failed to add edge " << add_edge_result.first << " to subgraph.\n";
				// Check the graph for validity.
				bool valid = true;

				auto degree = boost::out_degree(SOURCE, subproblem);
				if(degree > 2)
				{
					valid = false;
#ifndef NDEBUG
					std::cout << "!invalid edge: " << EDGES[a] << " on " << SOURCE << "\n";
#endif
				}
				else
				{
					degree = boost::out_degree(TARGET, subproblem);

					if(degree > 2)
					{
						valid = false;
#ifndef NDEBUG
						std::cout << "!invalid edge: " << EDGES[a] << " on " << TARGET << "\n";
#endif
					}
					else
					{
						if(STATE.size() != n - 1)
						{
							cycle_detector vis;

							try
							{
								boost::depth_first_search(subproblem, boost::visitor(vis));
							}
							catch (found_cycle &ex)
							{
								valid = false;
#ifndef NDEBUG
								std::cout << "!cycle found: " << ex.edge << "\n";
#endif
							}
						}
					}
				}

				// If valid, add action to result.
				if(valid)
				{
#ifndef NDEBUG
					std::cout << "GOOD edge: " << EDGES[a] << "\n";
#endif
					result.push_back(a);
				}
				// Remove action from subgraph.
				boost::remove_edge(add_edge_result.first, subproblem);
			}
		}
		else
		{
			// All actions are theoretically valid.
			for(Action a = START; a < END; ++a)
				result.push_back(a);
		}

		return result;
	}
};


// Trivial function that adds action to the set of edges.
template <typename State, typename Action>
class AppendEdge	// ho ho ho
{
protected:
	State result(State const &PARENT, Action const &ACTION) const
	{
		State child(PARENT);
		child.push_back(ACTION);
		return child;
	}
};


// GoalTestPolicy: Is it a Hamiltonian cycle?
// I think it is enough to have n edges that do not break any contraints.
template <typename State>
class ValidTour
{
protected:
	bool goal_test(State const &STATE) const
	{
		return STATE.size() == n;
	}
};

#endif // TSP_H

