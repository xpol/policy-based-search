
#include "problem.h"

#include <unordered_map>
#include <string>
#include <algorithm>
#include <vector>

class Romania;

class Romania
{
public:
	typedef std::string state;
	typedef std::string action;
	typedef unsigned int pathcost;
	typedef jsearch::DefaultNode<Romania> node;
	static bool const combinatorial = false;
};

typedef std::unordered_map<Romania::state, Romania::pathcost> StateCost;

// Road costs from city to city as an adjacency list.
std::unordered_map<Romania::state, StateCost> const COST {
	// { "Sibiu", { {"Fagaras", 99}, {"Rimnicu Vilcea", 80}, {"Arad", 140}, {"Oradea", 151} } },
	// { "Arad", { {"Zerind", 75}, {"Sibiu", 140}, {"Timisoara", 118} } },
	// { "Zerind", { {"Oradea", 71}, {"Arad", 75} } },
	// { "Oradea", { { "Zerind", 71 }, { "Sibiu", 151 } } },
	{ "Timisoara", { { "Arad", 118 }, { "Lugoj", 111 } } },
	{ "Sibiu", { {"Fagaras", 99}, {"Rimnicu Vilcea", 80} } },
	{ "Fagaras", { {"Sibiu", 99}, {"Bucharest", 211} } },
	{ "Rimnicu Vilcea", { {"Sibiu", 80}, {"Pitesti", 97} } },
	{ "Pitesti", { {"Rimnicu Vilcea", 97}, {"Bucharest", 101} } },
	{ "Bucharest", { {"Pitesti", 101}, {"Fagaras", 211} } }
};

// Straight-line distance from city to Bucharest.
std::unordered_map<Romania::state, Romania::pathcost> const SLD {
	{"Sibiu", 253}, {"Bucharest", 0}, {"Rimnicu Vilcea", 193}, {"Pitesti", 100}, {"Fagaras", 176}
};


// Simple StepCostPolicy that returns the road cost from city(STATE) to city(ACTION).
template <typename PathCost, typename State, typename Action>
class Distance
{
protected:
	PathCost step_cost(State const &STATE, Action const &ACTION) const
	{
		return COST.at(STATE).find(ACTION)->second;
	}
};


// ActionsPolicy returns neighbouring cities to STATE.
template <typename State, typename Action>
class Neighbours
{
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

		/*
		std::for_each(std::begin(COST.find(STATE)->second), std::end(COST.find(STATE)->second), [&](typename StateCost::const_reference P)
		{
			result.push_back(P.first);
		});
		*/
		
		return result;
	}
};


template <typename State, typename Action>
class Visit
{
protected:
	State result(State const &, Action const &ACTION) const
	{
		return ACTION;
	}
};


template <typename State>
class GoalTest
{
protected:
	bool goal_test(State const &STATE) const
	{
		return STATE == "Bucharest";
	}
};


template <typename PathCost, class State>
class EuclideanDistance
{
protected:
	PathCost h(State const &STATE) const
	{
		// std::unordered_map<Romania::state, Romania::pathcost>::const_iterator I(SLD.find(STATE));
		// return I->second;
		auto const RESULT = SLD.at(STATE);
		return RESULT;
	}
};
