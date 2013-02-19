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

#include "evaluation.hpp"
#include "problem.hpp"
#include "to_string.hpp"
#include "queue_set.hpp"

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
	 */
	class goal_not_found : public std::exception
	{
	public:
		goal_not_found() {}
	};


	/**
	 * @brief Encapsulate the top()+pop() calls into a single pop().
	 */
	template <typename PriorityQueue>
	inline typename PriorityQueue::value_type pop(PriorityQueue &pq)
	{
		auto const E(pq.top());
		pq.pop();
		return E;
	}


	/**
	 * @brief Handle the fate of a child being added to the frontier.
	 *
	 * @return: An OpenList element that equals
	 * 				i) nullptr if CHILD was not added to the frontier
	 * 				ii) CHILD if CHILD was added to the frontier, or
	 * 				iii) another element if CHILD replaced it on the frontier.
	 * */
	template <class OpenList>
	inline typename OpenList::value_type handle_child(OpenList &frontier, typename OpenList::const_reference const &CHILD)
	{
		typename OpenList::value_type result(nullptr); // Initialize to nullptr since it might be a bald pointer.

		auto const IT(frontier.find(CHILD->state()));

		if(IT != std::end(frontier))
		{
			auto const &DUPLICATE((IT->second)); // The duplicate on the frontier.
			if(CHILD->path_cost() < (*DUPLICATE)->path_cost())
			{
#ifndef NDEBUG
				std::cout << jwm::to_string(CHILD->state()) << ": replace " << (*DUPLICATE)->path_cost() << " with " << CHILD->path_cost() << ".\n";
#endif
#ifdef STATISTICS
				++stats.decreased;
#endif
				result = (*DUPLICATE); // Store a copy of the node that we are about to replace.
				frontier.increase(DUPLICATE, CHILD); // The DECREASE-KEY operation is an increase because it is a max-heap.
			}
			else
			{
#ifndef NDEBUG
				std::cout << jwm::to_string(CHILD->state()) << ": keep " << (*DUPLICATE)->path_cost() << " and throw away " << CHILD->path_cost() << ".\n";
#endif
#ifdef STATISTICS
				++stats.discarded;
#endif
			}
		}
		else
		{
			frontier.push(CHILD);
			result = CHILD;
#ifndef NDEBUG
			std::cout << "frontier <= " << jwm::to_string(CHILD->state()) << "\n";
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
			template <typename T> class Set,
			template <typename Key, typename Value> class Map,
			typename Traits,
			template <typename Traits_> class StepCostPolicy,
			template <typename Traits_> class ActionsPolicy,
			template <typename Traits_> class ResultPolicy,
			template <typename Traits_> class GoalTestPolicy,
			template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
			template <typename Traits_,
				template <typename Traits__> class StepCostPolicy,
				template <typename Traits__> class ResultPolicy,
				template <typename Traits__> class CreatePolicy>
				class ChildPolicy = DefaultChildPolicy>
	typename Traits::node best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;

		typedef jsearch::queue_set<PriorityQueue<Node, Comparator<Traits>>, Map> Frontier;
		typedef Set<State> ClosedSet;

		Frontier frontier;
		ClosedSet closed;
		frontier.push(PROBLEM.create(PROBLEM.initial(), Node(), Action(), 0));

		while(!frontier.empty())
		{
			auto const S(pop(frontier));
#ifndef NDEBUG
			std::cout << S->state() << " <= frontier\n";
#endif
#ifdef STATISTICS
			++stats.popped;
#endif
			if(PROBLEM.goal_test(S->state()))
			{
#ifndef NDEBUG
				std::cout << "frontier: " << frontier.size() << "\n";
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
						handle_child(frontier, CHILD);
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
			template <typename Traits_> class StepCostPolicy,
			template <typename Traits_> class ActionsPolicy,
			template <typename Traits_> class ResultPolicy,
			template <typename Traits_> class GoalTestPolicy,
			template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
			template <typename Traits_,
				template <typename Traits__> class StepCostPolicy,
				template <typename Traits__> class ResultPolicy,
				template <typename Traits__> class CreatePolicy>
				class ChildPolicy = DefaultChildPolicy>
	typename Traits::node best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;

		typedef PriorityQueue<Node, Comparator<Traits>> Frontier;

		Frontier frontier;
		frontier.push(PROBLEM.create(PROBLEM.initial(), Node(), Action(), 0));

		while(!frontier.empty())
		{
			auto const S(pop(frontier));

			if(PROBLEM.goal_test(S->state()))
			{
#ifndef NDEBUG
				std::cout << "frontier: " << frontier.size() << "\n";
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
					frontier.push(CHILD);
				}
			}
		}

		throw goal_not_found();
	}


	namespace recursive
	{
		template <typename Node>
		class goal_found : public std::exception
		{
		public:
			goal_found(Node const &SOLUTION) : SOLUTION(SOLUTION) {}
			Node solution() const { return SOLUTION; }

		private:
			Node SOLUTION;
		};


		template <typename Traits, template <typename Traits_> class TiePolicy, template <typename T> class PriorityQueue>
		class NodeCost : protected TiePolicy<Traits>
		{
			typedef typename Traits::node Node;
			typedef typename Traits::cost Cost;
			using TiePolicy<Traits>::split;
			typedef typename PriorityQueue<NodeCost<Traits, TiePolicy, PriorityQueue>>::handle_type handle_type;

		public:
			NodeCost(Node const &NODE, Cost const &COST) : node_(NODE), cost_(COST) {}

			const Node &node() const { return node_; }
			const Cost &cost() const { return cost_; }

			/**
			 * First compare on the stored cost, if they are equal use the TiePolicy.
			 */
			bool operator<(NodeCost<Traits, TiePolicy, PriorityQueue> const &OTHER) const
			{
				// Greater-than for max-heap.
				auto const RESULT(cost_ == OTHER.cost_ ? split(node_, OTHER.node_) : (cost_ > OTHER.cost_ ? true : false));
				return RESULT;
			}

			void update_cost(Cost const &COST) {
				cost_ = COST; }

			handle_type handle;

		private:
			Node node_;
			Cost cost_;
		};


#ifndef NDEBUG
		template <typename Traits, template <typename Traits_> class TiePolicy, template <typename T> class PriorityQueue>
		std::ostream& operator<<(std::ostream& stream, NodeCost<Traits, TiePolicy, PriorityQueue> const &O)
		{
			stream << "{" << *O.node() << ", " << O.cost() << "}";
			return stream;
		}
#endif


		/*******************************
		* Recursive best-first search *
		*******************************/
		/**
		 * This is the recursive implementation of the search, not to be called by clients.
		 */
		template <template <typename Traits> class CostFunction,
			template <typename Traits> class TiePolicy,
			template <typename T> class PriorityQueue,
			typename Traits,
			template <typename Traits_> class StepCostPolicy,
			template <typename Traits_> class ActionsPolicy,
			template <typename Traits_> class ResultPolicy,
			template <typename Traits_> class GoalTestPolicy,
			template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
			template <typename Traits_,
				template <typename Traits__> class StepCostPolicy,
				template <typename Traits__> class ResultPolicy,
				template <typename Traits__> class CreatePolicy>
				class ChildPolicy = DefaultChildPolicy>
		typename Traits::pathcost recursive_best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM, CostFunction<Traits> const &COST, typename Traits::node const &NODE, typename Traits::pathcost const &F_N, typename Traits::pathcost const &B)
		{
			typedef typename Traits::node Node;
			typedef typename Traits::state State;
			typedef typename Traits::action Action;
			typedef typename Traits::pathcost PathCost;
			typedef typename Traits::cost Cost;
			typedef NodeCost<Traits, TiePolicy, PriorityQueue> RBFSNodeCost; // uhh...
			typedef PriorityQueue<RBFSNodeCost> ChildrenPQ;

			using std::placeholders::_1;
			using std::placeholders::_2;

			/*	A single-line comment (//) is a direct quotes from the algorithm, to show how it has been interpreted.
			*	Mainly so that if there is a bug, it will be easier to track down.  :)
			*
			*	It is assumed that the algorithm used 1-offset arrays.
			*/
#ifndef NDEBUG
			std::cerr << ">>> " << __FUNCTION__ << "(PROBLEM, COST, " << *NODE << ", " << F_N << ", " << B << ")\n";
#endif

			constexpr auto const INF(std::numeric_limits<PathCost>::max()); // TODO: Is this correct?
			auto const f_N(COST.f(NODE));

			// IF f(N)>B, return f(N)
			if(f_N > B)
				return f_N;

			// IF N is a goal, EXIT algorithm
			if(PROBLEM.goal_test(NODE->state()))
				throw recursive::goal_found<Node>(NODE);
			auto const ACTIONS(PROBLEM.actions(NODE->state()));

			// IF N has no children, RETURN infinity
			if(ACTIONS.empty())
				return INF;

			PriorityQueue<RBFSNodeCost> children;

			// FOR each child Ni of N,
			for(auto const ACTION : ACTIONS)
			{
				auto const CHILD(PROBLEM.child(NODE, ACTION));
				auto const f_CHILD(COST.f(CHILD));
				// IF f(N)<F(N) THEN F[i] := MAX(F(N),f(Ni))
				// ELSE F[i] := f(Ni)
				auto const f_RESULT(f_N < F_N ? std::max(F_N, f_CHILD) : f_CHILD);
				auto const HANDLE(children.push(RBFSNodeCost(CHILD, f_RESULT)));
				(*HANDLE).handle = HANDLE; // Looks weird, makes sense.
			}

			// sort Ni and F[i] in increasing order of F[i]
			/*	They sort automatically.	*/

			// IF only one child, F[2] := infinity
			/*	Handle this sentinel value later.	*/

			// WHILE (F[1] <= B and F[1] < infinity)
			/*	TODO: I almost fail to see the point of testing for less than infinity???	*/
			while(children.top().cost() <= B && children.top().cost() < INF)
			{
				auto it(children.ordered_begin());
				auto const &BEST(*it++);
				auto const SECOND_BEST_COST(it == children.ordered_end() ? INF : it->cost());
				// F[1] := RBFS(N1, F[1], MIN(B, F[2]))
				(*BEST.handle).update_cost(recursive_best_first_search<CostFunction, TiePolicy, PriorityQueue>(PROBLEM, COST, BEST.node(), BEST.cost(), std::min(B, SECOND_BEST_COST)));
				// insert N1 and F[1] in sorted order
				/*	N1 is updated in-place.	*/
				children.update(BEST.handle);
			}

			// return F[1]
			return children.top().cost();
		}
	}


	/**
	 * The interface to RBFS.
	 */
	template <template <typename Traits> class CostFunction,
		template <typename Traits> class TiePolicy,
		template <typename T> class PriorityQueue,
		typename Traits,
		template <typename Traits_> class StepCostPolicy,
		template <typename Traits_> class ActionsPolicy,
		template <typename Traits_> class ResultPolicy,
		template <typename Traits_> class GoalTestPolicy,
		template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
		template <typename Traits_,
			template <typename Traits__> class StepCostPolicy,
			template <typename Traits__> class ResultPolicy,
			template <typename Traits__> class CreatePolicy>
			class ChildPolicy = DefaultChildPolicy>
	typename Traits::node recursive_best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;


		constexpr auto const INF(std::numeric_limits<PathCost>::max());
		auto initial(PROBLEM.create(PROBLEM.initial(), Node(), Action(), 0));
		Node result; // Unnecessary but polite to the compiler.
		CostFunction<Traits> const COST; // TODO: Design flaw?


		try
		{
			recursive::recursive_best_first_search<CostFunction, TiePolicy, PriorityQueue>(PROBLEM, COST, initial, COST.f(initial), INF);
			assert(result); // Should not actually ever reach this code.
		}
		catch(recursive::goal_found<Node> const &EX)
		{
			result = EX.solution();
		}
		// Don't catch goal_not_found, let it propagate.
		return result;
	}
}

#endif // SEARCH_H
