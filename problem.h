

#ifndef PROBLEM_H
#define PROBLEM_H


namespace jsearch
{
	template <typename Traits>
	struct DefaultNode
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;

		DefaultNode(State const &STATE, DefaultNode<Traits> const *PARENT, Action const &ACTION, PathCost const &PATH_COST) : state(STATE), parent(PARENT),  action(ACTION), path_cost(PATH_COST) {}

		State state;
		DefaultNode<Traits> const *parent;
		Action action;
		PathCost path_cost;

		// bool operator<();
	};


	template <typename Node, typename State, typename Action,
		template <typename State, typename Action> class StepCostPolicy,
		template <typename State, typename Action> class ResultPolicy>
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
