/*
    Romania.h: Problem definition.
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

/* Romania is a country in eastern Europe.  It is also a didactic pathfinding
 * example in AIMA, set in the same country.  The problem is usually cast as
 * how to get from Arad, in the northwest, to the capital Bucharest via the
 * road system.
 *
 * Arad => Sibiu => Rimnicu Vilcea => Pitesti => Bucharest: 418
 * 
 */

#include "problem.hpp"

#include <unordered_map>
#include <string>
#include <algorithm>
#include <vector>

class Romania
{
public:
	typedef std::string state;
	typedef std::string action;
	typedef unsigned int pathcost;
	typedef std::shared_ptr<jsearch::DefaultNode<Romania>> node;
	typedef unsigned int heuristic; // Often but not necessarily the same as pathcost.
	static bool const combinatorial = false;
};

/* Obviously, one would never use the following data structure in a real program, but for this toy
 * problem I wanted to keep the conceptual overhead as low as possible and the data structures transparent.
 */
typedef std::unordered_map<Romania::state, Romania::pathcost> StateCost;

// Road costs from city to city as an adjacency list.
std::unordered_map<Romania::state, StateCost> const COST {
// 	{  ORIGIN,   { DESTINATIONS... }}
	{ "Arad", { {"Zerind", 75}, {"Sibiu", 140}, {"Timisoara", 118} } },
	{ "Bucharest", { {"Pitesti", 101}, {"Fagaras", 211} } },
	{ "Craiova", { {"Drobeta", 120}, {"Rimnicu Vilcea", 146}, {"Pitesti", 138} } },
	{ "Drobeta", { {"Mehadia", 75}, {"Craiova", 120} } },
	{ "Fagaras", { {"Sibiu", 99}, {"Bucharest", 211} } },	// Done.
	{ "Lugoj", { {"Timisoara", 111}, {"Mehadia", 70} } },
	{ "Mehadia", { {"Lugoj", 70}, {"Drobeta", 75} } },
	{ "Oradea", { { "Zerind", 71 }, { "Sibiu", 151 } } },
	{ "Pitesti", { {"Rimnicu Vilcea", 97}, {"Bucharest", 101}, {"Craiova", 138} } },
	{ "Rimnicu Vilcea", { {"Sibiu", 80}, {"Pitesti", 97}, {"Craiova", 146} } },
	{ "Sibiu", { {"Fagaras", 99}, {"Rimnicu Vilcea", 80}, {"Arad", 140}, {"Oradea", 151} } },
	{ "Timisoara", { { "Arad", 118 }, { "Lugoj", 111 } } },	// Done.
	{ "Zerind", { {"Oradea", 71}, {"Arad", 75} } },
	
	// South and eastern sections of the map omitted.
};

// Straight-line distance from city to Bucharest.
std::unordered_map<Romania::state, Romania::heuristic> const SLD {
	{"Arad", 366},
	{"Bucharest", 0},
	{"Craiova", 160},
	{"Drobeta", 242},
	{"Fagaras", 176},
	{"Lugoj", 244},
	{"Mehadia", 241},
	{"Oradea", 380},
	{"Pitesti", 100},
	{"Rimnicu Vilcea", 193},
	{"Sibiu", 253},
	{"Timisoara", 329},
	{"Zerind", 374},
};


// Simple StepCostPolicy that returns the road cost from city(STATE) to city(ACTION).
template <typename Traits>
class Distance
{
public:
	typedef typename Traits::state State;
	typedef typename Traits::action Action;
	typedef typename Traits::pathcost PathCost;
	
protected:
	PathCost step_cost(State const &STATE, Action const &ACTION) const
	{
		return COST.at(STATE).at(ACTION);
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
		std::vector<Action> result;
		auto const NBRS = COST.at(STATE);
		// Iterating over the inner cost map: slower for unordered_map?
		// Probably not, because a pathfinding problem like this is not a complete graph, so the number of neighbours does
		// not increase with n.
		std::transform(std::begin(NBRS), std::end(NBRS), std::back_inserter(result), [&](typename StateCost::const_reference P)
		{
			return P.first;
		});
		
		return result;
	}
};


template <typename Traits>
class Visit
{
public:
	typedef typename Traits::state State;
	typedef typename Traits::action Action;
	
protected:
	State result(State const &, Action const &ACTION) const
	{
		return ACTION;
	}
};


template <typename Traits>
class GoalTest
{
public:
	typedef typename Traits::state State;
	
protected:
	bool goal_test(State const &STATE) const
	{
		return STATE == "Bucharest";
	}
};


template <typename Traits>
class EuclideanDistance
{
public:
	typedef typename Traits::state State;
	typedef typename Traits::pathcost PathCost;
	
protected:
	PathCost h(State const &STATE) const
	{
		auto const RESULT = SLD.at(STATE);
		return RESULT;
	}
};
