/*
    evaluation.h: Convenience class to make the search() function template easier.
    Also includes heuristic and pathcost policies, and queue comparators.
    Copyright (C) 2012  Jeremy Murphy <jeremy.william.murphy@gmail.com>

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

#ifndef EVALUATION_H
#define EVALUATION_H

#include <functional>

namespace jsearch
{
	template <typename PathCost, class State>
	class ZeroHeuristic
	{
	protected:
		PathCost h(State const &) const
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
			template <typename PathCost, typename State> class HeuristicPolicy>
	class AStarNodeComparator : public std::binary_function<typename Traits::node, typename Traits::node, bool>,
							private HeuristicPolicy<typename Traits::pathcost, typename Traits::state>,
							private PathCostPolicy<typename Traits::node, typename Traits::pathcost>
	{
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;
		typedef typename Traits::state State;
		
		using PathCostPolicy<Node, PathCost>::g;
		using HeuristicPolicy<PathCost, State>::h;
	public:
		bool operator()(std::shared_ptr<Node> const &A, std::shared_ptr<Node> const &B) const
		{
			return g(*A) + h(A->state) < g(*B) + h(B->state);
		}
	};


	// Weighted A* comparator functor.
	template <typename Traits,
		template <typename State, typename PathCost> class PathCostPolicy,
		template <typename PathCost, typename State> class HeuristicPolicy,
		size_t Weight = 10>
	class WAStarNodeComparator : public std::binary_function<typename Traits::node, typename Traits::node, bool>,
							private HeuristicPolicy<typename Traits::pathcost, typename Traits::state>,
							private PathCostPolicy<typename Traits::node, typename Traits::pathcost>
	{
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;
		typedef typename Traits::state State;

		using PathCostPolicy<Node, PathCost>::g;
		using HeuristicPolicy<PathCost, State>::h;
	public:
		bool operator()(std::shared_ptr<Node> const &A, std::shared_ptr<Node> const &B) const
		{
			return g(*A) + (Weight / 10.0) * h(A->state) < g(*B) + (Weight / 10.0) * h(B->state);
		}

		/*
		// Returns old weight.
		float weight(float new_weight)
		{
			float const old_weight(weight_);
			weight_ = new_weight;
			return old_weight;
		}

	private:
		float weight_;
		*/
	};

	
	// AStarOperator
	template < 	typename Traits,
				template <typename PathCost, typename State> class HeuristicPolicy,
				template <typename State, typename PathCost> class PathCostPolicy >
	class AStarNodeOperator : 	private HeuristicPolicy<typename Traits::pathcost, typename Traits::state>,
								private PathCostPolicy<typename Traits::state, typename Traits::pathcost>
	{
		using PathCostPolicy<typename Traits::state, typename Traits::pathcost>::g;
		using HeuristicPolicy<typename Traits::pathcost, typename Traits::state>::h;
	protected:
		bool operator<(typename Traits::node const &OTHER) const
		{
			return g(*this) + h(this->state) < g(OTHER) + h(OTHER.state);
		}
	};


	// Convenience class until I figure out a better way to do it.
	template <template <typename PathCost, typename State> class HeuristicPolicy = ZeroHeuristic,
		template <typename State, typename PathCost> class PathCostPolicy = DefaultPathCost,
		template <typename Traits,
			template <typename State, typename PathCost> class PathCostPolicy,
			template <typename PathCost, typename State> class HeuristicPolicy> class Comparator = AStarNodeComparator>
	class Evaluation
	{
		/*
	public:
		Evaluation(float const weight) : weight_(weight) { }

		float weight() const
		{
			return weight_;
		}
		
	private:
		float const weight_;
		*/
	};
}

#endif
