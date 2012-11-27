/*
    evaluation.h: Convenience class to make the search() function template easier.
    Also includes heuristic and pathcost policies, and queue comparators.
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

#ifndef EVALUATION_H
#define EVALUATION_H

#include <functional>
#ifndef NDEBUG
#include <iostream>
#endif

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


	// Low-h tie policy.
	template <typename Traits,
			template <typename PathCost, typename State> class HeuristicPolicy,
			template <typename State, typename PathCost> class PathCostPolicy>
	class LowH
	{
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;
		typedef typename Traits::state State;

		/* This inner class works around the problem that would arise if this policy class inherited
		 * from HeuristicPolicy: multiple inheritance in the Comparator class. */
		class Heuristic : private HeuristicPolicy<typename Traits::pathcost, typename Traits::state>
		{
		public:
			Heuristic() {}
			using HeuristicPolicy<PathCost, State>::h;
		};
		
	protected:
		// This function would ideally be called "break" but obviously that is taken.
		bool split(std::shared_ptr<Node> const &A, std::shared_ptr<Node> const &B) const
		{
			Heuristic const POLICY;
			return POLICY.h(A->state) > POLICY.h(B->state);
		}
	};

	
	// Comparator classes, passed to the priority_queue.  NOT a policy class, actually a host!
	template <typename Traits,
			template <typename PathCost, typename State> class HeuristicPolicy = ZeroHeuristic,
			template <typename State, typename PathCost> class PathCostPolicy = DefaultPathCost,
			template <typename Traits_,
				template <typename PathCost, typename State> class HeuristicPolicy,
				template <typename State, typename PathCost> class PathCostPolicy>
					class TiePolicy = LowH>
	class AStar : public std::binary_function<typename Traits::node, typename Traits::node, bool>,
					private HeuristicPolicy<typename Traits::pathcost, typename Traits::state>,
					private PathCostPolicy<typename Traits::node, typename Traits::pathcost>,
						private TiePolicy<Traits, HeuristicPolicy, PathCostPolicy>
	{
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;
		typedef typename Traits::state State;
		
		using PathCostPolicy<Node, PathCost>::g;
		using HeuristicPolicy<PathCost, State>::h;
		using TiePolicy<Traits, HeuristicPolicy, PathCostPolicy>::split;
		
	public:
		bool operator()(std::shared_ptr<Node> const &A, std::shared_ptr<Node> const &B) const
		{
			auto const Af(g(*A) + h(A->state)), Bf(g(*B) + h(B->state));
			bool result(Af == Bf ? split(A, B) : Af > Bf);
			return result;
		}
	};


	template <typename Traits,
			template <typename PathCost, typename State> class HeuristicPolicy,
			template <typename State, typename PathCost> class PathCostPolicy>
	using AStarLowH = AStar<Traits, HeuristicPolicy, PathCostPolicy, LowH>;


	// Err... can't think of a better name.
	template <typename Traits,
			template <typename PathCost, typename State> class HeuristicPolicy>
	using DefaultAStar = AStarLowH<Traits, HeuristicPolicy, DefaultPathCost>;


	// Greedy comparator by using h() == 0.
	template <typename Traits>
	using Greedy = DefaultAStar<Traits, ZeroHeuristic>;
	
	
	/*	Weighted A* comparator functor.  Until template parameters support float, we pass the weight as a ratio of two numbers:
		Weight / Divisor.  Therefore, the default weight is 1.0.

		Example weighted comparator with a weight of 10 (owing to the default divisor of 10):

		template <typename Traits,
			template <typename PathCost, typename State> class HeuristicPolicy,
			template <typename State, typename PathCost> class PathCostPolicy,
				template <typename Traits_, template <typename State, typename PathCost> class PathCostPolicy,
				template <typename PathCost, typename State> class HeuristicPolicy> class TiePolicy>
			using W10AStar = WeightedAStar<Traits, HeuristicPolicy, PathCostPolicy, TiePolicy, 100>;

		The template alias W10AStar then fits the required type for the Evaluation class.
	 */
	// TODO: I got a feeling that the weight needs to be a run-time parameter.
	template <typename Traits,
		template <typename PathCost, typename State> class HeuristicPolicy = ZeroHeuristic,
		template <typename State, typename PathCost> class PathCostPolicy = DefaultPathCost,
		template <typename Traits_,
			template <typename PathCost, typename State> class HeuristicPolicy,
			template <typename State, typename PathCost> class PathCostPolicy>
			class TiePolicy = LowH,
		size_t Weight = 10, size_t Divisor = 10> // Templates do not accept floats, so we pass a ratio.
	class WeightedAStar : public std::binary_function<typename Traits::node, typename Traits::node, bool>,
							private HeuristicPolicy<typename Traits::pathcost, typename Traits::state>,
							private PathCostPolicy<typename Traits::node, typename Traits::pathcost>,
							private TiePolicy<Traits, HeuristicPolicy, PathCostPolicy>
	{
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;
		typedef typename Traits::state State;

		using PathCostPolicy<Node, PathCost>::g;
		using HeuristicPolicy<PathCost, State>::h;
		using TiePolicy<Traits, HeuristicPolicy, PathCostPolicy>::split;
		
	public:
		WeightedAStar() : weight(static_cast<float>(Weight) / Divisor)
		{
#ifndef NDEBUG
			std::cout << __FUNCTION__ << "(): " << weight << "\n";
#endif
		}
		
		
		bool operator()(std::shared_ptr<Node> const &A, std::shared_ptr<Node> const &B) const
		{
			auto const Af(g(*A) + weight * h(A->state)), Bf(g(*B) + weight * h(B->state));
			bool result(Af == Bf ? split(A, B) : Af < Bf);
			return result;
		}
		
	private:
		float const weight;  // TODO: This still feels a bit "runny": how to make it totally compile-time constant?  So that it does not even require memory access?
	};

	
	template <typename Traits,
		template <typename PathCost, typename State> class HeuristicPolicy,
		template <typename State, typename PathCost> class PathCostPolicy,
		size_t Weight>
	using WAStarLowH = WeightedAStar<Traits, HeuristicPolicy, PathCostPolicy, LowH, Weight>;
	
	
	// Err... can't think of a better name.
	template <typename Traits,
		template <typename PathCost, typename State> class HeuristicPolicy,
		size_t Weight>
	using DefaultWAStar = WAStarLowH<Traits, HeuristicPolicy, DefaultPathCost, Weight>;
	
}

#endif
