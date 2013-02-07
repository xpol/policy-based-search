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

#include "problem.hpp"

#include <boost/graph/adjacency_matrix.hpp>

using std::size_t;
using boost::property;
using boost::undirectedS;
using boost::no_property;
using boost::edge_weight_t;

typedef unsigned cost_t;
typedef property<edge_weight_t, cost_t> edge_prop;
typedef boost::adjacency_matrix<undirectedS, no_property, edge_prop> Graph;
typedef boost::property_map<Graph, edge_weight_t>::type WeightMap;
typedef typename boost::graph_traits<Graph>::edge_descriptor edge_desc;
typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_desc;

struct Random
{
	typedef vertex_desc state;
	typedef edge_desc action;
	typedef cost_t cost;
	typedef cost_t pathcost;
	typedef std::shared_ptr<jsearch::DefaultNode<Random>> node;
	// typedef size_t heuristic; // Often but not necessarily the same as pathcost.
	static bool const combinatorial = false;
};


Graph G(0);
WeightMap const weight = boost::get(boost::edge_weight, G);
size_t 	b = 0, // Branching factor.
		e = 0; // Expanded nodes.


template <typename Traits>
class Distance
{
public:
	typedef typename Traits::state State;
	typedef typename Traits::action Action;
	typedef typename Traits::pathcost PathCost;
	
protected:
	PathCost step_cost(State const &, Action const &ACTION) const
	{
		return weight[ACTION];
	}
};


// ActionsPolicy returns neighbouring cities to STATE.
template <typename Traits>
class Neighbours
{
public:
	typedef typename Traits::state State;
	typedef typename Traits::action Action;
	
protected:
	std::vector<Action> actions(State const &STATE) const
	{
		auto const IT(boost::out_edges(STATE, G));
		return std::vector<Action>(IT.first, IT.second);
	}
};


template <typename Traits>
class Visit
{
public:
	typedef typename Traits::state State;
	typedef typename Traits::action Action;
	
protected:
	State result(State const &STATE, Action const &ACTION) const
	{
		auto const 	SOURCE(boost::source(ACTION, G)),
					TARGET(boost::target(ACTION, G));
		return SOURCE == STATE ? TARGET : SOURCE;
	}
};


template <typename Traits>
class GoalTest
{
public:
	typedef typename Traits::state State;
	
protected:
	bool goal_test(State const &) const
	{
		static size_t e_(0);
		return e_++ == e;
	}
};
