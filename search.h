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
#include "loki/TypeManip.h"

#include <iostream>


namespace jsearch
{
	/**
	 * @brief goal_not_found is thrown... when... < drum roll > THE GOAL IS NOT FOUND!
	 *
	 */
	class goal_not_found : public std::exception
	{
	public:
		goal_not_found() {}
	};


	template <template <typename Key_, typename Compare_, typename Alloc_ = std::allocator<Key_>> class Set,
			typename Key,
			typename Compare,
			template <typename T> class Alloc>
	Key pop(Set<Key, Compare, Alloc<Key>> &s)
	{
		auto it(std::end(s));
		Key const E(*--it);
		s.erase(it);
		return E;
	}


	template <template <typename T, typename... Options> class PriorityQueue,
				typename T,
				typename... Options>
	T pop(PriorityQueue<T, Options...> &pq)
	{
		auto const E(pq.top());
		pq.pop();
		return E;
	}


	template <template <typename Key_, typename Compare_, typename Alloc_ = std::allocator<Key_>> class Set,
			typename Key,
			typename Compare,
			template <typename T> class Alloc>
	void push(Set<Key, Compare, Alloc<Key>> &s, Key const &E)
	{
		auto const RESULT(s.insert(E));
		if(!RESULT.second)
			throw std::logic_error("FUCK.");
	}


	template <template <class T, class ...Options> class PriorityQueue, class T, class ...Options>
	void push(PriorityQueue<T, Options...> &pq, T const &E)
	{
		pq.push(E);
	}


	template <template <typename Key_, typename Compare_, typename Alloc_ = std::allocator<Key_>> class Set,
			typename Key,
			typename Compare,
			template <typename T> class Alloc>
	inline void decrease_key(Set<Key, Compare, Alloc<Key>> &s, typename Set<Key, Compare, Alloc<Key>>::const_iterator IT, typename Set<Key, Compare, Alloc<Key>>::const_reference E)
	{
		s.erase(IT);
		s.insert(E);
	}


	template <template <typename T, typename... Options> class PriorityQueue,
		typename T,
		typename... Options>
	inline void decrease_key(PriorityQueue<T, Options...> &pq, typename PriorityQueue<T, Options...>::const_iterator IT, typename PriorityQueue<T, Options...>::const_reference E)
	{
		auto const H(PriorityQueue<T, Options...>::s_handle_from_iterator(IT));
		pq.decrease(H, E);
	}


	// Interesting: this function needs a different template to the false specialization.
	template <typename E, class ClosedList>
	inline void handle_parent(ClosedList &, E const &, Loki::Int2Type<true>)
	{
	}


	template <class ClosedList>
	inline void handle_parent(ClosedList &closed, typename ClosedList::const_reference S, Loki::Int2Type<false>)
	{
		closed.insert(S);
	}


	// Combinatorial child handler.
	template <class OpenList>
	inline void handle_child(OpenList &open, typename OpenList::const_reference CHILD)
	{
		push(open, CHILD);
	}

		
	/**
	 * Canonical child handler.
	 *
	 * @return: A const reference to an OpenList element that equals i) nullptr if CHILD was not added to open, ii) CHILD if CHILD was added to open, or iii) another element if CHILD replaced it on open.
	 * */
	template <class OpenList, class ClosedList>
	inline typename OpenList::value_type handle_child(OpenList &open, ClosedList &closed, typename OpenList::const_reference CHILD)
	{
		typename OpenList::value_type result(nullptr); // Initialize to nullptr since it might be a bald pointer.


		if(closed.find(CHILD->state()) == std::end(closed)) // If it is NOT in closed...
		{
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
					decrease_key(open, IT, CHILD);
					result = *IT;
#ifndef NDEBUG
					std::cout << jwm::to_string(CHILD->state()) << ": replace " << (*IT)->path_cost() << " with " << CHILD->path_cost() << ".\n";
#endif
				}
#ifndef NDEBUG
				else
					std::cout << jwm::to_string(CHILD->state()) << ": keep " << (*IT)->path_cost() << " and throw away " << CHILD->path_cost() << ".\n";
#endif
			}
			else
			{
				push(open, CHILD);
				result = CHILD;
#ifndef NDEBUG
				std::cout << "open <= " << jwm::to_string(CHILD->state()) << "\n";
#endif
			}
		}

		return result;
	}


	/**************************
	 * 	 	 CANONICAL		  *
	 **************************/
	template <template <typename T> class PriorityQueue,
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

		typedef PriorityQueue<Node> OpenList;
		typedef MembershipBag<State> ClosedList;

		OpenList open;
		ClosedList closed;
		push(open, PROBLEM.create(PROBLEM.initial(), Node(), Action(), 0));

		while(!open.empty())
		{
			Node const S(pop(open));
#ifndef NDEBUG
			std::cout << S->state() << " <= open\n";
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
				handle_parent(closed, S->state(), Loki::Int2Type<Traits::combinatorial>());
				std::vector<Action> const ACTIONS(PROBLEM.actions(S->state()));
				// TODO: Change to std::for_each once gcc bug #53624 is fixed.
				for(Action const ACTION : ACTIONS)
				{
					Node const CHILD(PROBLEM.child(S, ACTION));
					handle_child(open, closed, CHILD);
				}
			}
		}
		
		throw goal_not_found();
	}


	/**************************
	 *		COMBINATORIAL	  *
	 **************************/
	template <template <typename T> class PriorityQueue,
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

		typedef PriorityQueue<Node> OpenList;

		OpenList open;
		push(open, PROBLEM.create(PROBLEM.initial(), Node(), Action(), 0));

		while(!open.empty())
		{
			Node const S(pop(open));

			if(PROBLEM.goal_test(S->state()))
			{
				std::cout << "open: " << open.size() << "\n";
				return S;
			}
			else
			{
				std::vector<Action> const ACTIONS(PROBLEM.actions(S->state()));
				// TODO: Lazy child generation.
				// TODO: Change to std::for_each once gcc bug #53624 is fixed.
				for(Action const ACTION : ACTIONS)
				{
					Node const CHILD(PROBLEM.child(S, ACTION));
					handle_child(open, CHILD);
				}
			}
		}

		throw goal_not_found();
	}
}

#endif // SEARCH_H
