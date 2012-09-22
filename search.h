
#ifndef SEARCH_H
#define SEARCH_H

#include "problem.h"
#include "evaluation.h"

#include <queue>
#include <set>
#include <algorithm>
#include <stdexcept>

#ifndef NDEBUG
#include <iostream>
#endif


namespace jsearch
{
	class GoalNotFound : public std::exception {};
	
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
	typename Traits::node search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, ChildPolicy> const &PROBLEM, Evaluation<PathCostPolicy, HeuristicPolicy, Comparator> const &, bool const TREE)
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;


		std::priority_queue<Node, std::vector<Node>, Comparator<Traits, PathCostPolicy, HeuristicPolicy>> open;
		std::set<State> closed; // TODO: Make the closed list optional for combinatorial search.
		open.push(Node(PROBLEM.initial(), nullptr, 0, 0));

		while(!open.empty())
		{
			Node const &S = open.top();
			open.pop();

#ifndef NDEBUG
			// std::cerr << "open: " << open.size() << ", closed: " << closed.size() << "\n";
#endif
			
			if(PROBLEM.goal_test(S.state))
			{
				return S; // OK, I don't like non-local returns, but what else?
			}
			else
			{
				closed.insert(S.state);
				std::set<Action> const actions = PROBLEM.actions(S.state);
				auto const beginning = std::begin(actions), ending = std::end(actions);
				std::for_each(beginning, ending, [&](typename std::set<Action>::const_reference ACTION)
				{
					auto const child = Node(PROBLEM.result(S.state, ACTION), &S, ACTION, S.path_cost + PROBLEM.step_cost(ACTION));

					if(!TREE)
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
		throw GoalNotFound();
	}
}

#endif // SEARCH_H
