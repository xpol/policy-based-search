
#include "problem.h"

#include <map>
#include <string>

class Romania;

class Romania
{
public:
	typedef std::string state;
	typedef std::string action;
	typedef unsigned int pathcost;
	typedef jsearch::DefaultNode<Romania> node;
};

std::map<Romania::state, std::map<Romania::state, Romania::pathcost>> const COST {  };


template <typename PathCost, typename State, typename Action>
class Distance
{
protected:
	PathCost step_cost(State const &STATE, Action const &ACTION) const
	{
		return COST[STATE][ACTION];
	}
};

