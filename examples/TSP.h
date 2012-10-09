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

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/edge_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/depth_first_search.hpp>

using boost::adjacency_matrix;


/*
inline Index city_Index(City const &C)
{
	if(C < 'A' || C > 'Z')
		throw std::runtime_error("City out of range.");
	return C - 'A';
}


inline Index edge_Index(Edge const &E)
{
	if(E < 'a' || E > 'z')
		throw std::runtime_error("Edge out of range.");
	return E - 'a';
}


inline City city(Index const &I)
{
	return I + 'A';
}


inline Edge edge(Index const &I)
{
	return I + 'a';
}
*/



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
	EdgeProps(std::string const &NAME, unsigned int const &W) : name(NAME), cost(W) {}

	std::string name;
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

// typedef std::pair<int, int> E;

typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iter;
typedef typename boost::graph_traits<Graph>::edge_iterator edge_iter;
typedef typename boost::graph_traits<Graph>::vertices_size_type vertices_size_type;
typedef typename boost::graph_traits<Graph>::edges_size_type edges_size_type;
typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_desc;
typedef typename boost::graph_traits<Graph>::edge_descriptor edge_desc;

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
	static bool const combinatorial = true;
};

Graph const *g = nullptr;
// Could these two be combined into a std::map<TSP::action, EdgeProps> without sacrificing complexity?
std::vector<EdgeProps> COST;
////////////////////////////////////////////////////////////////////////


// NOTE: Could this data somehow be stored on and accessed from Problem?
// std::vector<EdgeData<TSP::pathcost>> COST;
// std::vector<TSP::action> EDGES;
// size_t n; // I'm using a convention that n == number of cities, and N == number of edges.
// So n is the size of the TSP in cities, but N is the size of the TSP in edges.
// Curiously, I just realized that the real difficulty of a TSP is the difference between n and N: neither by themselves tells you enough.

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
	// I thought about returning a pair of iterators for a while until I realized that, derr, I could be
	// returning any arbitray subset of the available actions, not a contiguous one.
	std::vector<Action> actions(State const &STATE) const
	{
		std::vector<Action> result;
		Action const START = STATE.empty() ? 0 : STATE.back() + 1,
					END = N - n + STATE.size() + 1;
		
		if(STATE.size() > 1)
		{
			for(Action a = START; a < END; ++a)
			{
				// Check each theoretical action for validity.
				// Add valid actions to result.
				result.push_back(a);
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
