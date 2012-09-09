
#ifndef SEARCH_H
#define SEARCH_H

#include "problem.h"

#include <queue>
#include <set>
#include <algorithm>
#include <iostream>

namespace jsearch
{


	template <typename Traits,
			template <typename State, typename Action> class StepCostPolicy,
			template <typename State, typename Action> class ActionsPolicy,
			template <typename State, typename Action> class ResultPolicy,
			template <typename State> class GoalTestPolicy,
			template <typename Node, typename State, typename Action,
				template <typename State, typename Action> class StepCostPolicy,
				template <typename State, typename Action> class ResultPolicy >
					class ChildPolicy = DefaultChildPolicy
					// , template <typename Node, typename Heuristic> class Comparator = AStarComparator
					>
	typename Traits::node *search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, ChildPolicy> const &PROBLEM, bool const combinatorial)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::action Action;
		typedef typename Traits::state State;
		typedef typename Traits::heuristic Heuristic;

		Node *solution = nullptr;

		// std::priority_queue<Node *, AStarComparator<Node, Heuristic>> open;
		std::priority_queue<Node *> open;
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

