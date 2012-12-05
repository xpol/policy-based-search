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

#ifndef NDEBUG
#include "to_string.h"
#include <iostream>
#include <typeinfo>
#endif

#include <memory>


namespace jsearch
{
	template <typename Traits>
	class DefaultNodeCreator
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;
		
	protected:
		DefaultNodeCreator() {}
		~DefaultNodeCreator() {}

		Node create(State const &STATE, Node const &NODE, Action const &ACTION, PathCost const &PATHCOST) const
		{
			return std::make_shared<typename Node::element_type>(STATE, NODE, ACTION, PATHCOST);
		}
	};


	template <typename Traits>
	class ComboNodeCreator
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;
		
	protected:
		ComboNodeCreator() {}
		~ComboNodeCreator() {}

		Node create(State const &STATE, Node const &, Action const &ACTION, PathCost const &PATHCOST) const
		{
			return std::make_shared<typename Node::element_type>(STATE, ACTION, PATHCOST);
		}
	};


	// Default- and ComboNode are the rare case of a concrete base class.
	template <typename Traits>
	class DefaultNode
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;

	public:
		DefaultNode(State const &STATE, Node const &PARENT, Action const &ACTION, PathCost const &PATH_COST) : state_(STATE), parent_(PARENT),  action_(ACTION), path_cost_(PATH_COST) {}

		DefaultNode(DefaultNode<Traits> const &OTHER) = delete;

		DefaultNode<Traits> &operator=(DefaultNode<Traits> const &OTHER) = delete;

		State const &state() const { return state_; }
		Node const &parent() const { return parent_; }
		Action const &action() const { return action_; }
		PathCost const &path_cost() const { return path_cost_; }

	private:
		State state_;
		Node parent_;
		Action action_;
		PathCost path_cost_;
	};


#ifndef NDEBUG
	template <typename Traits>
	std::ostream& operator<<(std::ostream& stream, DefaultNode<Traits> const &NODE)
	{
		stream << "{" << jwm::to_string(NODE.state()) << ", " << NODE.parent() << ", " << NODE.action() << ", " << NODE.path_cost() << "}";
		return stream;
	}
#endif


	template <typename Traits>
	class ComboNode
	{
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;
		
	public:
		ComboNode(State const &STATE, Action const &ACTION, PathCost const &PATH_COST) : state_(STATE), action_(ACTION), path_cost_(PATH_COST) {}
		
		ComboNode(ComboNode<Traits> const &OTHER) = delete;
		
		ComboNode<Traits> &operator=(ComboNode<Traits> const &OTHER) = delete;
		
		State const &state() const { return state_; }
		Action const &action() const { return action_; }
		PathCost const &path_cost() const { return path_cost_; }
		
	private:
		State state_;
		Action action_;
		PathCost path_cost_;
	};


#ifndef NDEBUG
	template <typename Traits>
	std::ostream& operator<<(std::ostream& stream, ComboNode<Traits> const &NODE)
	{
		stream << "{" << jwm::to_string(NODE.state()) << ", " << NODE.action() << ", " << NODE.path_cost() << "}";
		return stream;
	}
#endif
	

	template <typename Traits,
		template <typename PathCost, typename State, typename Action> class StepCostPolicy,
		template <typename State, typename Action> class ResultPolicy,
		template <typename Traits_> class CreatePolicy = DefaultNodeCreator>
	class DefaultChildPolicy :
		protected virtual StepCostPolicy<typename Traits::pathcost, typename Traits::state, typename Traits::action>,
		protected virtual ResultPolicy<typename Traits::state, typename Traits::action>,
		protected virtual CreatePolicy<Traits>
	{
		typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;

		using StepCostPolicy<PathCost, State, Action>::step_cost;
		using ResultPolicy<State, Action>::result;
		using CreatePolicy<Traits>::create;
		
	protected:
		DefaultChildPolicy() {}
		~DefaultChildPolicy() {}

		Node child(Node const &PARENT, Action const &ACTION) const
		{
			return create(result(PARENT->state(), ACTION), PARENT, ACTION, PARENT->path_cost() + step_cost(PARENT->state(), ACTION));
		}
	};


	template <typename Traits,
			 template <typename PathCost, typename State, typename Action> class StepCostPolicy,
			 template <typename State, typename Action> class ActionsPolicy,
			 template <typename State, typename Action> class ResultPolicy,
			 template <typename State> class GoalTestPolicy,
			 template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
			 template <typename Traits_,
				template <typename PathCost, typename State, typename Action> class StepCostPolicy_,
				template <typename State, typename Action> class ResultPolicy_,
				template <typename Traits__> class CreatePolicy>
				class ChildPolicy = DefaultChildPolicy>
	class Problem :
		protected virtual StepCostPolicy<typename Traits::pathcost, typename Traits::state, typename Traits::action>,
		protected virtual ActionsPolicy<typename Traits::state, typename Traits::action>,
		protected virtual ResultPolicy<typename Traits::state, typename Traits::action>,
		protected virtual GoalTestPolicy<typename Traits::state>,
		protected virtual ChildPolicy<Traits, StepCostPolicy, ResultPolicy, CreatePolicy>,
		protected virtual CreatePolicy<Traits>
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

		using ChildPolicy<Traits, StepCostPolicy, ResultPolicy, CreatePolicy>::child;
		using StepCostPolicy<PathCost, State, Action>::step_cost;
		using ActionsPolicy<State, Action>::actions;
		using ResultPolicy<State, Action>::result;
		using GoalTestPolicy<State>::goal_test;
		using CreatePolicy<Traits>::create;
		
	private:
		State const INITIAL;
	};
}

#endif
