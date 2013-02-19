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

/**
 * @file evaluation.h
 * @brief A* comparator and associated heuristic, tie-breaker and path-cost policies.
 */

#ifndef EVALUATION_H
#define EVALUATION_H

#include <functional>
#ifndef NDEBUG
#include <iostream>
#endif

namespace jsearch
{
	template <typename Traits>
	class ZeroHeuristic
	{
	protected:
		typedef typename Traits::state State;
		typedef typename Traits::pathcost PathCost;

		ZeroHeuristic() {}
		~ZeroHeuristic() {}

		PathCost h(State const &) const
		{
			return 0;
		}
	};


	template <typename Traits>
	class DefaultPathCost
	{
	protected:
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;

		DefaultPathCost() {}
		~DefaultPathCost() {}

		PathCost g(Node const &NODE) const
		{
			return NODE->path_cost();
		}
	};


	// Low-h tie policy, non-total.
	template <typename Traits, template <typename Traits_> class HeuristicPolicy>
	class LowH : protected virtual HeuristicPolicy<Traits>
	{
		using HeuristicPolicy<Traits>::h;
		
	protected:
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;
		typedef typename Traits::state State;

		LowH() {}
		~LowH() {}
		
		// This function would ideally be called "break" but obviously that is taken.
		bool split(Node const &A, Node const &B) const
		{
			auto const Ah(h(A->state())), Bh(h(B->state()));
			return Ah > Bh;
		}
	};


	// Low-h tie policy, total ordering.
	template <typename Traits, template <typename Traits_> class HeuristicPolicy>
	class LowHTotal : protected virtual HeuristicPolicy<Traits>
	{
		using HeuristicPolicy<Traits>::h;
		
	protected:
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;
		typedef typename Traits::state State;

		LowHTotal() {}
		~LowHTotal() {}
		
		// This function would ideally be called "break" but obviously that is taken.
		bool split(Node const &A, Node const &B) const
		{
			auto const Ah(h(A->state())), Bh(h(B->state()));
			auto const RESULT(Ah == Bh ? A->state() > B->state() : Ah > Bh);
			return RESULT;
		}
	};


	/******************
	 * Cost functions *
	 ******************/

	/**	AStar: f(n) = g(n) + h(n)	*/
	template <typename Traits,
			template <typename Traits_> class HeuristicPolicy = ZeroHeuristic,
			template <typename Traits_> class PathCostPolicy = DefaultPathCost>
	class AStar :
			protected virtual HeuristicPolicy<Traits>,
			protected virtual PathCostPolicy<Traits>
	{
		using PathCostPolicy<Traits>::g;
		using HeuristicPolicy<Traits>::h;

	public:
		typedef typename Traits::node Node;
		typedef typename Traits::cost Cost;

		AStar() {}
		~AStar() {}

		Cost f(Node const &N) const
		{
			return g(N) + h(N->state());
		}
	};


	/**	Greedy: f(n) = h(n)
	 */
	template <typename Traits, template <typename Traits_> class HeuristicPolicy = ZeroHeuristic>
	class Greedy : protected virtual HeuristicPolicy<Traits>
	{
		using HeuristicPolicy<Traits>::h;

	public:
		typedef typename Traits::node Node;
		typedef typename Traits::cost Cost;
		typedef typename Traits::heuristic_cost HeuristicCost;

		Greedy() {}
		~Greedy() {}

		Cost f(Node const &N) const
		{
			// TODO: Need a conversion function from HeuristicCost to Cost?
			return h(N->state());
		}
	};


	/**	Dijkstra: f(n) = g(n)
	 */
	template <typename Traits, template <typename Traits_> class PathCostPolicy = DefaultPathCost>
	class Dijkstra : protected virtual PathCostPolicy<Traits>
	{
		using PathCostPolicy<Traits>::g;

	public:
		typedef typename Traits::node Node;
		typedef typename Traits::cost Cost;
		typedef typename Traits::pathcost PathCost;

		Dijkstra() {}
		~Dijkstra() {}


		Cost f(Node const &N) const
		{
			// TODO: Need a conversion function from PathCost to Cost?
			return g(N);
		}
	};


	/**********************
	 * Comparator classes *
	 **********************/
	/** TiebreakingComparator: passed to the priority_queue.  NOT a policy class, actually a host!
	 *
	 * CostPolicy and TiePolicy are not required to have any relationship.  That is, although a typical
	 * TiePolicy might split ties on the same heuristic that the CostPolicy uses, it is just as possible
	 * that the TiePolicy splits on something totally unrelated.
	 */
	template <typename Traits,
				template <typename Traits_> class CostPolicy = AStar,
				template <typename Traits_> class TiePolicy = LowH>
	class TiebreakingComparator : protected virtual CostPolicy<Traits>,
									protected virtual TiePolicy<Traits>
	{
		using TiePolicy<Traits>::split;
		using CostPolicy<Traits>::f;

	public:
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;
		typedef typename Traits::state State;
		typedef typename Traits::cost Cost;

		TiebreakingComparator() {}

		bool operator()(Node const &A, Node const &B) const
		{
			auto const Af(f(A)), Bf(f(B));
			bool const RESULT(Af == Bf ? split(A, B) : Af > Bf);
			return RESULT;
		}
	};


	/** SimpleComparator does not break ties, it just compares f(), and so uses the Dijkstra CostPolicy by default.
	 */
	template <typename Traits, template <typename Traits_> class CostPolicy = Dijkstra>
	class SimpleComparator : protected virtual CostPolicy<Traits>
	{
		using CostPolicy<Traits>::f;

	public:
		typedef typename Traits::node Node;
		typedef typename Traits::pathcost PathCost;
		typedef typename Traits::state State;
		typedef typename Traits::cost Cost;

		SimpleComparator() {}

		bool operator()(Node const &A, Node const &B) const
		{
			auto const Af(f(A)), Bf(f(B));
			bool const RESULT(Af > Bf);
			return RESULT;
		}
	};
}

#endif
