/*
    random.h: Create a random graph for canonical search.
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

#include "problem.h"

#include <boost/graph/adjacency_matrix.hpp>

using std::size_t;
using boost::property;
using boost::undirectedS;
using boost::no_property;
using boost::edge_weight_t;

typedef boost::adjacency_matrix<undirectedS, no_property, property<edge_weight_t, size_t>> Graph;
typedef boost::property_map<Graph, edge_weight_t>::type WeightMap;
typedef typename boost::graph_traits<Graph>::edge_descriptor edge_desc;
typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_desc;

struct Random
{
	typedef vertex_desc state;
	typedef edge_desc action;
	typedef double pathcost;
	typedef std::shared_ptr<jsearch::DefaultNode<Random>> node;
	// typedef size_t heuristic; // Often but not necessarily the same as pathcost.
	static bool const combinatorial = false;
};


Graph *G = nullptr;
WeightMap const *weight = nullptr;
size_t 	b = 0, // Branching factor.
		e = 0; // Expanded nodes.


template <typename PathCost, typename State, typename Action>
class Distance
{
protected:
	PathCost step_cost(State const &, Action const &ACTION) const
	{
		return (*weight)[ACTION];
	}
};


// ActionsPolicy returns neighbouring cities to STATE.
template <typename State, typename Action>
class Neighbours
{
protected:
	std::vector<Action> actions(State const &STATE) const
	{
		auto const IT(boost::out_edges(STATE, *G));
		return std::vector<Action>(IT.first, IT.second);
	}
};


template <typename State, typename Action>
class Visit
{
protected:
	State result(State const &STATE, Action const &ACTION) const
	{
		auto const 	SOURCE(boost::source(ACTION, *G)),
					TARGET(boost::target(ACTION, *G));
		return SOURCE == STATE ? TARGET : SOURCE;
	}
};


template <typename State>
class GoalTest
{
protected:
	bool goal_test(State const &) const
	{
		static size_t e_(0);
		return e_++ == e;
	}
};
