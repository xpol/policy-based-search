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

#include <set>
#include <unordered_set>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include "loki/TypeManip.h"
#include <boost/heap/pairing_heap.hpp>

#ifndef NDEBUG
#include <iostream>
#endif


namespace jsearch
{
	class goal_not_found : public std::exception
	{
	public:
		goal_not_found() {}
	};


	namespace Private
	{
		template <template<typename T, class ...Options> class PriorityQueue, typename T, class ...Options>
		T pop(PriorityQueue<T, Options...> &pq)
		{
			T const E(pq.top());
			pq.pop();
			return E;
		}

		
		template <template<typename T, class ...Options> class PriorityQueue, typename T, class ...Options>
		void insert(PriorityQueue<T, Options...> &pq, T const &E)
		{
			pq.push(E);
		}


		template <template<typename T, class ...Options> class PriorityQueue, typename T, class ...Options>
		void erase()
		{
			
		}
	}
	

	template <class ClosedList>
	inline void debug_closed(ClosedList const &CLOSED, Loki::Int2Type<false>)
	{
		std::cout << "closed: " << CLOSED.size() << "\n";
	}
	
	
	template <class ClosedList>
	inline void debug_closed(ClosedList const &CLOSED, Loki::Int2Type<true>)
	{
	}
	
	
	template <template <typename Traits_> class Comparator = Greedy,
			typename Traits,
			template <typename PathCost, typename State, typename Action> class StepCostPolicy,
			template <typename State, typename Action> class ActionsPolicy,
			template <typename State, typename Action> class ResultPolicy,
			template <typename State> class GoalTestPolicy,
			template <typename Traits_,
				template <typename PathCost, typename State, typename Action> class StepCostPolicy,
				template <typename State, typename Action> class ResultPolicy>
					class ChildPolicy = DefaultChildPolicy>
	typename Traits::node best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, ChildPolicy> const &PROBLEM)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;

		typedef std::shared_ptr<Node> OpenListElement;
		// TODO: Try using Boost's pairing heap and Fibonacci heap for the Open list.
		// TODO: Use type traits to determine whether to use a set or unordered_set for Open/Closed list?
		// typedef std::set<OpenListElement, Comparator<Traits>> OpenList;
		typedef boost::heap::pairing_heap<OpenListElement, boost::heap::compare<Comparator<Traits>>> OpenList;
		typedef typename Loki::Select<Traits::combinatorial, void *, std::unordered_set<State>>::Result ClosedList;

		OpenList open;
		ClosedList closed; // TODO: Make the closed list optional for combinatorial search.
		open.push(std::make_shared<Node>(PROBLEM.initial(), nullptr, Action(), 0));

		while(!open.empty())
		{
			OpenListElement const S(Private::pop(open));

#ifndef NDEBUG
			std::cout << "pop => " << S->state << "\n";
#endif
			
			if(PROBLEM.goal_test(S->state))
			{
#ifndef NDEBUG
				std::cout << "open: " << open.size();
				debug_closed(closed, Loki::Int2Type<Traits::combinatorial>());
				std::cout << "\n";
#endif
				return *S; // OK, I don't like non-local returns, but what else?
			}
			else
			{
				handle_parent(closed, S, Loki::Int2Type<Traits::combinatorial>());
				std::vector<Action> const ACTIONS = PROBLEM.actions(S->state);
				// TODO: If combinatorial == true, do lazy child generation.
				// TODO: Change to std::for_each once gcc bug #53624 is fixed.
				for(Action const ACTION : ACTIONS)
				{
					OpenListElement const CHILD(std::make_shared<Node>(PROBLEM.result(S->state, ACTION), S, ACTION, S->path_cost + PROBLEM.step_cost(S->state, ACTION)));
					handle_child(open, closed, CHILD, Loki::Int2Type<Traits::combinatorial>());
				}
			}
		}
		
		throw goal_not_found();
	}


	// Combinatorial child handler.
	template <typename E, class OpenList, class ClosedList>
	inline void handle_child(OpenList &open, ClosedList &, E const &CHILD, Loki::Int2Type<true>)
	{
		Private::insert(open, CHILD);
	}

	
	// Non-combinatorial child handler.
	template <typename E, class OpenList, class ClosedList>
	inline void handle_child(OpenList &open, ClosedList &closed, E const &CHILD, Loki::Int2Type<false>)
	{
		typedef typename OpenList::const_iterator const_iterator;
		
		if(closed.find(CHILD->state) == std::end(closed)) // If it is NOT in closed...
		{
			/* 	TODO: Sadly linear: can it be improved?  I am personally not very invested in the
			 *	performance of this section of code.	*/
			auto const END(std::end(open));
			bool found(false);
			
			for(const_iterator IT = std::begin(open); IT != END; ++IT)
			{
				if(CHILD->state == (*IT)->state && CHILD->path_cost < (*IT)->path_cost)
				{
					found = true;
#ifndef NDEBUG
					std::cout << "Replace " << (*IT)->state << " with " << CHILD->state << "\n";
#endif
					// DecreaseKey operation.
					auto const H(OpenList::s_handle_from_iterator(IT));
					open.decrease(H, CHILD); // Invalidates iterator;
					break;
				}
			}

			if(!found)
			{
#ifndef NDEBUG
				std::cout << "open <= " << CHILD->state << "\n";
#endif
				Private::insert(open, CHILD);
			}
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
