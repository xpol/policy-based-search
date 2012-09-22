
#ifndef SEARCH_H
#define SEARCH_H

#include "problem.h"

#include <queue>
#include <set>
#include <algorithm>
#include <iostream>

#define QCOMPARATOR

namespace jsearch
{
	template <typename StepCost, class State>
	class ZeroHeuristic
	{
	protected:
		StepCost h(State const &) const
		{
			return 0;
		}
	};


		template <typename Node, typename PathCost>
	class DefaultPathCost
	{
	protected:
		PathCost g(Node const &NODE)
		{
			return NODE.path_cost;
		}
	};

	// Comparator classes, passed to the priority_queue.  NOT a policy class, actually a host!
	template <typename Traits,
			template <typename State, typename PathCost> class PathCostPolicy,
			template <typename StepCost, typename State> class HeuristicPolicy>
	class AStarComparator : public std::binary_function<typename Traits::node, typename Traits::node, bool>,
							private HeuristicPolicy<typename Traits::stepcost, typename Traits::state>,
							private PathCostPolicy<typename Traits::node, typename Traits::pathcost>
	{
		typedef typename Traits::node Node;
		using PathCostPolicy<typename Traits::node, typename Traits::pathcost>::g;
		using HeuristicPolicy<typename Traits::stepcost, typename Traits::state>::h;
	public:
		bool operator()(Node const *A, Node const *B) const
		{
			return g(A) + h(A->state) < g(B) + h(B->state);
		}
	};


	// AStarOperator
	template < 	typename Traits,
				template <typename StepCost, typename State> class HeuristicPolicy,
				template <typename State, typename PathCost> class PathCostPolicy >
	class AStarNodeOperator : 	private HeuristicPolicy<typename Traits::stepcost, typename Traits::state>,
								private PathCostPolicy<typename Traits::state, typename Traits::pathcost>
	{
		using PathCostPolicy<typename Traits::state, typename Traits::pathcost>::g;
		using HeuristicPolicy<typename Traits::stepcost, typename Traits::state>::h;
	protected:
		bool operator<(typename Traits::node const &OTHER) const
		{
			return g(*this) + h(this->state) < g(OTHER) + h(OTHER.state);
		}
	};

	
	template <typename Traits,
			template <typename State, typename Action> class StepCostPolicy,
			template <typename State, typename Action> class ActionsPolicy,
			template <typename State, typename Action> class ResultPolicy,
			template <typename State> class GoalTestPolicy,
			template <typename Node, typename State, typename Action,
				template <typename State, typename Action> class StepCostPolicy,
				template <typename State, typename Action> class ResultPolicy >
					class ChildPolicy = DefaultChildPolicy,

			template <typename State, typename PathCost> class PathCostPolicy = DefaultPathCost,
			template <typename StepCost, typename State> class HeuristicPolicy = ZeroHeuristic,
			template <typename Traits,
				template <typename State, typename PathCost> class PathCostPolicy,
				template <typename StepCost, typename State> class HeuristicPolicy> class Comparator = AStarComparator>
	typename Traits::node *search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, ChildPolicy> const &PROBLEM, bool const combinatorial)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;
		// typedef typename Traits::heuristicpolicy Heuristic;

		Node *solution = nullptr;

		std::priority_queue<Node *, std::vector<Node *>, Comparator<Traits, PathCostPolicy, HeuristicPolicy>> open;
		// std::priority_queue<Node *> open;
		std::set<State> closed;
		open.push(new Node(PROBLEM.initial(), (nullptr), 0, 0));

		while(!solution && !open.empty())
		{
			Node * const S = open.top();
			open.pop();

			std::cerr << "open: " << open.size() << ", closed: " << closed.size() << "\n";

			if(PROBLEM.goal_test(S->state))
			{
				solution = S;
			}
			else
			{
				closed.insert(S->state);
				std::set<Action> const actions(PROBLEM.actions(S->state));
				typename std::set<Action>::const_iterator beginning = actions.begin();
				typename std::set<Action>::const_iterator ending = actions.end();
				std::for_each(beginning, ending, [&](typename std::set<Action>::const_reference ACTION)
				{
					Node *child = new Node(PROBLEM.result(S->state, ACTION), S, ACTION, S->path_cost + PROBLEM.step_cost(ACTION));

					if(!combinatorial)
					{
						// TODO: Check if it is in open or closed.

					}
					else
					{
						/*	The combinatorial search spaces in mind are a tree with no repeating nodes,
							so the algorithm is optimized to not worry about checking in open or closed.	*/
						open.push(child);
					}

				});
			}

		}

		return solution;
	}
}

#endif // SEARCH_H
