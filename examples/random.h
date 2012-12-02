/*
    random.h: Generate random data simply to stress-test the algorithm.
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


#ifndef RANDOM_H
#define RANDOM_H

#include "problem.h"

#include <vector>
#include <random>
#include <functional>
#include <algorithm>
#include <limits>
#include <iostream>

struct Random;

struct Random
{
	typedef uint action;
	typedef size_t state;
	typedef size_t pathcost;
	typedef jsearch::DefaultNode<Random> node;
	typedef unsigned int heuristic;
	static bool const combinatorial = true;
};


namespace
{
	size_t generated = 0, max_nodes;
	size_t b(10); // Branching factor.
	auto generator(std::bind(std::uniform_int_distribution<size_t>(0, std::numeric_limits<size_t>::max()), std::mt19937()));
}

// Simple StepCostPolicy that returns the road cost from city(STATE) to city(ACTION).
template <typename PathCost, typename State, typename Action>
class Distance
{
protected:
	PathCost step_cost(State const &, Action const &ACTION) const
	{
		return ACTION;
	}
};


// ActionsPolicy returns neighbouring cities to STATE.
template <typename State, typename Action>
class Neighbours
{
protected:
	std::vector<Action> actions(State const &STATE) const
	{
		std::vector<Action> result(b);
		std::generate(std::begin(result), std::end(result), generator);
		return std::move(result);
	}
};


template <typename State, typename Action>
class Visit
{
protected:
	State result(State const &STATE, Action const &ACTION) const
	{
		if(++generated % 10000 == 0)
		{
			std::cout << generated << std::endl;
		}
		return ACTION;
	}
};


template <typename State>
class GoalTest
{
protected:
	bool goal_test(State const &STATE) const
	{
		return generated >= max_nodes;
	}
};


#endif
