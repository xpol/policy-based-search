

#ifndef PROBLEM_H
#define PROBLEM_H

#include <functional>

namespace jsearch
{
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
