

#ifndef PROBLEM_H
#define PROBLEM_H

#include <functional>

namespace jsearch
{
	// Comparator class, passed to the priority_queue.
	template <typename StepCost, typename Node, class Heuristic>
	class AStarComparator : public std::binary_function<Node, Node, bool>, private Heuristic
	{
		using Heuristic::h;

		bool operator()(Node const &A, Node const &B) const
		{
			return A.path_cost + h(A.state) < B.path_cost + h(B.state);
		}
	};




	template <typename State, typename Action, typename PathCost>
	struct DefaultNode
	{
		DefaultNode(State const &STATE, DefaultNode<State, Action, PathCost> const *PARENT, Action const &ACTION, PathCost const &PATH_COST) : state(STATE), parent(PARENT),  action(ACTION), path_cost(PATH_COST) {}

		State state;
		DefaultNode<State, Action, PathCost> const *parent;
		Action action;
		PathCost path_cost;
	};


	template <typename Node, typename State, typename Action, template <typename State, typename Action> class StepCostPolicy, template <typename State, typename Action> class ResultPolicy>
	class DefaultChildPolicy
	{
	protected:
		Node child(Node const &PARENT, Action const &ACTION)
		{
			return Node(PARENT, ACTION, PARENT.path_cost() + StepCostPolicy<State, Action>::step_cost(PARENT.state(), ACTION), ResultPolicy<State, Action>::result(PARENT.state(), ACTION));
		}
	};


	template < typename ProblemTraits,
			 template <typename State, typename Action> class StepCostPolicy,
			 template <typename State, typename Action> class ActionsPolicy,
			 template <typename State, typename Action> class ResultPolicy,
			 template <typename State> class GoalTestPolicy,
			 template < typename Node, typename State, typename Action,
			 template <typename State, typename Action> class StepCostPolicy,
			 template <typename State, typename Action> class ResultPolicy >
			 class ChildPolicy = DefaultChildPolicy >
	class Problem :
		private StepCostPolicy<typename ProblemTraits::state, typename ProblemTraits::action>,
		private ActionsPolicy<typename ProblemTraits::state, typename ProblemTraits::action>,
		private ResultPolicy<typename ProblemTraits::state, typename ProblemTraits::action>,
		private GoalTestPolicy<typename ProblemTraits::state>,
		private ChildPolicy<typename ProblemTraits::node, typename ProblemTraits::state, typename ProblemTraits::action, StepCostPolicy, ResultPolicy>
	{
	public:
		Problem(typename ProblemTraits::state const &INITIAL) : INITIAL(INITIAL) {}

		typename ProblemTraits::state initial() const
		{
			return INITIAL;
		}

		using ChildPolicy<typename ProblemTraits::node, typename ProblemTraits::state, typename ProblemTraits::action, StepCostPolicy, ResultPolicy>::child;
		using StepCostPolicy<typename ProblemTraits::state, typename ProblemTraits::action>::step_cost;
		using ActionsPolicy<typename ProblemTraits::state, typename ProblemTraits::action>::actions;
		using ResultPolicy<typename ProblemTraits::state, typename ProblemTraits::action>::result;
		using GoalTestPolicy<typename ProblemTraits::state>::goal_test;

	private:
		typename ProblemTraits::state const INITIAL;
	};
}

#endif
