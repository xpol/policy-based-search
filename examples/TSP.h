/*
    TSP.h: Travelling Salesman Problem cast as an implicit graph search.
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

#ifndef TSP_H
#define TSP_H

#include "problem.h"
#include "evaluation.h"
#include "to_string.h"

#include <set>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <array>
#include <string>
#include <memory>
#include <stdexcept>
#include <sstream>

#ifndef NDEBUG
#include <iostream>
#endif

#include <boost/graph/graph_traits.hpp>
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

typedef typename boost::graph_traits<Graph>::out_edge_iterator out_edge_iterator;

vertices_size_type n; // Size of the TSP instance (number of cities).
edges_size_type N; // Size of the TSP instance (number of edges).

std::unique_ptr<Graph> problem;
std::vector<edge_desc> EDGES;

// Problem definition
struct TSP
{
	typedef typename std::vector<edge_desc>::const_iterator action;
	typedef std::vector<action> state; // Why vector again?  Remind me?  Why not set?  Do I really need back()?
	typedef unsigned int pathcost;
	typedef std::shared_ptr<jsearch::ComboNode<TSP>> node;
	static constexpr bool const combinatorial = true; // TODO: Hmmm... is this actually usable at compile time?
};


// TSP heuristic: shortest imaginable tour including these edges.
template <typename PathCost, class State>
class MinimalImaginableTour
{
protected:
	MinimalImaginableTour() {}
	~MinimalImaginableTour() {}
	
	PathCost h(State const &STATE) const
	{
		// Expects edge costs to be ordered.
		auto const EMPTY(STATE.empty());
		auto const LAST(STATE.back());
		auto const START(EMPTY ? std::begin(EDGES) : LAST + 1);
		auto const END(START + n - STATE.size());

		PathCost const RESULT(std::accumulate(START, END, 0, [&](PathCost const &A, edge_desc const &B)
		{
			return A + (*problem)[B].cost;
		}));
		
		return RESULT;
	}
};


template <typename PathCost, typename State, typename Action>
class EdgeCost
{
protected:
	EdgeCost() {}
	~EdgeCost() {}
	
	PathCost step_cost(State const &, Action const &ACTION) const
	{
		PathCost const RESULT((*problem)[*ACTION].cost);
		return RESULT;
	}
};


// This actually has to do the bulk of the work in the algorithm.
template <typename State, typename Action>
class HigherCostValidEdges
{
	// subgraph is used for testing whether a given combination of edges form a valid subsection of a tour.
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> subgraph;
protected:
	HigherCostValidEdges(){}
	~HigherCostValidEdges(){}
	
	// I thought about returning a pair of iterators for a while until I realized that, derr, I could be
	// returning any arbitray subset of the available actions, not a contiguous one.
	// Although it looks like this functions returns a State, it doesn't.
	std::vector<Action> actions(State const &STATE) const
	{
		std::vector<Action> result;
		auto const START(STATE.empty() ? EDGES.cbegin() : STATE.back() + 1),
					END(EDGES.cbegin() + N - n + STATE.size() + 1);
#ifndef NDEBUG
		std::cout << "Generating actions for state: {";
		std::for_each(std::begin(STATE), std::end(STATE), [&](typename State::const_reference ACTION){ std::cout << *ACTION; });
		std::cout << "}" << "\n";
#endif
		if(STATE.size() > 1)
		{
			// Create a graph containing all the edges in STATE.
			subgraph subproblem;
			
			std::for_each(std::begin(STATE), std::end(STATE), [&](typename State::const_reference &E)
			{
				auto const SOURCE(boost::source(*E, *problem)),
						   TARGET(boost::target(*E, *problem));
				// Use the vertices from the main problem in the subproblem.
				auto const RESULT(boost::add_edge(SOURCE, TARGET, subproblem));
				if(!RESULT.second)
				{
					std::ostringstream tmp;
					tmp << "  ERROR: Failed to add edge " << RESULT.first << " to subgraph.\n";
					throw std::logic_error(tmp.str());
				}
			});
			

			for(auto edge(START); edge != END; ++edge)
			{
				// Add candidate action to the subgraph.
				auto const SOURCE(boost::source(*edge, *problem)),
							TARGET(boost::target(*edge, *problem));
				auto const ADD_EDGE_RESULT(boost::add_edge(SOURCE, TARGET, subproblem));
				if(!ADD_EDGE_RESULT.second)
				{
					std::ostringstream tmp;
					tmp << "  ERROR: Failed to add edge " << ADD_EDGE_RESULT.first << " to subgraph.\n";
					throw std::logic_error(tmp.str());
				}
				// Check the graph for validity.
				bool valid(true);

				auto degree(boost::out_degree(SOURCE, subproblem));
				if(degree > 2)
				{
					valid = false;
#ifndef NDEBUG
					auto const EI(boost::out_edges(SOURCE, subproblem));
					std::set<boost::graph_traits<subgraph>::edge_descriptor> const invalid(EI.first, EI.second);
					std::cout << "  !invalid SOURCE edge: " << *edge << " on " << SOURCE << ". " << jwm::to_string(invalid) << "\n";
#endif
				}
				else
				{
					degree = boost::out_degree(TARGET, subproblem);

					if(degree > 2)
					{
						valid = false;
#ifndef NDEBUG
						auto const EI(boost::out_edges(TARGET, subproblem));
						std::set<boost::graph_traits<subgraph>::edge_descriptor> const invalid(EI.first, EI.second);
						std::cout << "  !invalid TARGET edge: " << *edge << " on " << TARGET << ". " << jwm::to_string(invalid) << "\n";
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
								std::cout << "  !cycle found: " << *edge << "\n";
#endif
							}
						}
					}
				}

				// If valid, add action to result.
				if(valid)
				{
#ifndef NDEBUG
					std::cout << "  GOOD edge: " << *edge << "\n";
#endif
					result.push_back(edge);
				}
				// Remove action from subgraph.
				boost::remove_edge(ADD_EDGE_RESULT.first, subproblem);
			}
		}
		else
		{
			// All actions are theoretically valid.
			for(auto edge(START); edge != END; ++edge)
				result.push_back(edge);
		}

#ifndef NDEBUG
		std::vector<edge_desc> tmp;
		tmp.reserve(result.size());
		std::for_each(std::begin(result), std::end(result), [&](Action const &A){ tmp.push_back(*A); });
		std::cout << "  Actions: " << jwm::to_string(tmp) << "\n";
#endif
		return result;
	}

private:
	class found_cycle : public std::exception
	{
	public:
		found_cycle(subgraph::edge_descriptor const &EDGE) : edge(EDGE) {};
		subgraph::edge_descriptor const edge;
	};

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
	private:
		// TODO: Find a way to make this a hash/unordered set.
		std::set<subgraph::edge_descriptor> predecessors;
	};
};


// Trivial function that adds action to the set of edges.
template <typename State, typename Action>
class AppendEdge	// ho ho ho
{
protected:
	AppendEdge() {}
	~AppendEdge() {}
	
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
	ValidTour() {}
	~ValidTour() {}
	
	bool goal_test(State const &STATE) const
	{
		return STATE.size() == n;
	}
};

#endif // TSP_H
