#ifndef EVALUATION_H
#define EVALUATION_H

#include <functional>

namespace jsearch
{
	template <typename State, typename PathCost>
	class DefaultPathCost
	{
	protected:
		PathCost g(State const &STATE)
		{
			return STATE.path_cost;
		}
	};

	// Comparator classes, passed to the priority_queue.
	template <typename Traits,
	template <typename StepCost, typename State> class HeuristicPolicy,
	template <typename State, typename PathCost> class PathCostPolicy>
	class AStarComparator : public std::binary_function<typename Traits::node, typename Traits::node, bool>,
	private HeuristicPolicy<typename Traits::stepcost, typename Traits::state>,
		private PathCostPolicy<typename Traits::state, typename Traits::pathcost>
		{
			using PathCostPolicy<typename Traits::state, typename Traits::pathcost>::g;
			using HeuristicPolicy<typename Traits::stepcost, typename Traits::state>::h;
		protected:
			bool operator()(typename Traits::node const *A, typename Traits::node const *B) const
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
}
#endif
