#ifndef JSEARCH_QUEUE_SET_HPP
#define JSEARCH_QUEUE_SET_HPP 1

/**
 * @file queue_set.hpp
 *
 * Container suitable for A* searchs.
 *
 * Efficient implementation of A* requires access both by identity and
 * by (lowest) cost.  No single simple data structure provides both.
 *
 * @author Anthony Foiani <anthony.foiani@gmail.com>
 *
 * @butcher Jeremy W. Murphy <jeremy.william.murphy@gmail.com>
 * 
 * License: Boost License v1.0 -- http://www.boost.org/users/license.html
 */

#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <map>

#ifndef NDEBUG
#  include <iostream>
#  define DEBUG( x ) std::clog << "queue_set: " << x << std::endl
#else
#  define DEBUG( x ) do {} while ( 0 )
#endif

namespace jsearch
{
	namespace detail
	{
		template <typename T>
		struct ptr_less_type
		{
			bool operator()(const T * const lhs_ptr, const T * const rhs_ptr) const
			{
				return *lhs_ptr < *rhs_ptr;
			}
		};
	} // end namespace jsearch::detail

	/**
	 * Provide access to nodes by name or by lowest cost.
	 *
	 * We store and access the nodes via pointer, typically a shared_ptr.
	 *
	 * @tparam Node pointer to node that we're indexing.
	 *
	 * The pointer type must provide the "element_type" type, and that
	 * type (the actual "Node") must contain the following inner types:
	 *
	 *   Node::State
	 *   Node::PathCost
	 *
	 * And it must provide access to those values via:
	 *
	 *   const Node::State    & state()     const;
	 *   const Node::PathCost & path_cost() const;
	 */

	template <template <typename T> class PriorityQueue, typename Node>
	class queue_set
	{
		/** Type used to identify particular nodes. */
		typedef typename Node::element_type::State State;
		/** Type used to hold path cost (lower is better ). */
		typedef typename Node::element_type::PathCost Cost;
		
		typedef PriorityQueue<Node> PQ;
		// A handle to elements in the priority queue.
		typedef typename PQ::handle_type handle_t;

		typedef State const * key_type;
		typedef handle_t mapped_type;
		
		// A mapping from State -> PriorityQueue::handle_type (an element on the queue).
		typedef std::map<key_type, mapped_type, detail::ptr_less_type<State>> Map;
		
		// ... and some convenience typedefs derived from it
		typedef typename Map::iterator       s_iter_t;
		typedef typename Map::const_iterator s_const_iter_t;
		typedef typename Map::reference		 s_ref_t;
		typedef typename Map::value_type     s_item_t;

	public:
		// This data structure should appear as identical to a Boost priority queue as possible.
		// The only real difference in interface should be that we have a custom member find().
		typedef typename PQ::size_type size_type;
		typedef typename PQ::value_type value_type;
		typedef typename PQ::difference_type difference_type;
		
		typedef typename PQ::reference reference;
		typedef typename PQ::const_reference const_reference;

		/** Create a new queue_set. */
		queue_set() : ss(), priority_queue() {}

		/** Destroy this queue_set. */
		~queue_set() {}

		/**
		 * Push @a node on to the priority queue.
		 *
		 * If a node with the same state is already on the queue, an exception is thrown.
		 *
		 * @param[in] NODE constant reference to a Node.
		 *
		 */
		void push(Node const &NODE); // Customization point, defined out-of-class.
		
		/**
		 * See the documentation of Boost.heap for these priority queue functions.
		 */
		const_reference top() const { return priority_queue.top(); }
		void pop(); // Customization point, defined out-of-class.
		bool empty() const { return priority_queue.empty(); }
		size_type size() const { return priority_queue.size(); }


		// Mutable interface, all customization points, so defined out-of-class.
		void update(handle_t const &HANDLE, value_type const &NODE) { priority_queue.update(HANDLE, NODE); }
		void increase(handle_t const &HANDLE, value_type const &NODE) { priority_queue.increase(HANDLE, NODE); }
		void decrease(handle_t const &HANDLE, value_type const &NODE) { priority_queue.decrease(HANDLE, NODE); }


		/*******************
		 *	Map interface
		 *******************/
		/*
		s_iter_t begin() { return ss.begin(); }
		s_const_iter_t begin() const { return ss.cbegin(); }
		s_const_iter_t cbegin() const { return ss.cbegin(); }
		s_iter_t end() { return ss.end(); }
		s_const_iter_t end() const { return ss.end(); }
		s_const_iter_t cend() const { return ss.cend(); }
		*/
		s_iter_t find(key_type const &KEY);
		s_const_iter_t find(key_type const &KEY) const;

	private:
		// set of indexes into nodes, locatable by s
		Map ss;

		PQ priority_queue;
	};

// ---------------------------------------------------------------------

	template <template <typename T> class PriorityQueue, typename Node>
	inline void queue_set<PriorityQueue, Node>::push(Node const &NODE)
	{
		// Client assumes that NODE is not on the queue and wants to push it on.
		// It is thus a precondition that NODE is not on the queue and we throw an exception if it is.

		auto const &STATE(NODE->state());

		if(ss.find(STATE) == std::end(ss))
		{
			auto const HANDLE(priority_queue.push(NODE));
			typename Map::value_type const P(&STATE, HANDLE); // Is this legitimate use of the Map typedef in a function template?
			auto const INSERT_RESULT(ss.insert(P));
			if(!INSERT_RESULT.second)
			{
				std::ostringstream tmp;
				tmp << "Priority queue failed to insert a state with this value: " << STATE;
				throw std::logic_error(tmp.str());
			}
		}
		else
		{
			std::ostringstream tmp;
			tmp << "Priority queue alreadys contains a state with this value: " << STATE;
			throw std::logic_error(tmp.str());
		}
	}


	template <template <typename T> class PriorityQueue, typename Node>
	inline void queue_set<PriorityQueue, Node>::pop()
	{
		auto const &NODE(priority_queue.top());
		auto const &STATE(NODE->state());
		auto const REMOVED(ss.erase(&STATE));
		std::ostringstream tmp;
		switch(REMOVED)
		{
			case 0:
				tmp << "Failed to remove " << STATE << " from PQ::element lookup table.";
				throw std::logic_error(tmp.str());
				break;

			case 1:
				priority_queue.pop();
				break;
				
			default: // REMOVED > 1
				tmp << "Removed multiple " << STATE << "entries from PQ::element lookup table.";
				throw std::logic_error(tmp.str());
				break;
		}
	}
} // end namespace jsearch

#undef DEBUG

#endif // JSEARCH_QUEUE_SET_HPP

// Local Variables:
// mode: c++
// tab-width: 4
// indent-tabs-mode: t
// End:
