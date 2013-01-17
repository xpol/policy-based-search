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

/**
 * @file search.h
 * @brief Domain-independent best-first search function (and hidden helper functions).
 */

#ifndef SEARCH_H
#define SEARCH_H

#include "evaluation.h"
#include "problem.h"
#include "to_string.h"

#include <algorithm>
#include <stdexcept>

#ifndef NDEBUG
#include <iostream>
#endif

namespace jsearch
{
#ifdef STATISTICS
	struct statistics
	{
		statistics() : popped(0), pushed(0), decreased(0), discarded(0) {}
		size_t popped;
		size_t pushed;
		size_t decreased;
		size_t discarded;
	};
	
	statistics stats;
#endif
	
	/**
	 * @brief goal_not_found is thrown... when... < drum roll > THE GOAL IS NOT FOUND!
	 *
	 */
	class goal_not_found : public std::exception
	{
	public:
		goal_not_found() {}
	};


	template <template <typename T, typename... Options> class PriorityQueue, typename T, typename... Options>
	inline T pop(PriorityQueue<T, Options...> &pq)
	{
		auto const E(pq.top());
		pq.pop();
		return E;
	}


	template <template <typename T, typename... Options> class PriorityQueue, typename T, typename... Options>
	inline void decrease_key(PriorityQueue<T, Options...> &pq, typename PriorityQueue<T, Options...>::iterator const &IT, typename PriorityQueue<T, Options...>::const_reference E)
	{
		auto const H(PriorityQueue<T, Options...>::s_handle_from_iterator(IT));
		pq.increase(H, E);
	}


	/**
	 * handle_child: handle the fate of a child being added to the open list.
	 *
	 * @return: An OpenList element that equals
	 * 				i) nullptr if CHILD was not added to open
	 * 				ii) CHILD if CHILD was added to open, or
	 * 				iii) another element if CHILD replaced it on open.
	 * */
	template <class OpenList>
	inline typename OpenList::value_type handle_child(OpenList &open, typename OpenList::const_reference CHILD)
	{
		typename OpenList::value_type result(nullptr); // Initialize to nullptr since it might be a bald pointer.


		/* 	TODO: Sadly linear: can it be improved?  I am personally not very invested in the
			*	performance of this section of code.	*/
		auto const 	END(std::end(open)),
					IT(std::find_if(std::begin(open), END, [&](typename OpenList::const_reference E)
					{
						return E->state() == CHILD->state();
					}));

		if(IT != END)
		{
			if(CHILD->path_cost() < (*IT)->path_cost())
			{
#ifndef NDEBUG
				std::cout << jwm::to_string(CHILD->state()) << ": replace " << (*IT)->path_cost() << " with " << CHILD->path_cost() << ".\n";
#endif
#ifdef STATISTICS
				++stats.decreased;
#endif
				result = *IT;
				decrease_key(open, IT, CHILD);
			}
#ifndef NDEBUG
			else
				std::cout << jwm::to_string(CHILD->state()) << ": keep " << (*IT)->path_cost() << " and throw away " << CHILD->path_cost() << ".\n";
#endif
#ifdef STATISTICS
				++stats.discarded;
#endif
		}
		else
		{
			open.push(CHILD);
			result = CHILD;
#ifndef NDEBUG
			std::cout << "open <= " << jwm::to_string(CHILD->state()) << "\n";
#endif
#ifdef STATISTICS
			++stats.pushed;
#endif
		}

		return result;
	}


	/**************************
	 * 	 	 Graph search	  *
	 **************************/
	template <template <typename T, typename Comparator> class PriorityQueue,
			template <typename Traits> class Comparator,
			template <typename T> class MembershipBag,
			typename Traits,
			template <typename PathCost, typename State, typename Action> class StepCostPolicy,
			template <typename State, typename Action> class ActionsPolicy,
			template <typename State, typename Action> class ResultPolicy,
			template <typename State> class GoalTestPolicy,
			template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
			template <typename Traits_,
				template <typename PathCost, typename State, typename Action> class StepCostPolicy,
				template <typename State, typename Action> class ResultPolicy,
				template <typename Traits__> class CreatePolicy>
				class ChildPolicy = DefaultChildPolicy>
	typename Traits::node best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;

		typedef PriorityQueue<Node, Comparator<Traits>> OpenList;
		typedef MembershipBag<State> ClosedList;

		OpenList open;
		ClosedList closed;
		open.push(PROBLEM.create(PROBLEM.initial(), Node(), Action(), 0));

		while(!open.empty())
		{
			auto const S(pop(open));
#ifndef NDEBUG
			std::cout << S->state() << " <= open\n";
#endif
#ifdef STATISTICS
			++stats.popped;
#endif
			if(PROBLEM.goal_test(S->state()))
			{
#ifndef NDEBUG
				std::cout << "open: " << open.size() << "\n";
				std::cout << "closed: " << closed.size() << "\n";
#endif
				return S;
			}
			else
			{
				closed.insert(S->state());
				auto const ACTIONS(PROBLEM.actions(S->state()));
				// TODO: Change to std::for_each once gcc bug #53624 is fixed.
				for(auto const ACTION : ACTIONS)
				{
					auto const SUCCESSOR(PROBLEM.result(S->state(), ACTION));
					if(closed.find(SUCCESSOR) == std::end(closed)) // If it is NOT in closed...
					{
						auto const CHILD(PROBLEM.child(S, ACTION, SUCCESSOR));
						handle_child(open, CHILD);
					}
				}
			}
		}

		throw goal_not_found();
	}


	/**************************
	 *		Tree search		  *
	 **************************/
	template <template <typename T, typename Comparator> class PriorityQueue,
			template <typename Traits> class Comparator,
			typename Traits,
			template <typename PathCost, typename State, typename Action> class StepCostPolicy,
			template <typename State, typename Action> class ActionsPolicy,
			template <typename State, typename Action> class ResultPolicy,
			template <typename State> class GoalTestPolicy,
			template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
			template <typename Traits_,
				template <typename PathCost, typename State, typename Action> class StepCostPolicy,
				template <typename State, typename Action> class ResultPolicy,
				template <typename Traits__> class CreatePolicy>
				class ChildPolicy = DefaultChildPolicy>
	typename Traits::node best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;

		typedef PriorityQueue<Node, Comparator<Traits>> OpenList;

		OpenList open;
		open.push(PROBLEM.create(PROBLEM.initial(), Node(), Action(), 0));

		while(!open.empty())
		{
			auto const S(pop(open));

			if(PROBLEM.goal_test(S->state()))
			{
#ifndef NDEBUG
				std::cout << "open: " << open.size() << "\n";
#endif
				return S;
			}
			else
			{
				std::vector<Action> const ACTIONS(PROBLEM.actions(S->state()));
				// TODO: Lazy child generation.
				// TODO: Change to std::for_each once gcc bug #53624 is fixed.
				for(auto const ACTION : ACTIONS)
				{
					auto const CHILD(PROBLEM.child(S, ACTION));
					open.push(CHILD);
				}
			}
		}

		throw goal_not_found();
	}
}

#endif // SEARCH_H
