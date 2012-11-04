/*
    search.h: Best-first search algorithm, taken from Russell & Norvig's AIMA.
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

#ifndef SEARCH_H
#define SEARCH_H

#include "evaluation.h"
#include "problem.h"

#include <queue>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include "loki/TypeManip.h"

#ifndef NDEBUG
#include <iostream>
#endif


namespace jsearch
{
	class goal_not_found : public std::exception
	{
	public:
		goal_not_found(size_t const &CLOSED) : CLOSED(CLOSED) {}
		
		size_t const CLOSED;
	};


	template <typename Traits,
			template <typename PathCost, typename State, typename Action> class StepCostPolicy,
			template <typename State, typename Action> class ActionsPolicy,
			template <typename State, typename Action> class ResultPolicy,
			template <typename State> class GoalTestPolicy,
			template <typename Traits,
				template <typename PathCost, typename State, typename Action> class StepCostPolicy,
				template <typename State, typename Action> class ResultPolicy >
					class ChildPolicy = DefaultChildPolicy,

			template <typename PathCost, typename State> class HeuristicPolicy = ZeroHeuristic,
			template <typename State, typename PathCost> class PathCostPolicy = DefaultPathCost,
			template <typename Traits,
				template <typename State, typename PathCost> class PathCostPolicy,
				template <typename PathCost, typename State> class HeuristicPolicy> class Comparator = AStar>
	typename Traits::node best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, ChildPolicy> const &PROBLEM, Evaluation<HeuristicPolicy, PathCostPolicy, Comparator> const &)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;

		typedef std::shared_ptr<Node> OpenListElement;
		// TODO: Try using Boost's pairing heap and Fibonacci heap for the Open list.
		// TODO: Use type traits to determine whether to use a set or unordered_set for Open/Closed list?
		typedef std::set<OpenListElement, Comparator<Traits, PathCostPolicy, HeuristicPolicy>> OpenList;
		typedef std::set<State> ClosedList;

		OpenList open;
		ClosedList closed; // TODO: Make the closed list optional for combinatorial search.
		open.insert(std::make_shared<Node>(PROBLEM.initial(), nullptr, Action(), 0));

		while(!open.empty())
		{
			typename OpenList::const_iterator IT = std::begin(open);
			// I learnt something whilst writing this: S must be a value not a reference in this case!
			OpenListElement const S = *IT;
			open.erase(IT);

			if(PROBLEM.goal_test(S->state))
			{
#ifndef NDEBUG
				std::cerr << "open: " << open.size();
				if(!Traits::combinatorial)
					std::cerr << ", closed: " << closed.size();
				std::cerr << "\n";
#endif
				return *S; // OK, I don't like non-local returns, but what else?
			}
			else
			{
				handle_parent(closed, S, Loki::Int2Type<Traits::combinatorial>());
				std::vector<Action> const ACTIONS = PROBLEM.actions(S->state);
				auto const BEGIN = std::begin(ACTIONS), END = std::end(ACTIONS);
				/* TODO: If combinatorial == true, do lazy child generation.
				 * This is an optimization whereby only the required children of a state are generated, 
				 * instead of all of them as per regular best-first search.
				 */
				std::for_each(BEGIN, END, [&](typename std::set<Action>::const_reference ACTION)
				{
					OpenListElement const CHILD(std::make_shared<Node>(PROBLEM.result(S->state, ACTION), S, ACTION, S->path_cost + PROBLEM.step_cost(S->state, ACTION)));

					handle_child(open, closed, CHILD, Loki::Int2Type<Traits::combinatorial>());
				});
			}

		}
		throw goal_not_found(closed.size());
	}


	// Combinatorial child handler.
	template <typename E, class OpenList, class ClosedList>
	inline void handle_child(OpenList &open, ClosedList &, E const &CHILD, Loki::Int2Type<true>)
	{
		open.insert(CHILD);
	}

	
	// Non-combinatorial child handler.
	template <typename E, class OpenList, class ClosedList>
	inline void handle_child(OpenList &open, ClosedList &closed, E const &CHILD, Loki::Int2Type<false>)
	{
		if(closed.find(CHILD->state) == std::end(closed)) // If it is NOT in closed...
		{
			/* 	TODO: Sadly linear: can it be improved?  I am personally not very invested in the
			 *	performance of this section of code.	*/
			for(typename OpenList::const_iterator IT = std::begin(open); IT != std::end(open); ++IT)
			{
				if(CHILD->state == (*IT)->state && CHILD->path_cost < (*IT)->path_cost)
				{
					open.erase(IT);
					break;
				}
			}
			
			open.insert(CHILD);
		}		
	}
	
	
	template <typename E, class ClosedList>
	inline void handle_parent(ClosedList &, E const &, Loki::Int2Type<true>)
	{
	}
	

	template <typename E, class ClosedList>
	inline void handle_parent(ClosedList &closed, E const &S, Loki::Int2Type<false>)
	{
		closed.insert(S->state);
	}
}

#endif // SEARCH_H
