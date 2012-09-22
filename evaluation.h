#ifndef EVALUATION_H
#define EVALUATION_H

#include <functional>

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
		PathCost g(Node const &NODE) const
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
		bool operator()(Node const &A, Node const &B) const
		{
			return g(A) + h(A.state) < g(B) + h(B.state);
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


	// Convenience class until I figure out a better way to do it.
	template <template <typename State, typename PathCost> class PathCostPolicy = DefaultPathCost,
		template <typename StepCost, typename State> class HeuristicPolicy = ZeroHeuristic,
		template <typename Traits,
			template <typename State, typename PathCost> class PathCostPolicy,
			template <typename StepCost, typename State> class HeuristicPolicy> class Comparator = AStarComparator>
	class Evaluation
	{
		// Absolutely nothing!
	};
}

#endif
