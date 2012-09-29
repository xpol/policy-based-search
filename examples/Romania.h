
#include "problem.h"

#include <map>
#include <string>
#include <algorithm>
#include <set>

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

typedef std::map<Romania::state, Romania::pathcost> StateCost;

// Road costs from city to city as an adjacency list.
std::map<Romania::state, StateCost> const COST {
	// { "Sibiu", { {"Fagaras", 99}, {"Rimnicu Vilcea", 80}, {"Arad", 140}, {"Oradea", 151} } },
	{ "Sibiu", { {"Fagaras", 99}, {"Rimnicu Vilcea", 80} } },
	{ "Fagaras", { {"Sibiu", 99}, {"Bucharest", 211} } },
	// { "Arad", { {"Zerind", 75}, {"Sibiu", 140}, {"Timisoara", 118} } },
	// { "Zerind", { {"Oradea", 71}, {"Arad", 75} } },
	{ "Rimnicu Vilcea", { {"Sibiu", 80}, {"Pitesti", 97} } },
	{ "Pitesti", { {"Rimnicu Vilcea", 97}, {"Bucharest", 101} } },
	{ "Bucharest", { {"Pitesti", 101}, {"Fagaras", 211} } }
};

// Straight-line distance from city to Bucharest.
std::map<Romania::state, Romania::pathcost> const SLD {
	{"Sibiu", 253}, {"Bucharest", 0}, {"Rimnicu Vilcea", 193}, {"Pitesti", 100}, {"Fagaras", 176}
};


// Simple StepCostPolicy that returns the road cost from city(STATE) to city(ACTION).
template <typename PathCost, typename State, typename Action>
class Distance
{
protected:
	PathCost step_cost(State const &STATE, Action const &ACTION) const
	{
		return COST.find(STATE)->second.find(ACTION)->second;
	}
};


// ActionsPolicy returns neighbouring cities to STATE.
template <typename State, typename Action>
class Neighbours
{
protected:
	std::set<Action> actions(State const &STATE) const
	{
		std::set<Action> result;

		std::for_each(std::begin(COST.find(STATE)->second), std::end(COST.find(STATE)->second), [&](typename StateCost::const_reference P)
		{
			result.insert(P.first);
		});
		
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
		std::map<Romania::state, Romania::pathcost>::const_iterator I(SLD.find(STATE));
		return I->second;
	}
};
