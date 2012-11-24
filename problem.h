/*
    problem.h: Search problem definition, taken from Russel & Norvig's AIMA.
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

#ifndef PROBLEM_H
#define PROBLEM_H

#include <memory>

namespace jsearch
{
	// TODO: Just a struct at the moment.  Should it enforce access public/private access rights?
	template <typename Traits>
	struct DefaultNode
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;
		typedef std::shared_ptr<DefaultNode<Traits>> ParentType;

		DefaultNode(State const &STATE, ParentType const PARENT, Action const &ACTION, PathCost const &PATH_COST) : state(STATE), parent(PARENT),  action(ACTION), path_cost(PATH_COST) {}

		State state;
		ParentType parent;
		Action action;
		PathCost path_cost;
	};


	template <typename Traits,
		template <typename PathCost, typename State, typename Action> class StepCostPolicy,
		template <typename State, typename Action> class ResultPolicy>
	class DefaultChildPolicy
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;
	protected:
		Node child(Node const &PARENT, Action const &ACTION)
		{
			return Node(PARENT, ACTION, PARENT.path_cost() + StepCostPolicy<PathCost, State, Action>::step_cost(PARENT.state(), ACTION), ResultPolicy<State, Action>::result(PARENT.state(), ACTION));
		}
	};


	template <typename Traits,
			 template <typename PathCost, typename State, typename Action> class StepCostPolicy,
			 template <typename State, typename Action> class ActionsPolicy,
			 template <typename State, typename Action> class ResultPolicy,
			 template <typename State> class GoalTestPolicy,
			 template <typename Traits_,
				template <typename PathCost, typename State, typename Action> class StepCostPolicy_,
				template <typename State, typename Action> class ResultPolicy_>
				class ChildPolicy = DefaultChildPolicy>
	class Problem :
		private StepCostPolicy<typename Traits::pathcost, typename Traits::state, typename Traits::action>,
		private ActionsPolicy<typename Traits::state, typename Traits::action>,
		private ResultPolicy<typename Traits::state, typename Traits::action>,
		private GoalTestPolicy<typename Traits::state>,
		private ChildPolicy<Traits, StepCostPolicy, ResultPolicy>
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;
	public:
		Problem(State const &INITIAL) : INITIAL(INITIAL) {}

		State initial() const
		{
			return INITIAL;
		}

		using ChildPolicy<Traits, StepCostPolicy, ResultPolicy>::child;
		using StepCostPolicy<PathCost, State, Action>::step_cost;
		using ActionsPolicy<State, Action>::actions;
		using ResultPolicy<State, Action>::result;
		using GoalTestPolicy<State>::goal_test;

	private:
		State const INITIAL;
	};
}

#endif
