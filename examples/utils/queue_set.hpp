#ifndef JSEARCH_QUEUE_SET_HPP
#define JSEARCH_QUEUE_SET_HPP 1

/*
    queue_set.hpp: Generic priority queue supported by hashed value lookup.
    Copyright (C) 2013  Jeremy W. Murphy <jeremy.william.murphy@gmail.com>
    Special thanks to Anthony Foiani for laying the groundwork from which it sprang.

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

/* INTERFACE
 * 
 * This structure is extremely similar to a Boost.Heap.  The interfaces
 * have been largely duplicated intact, with the major difference being
 * the replacement of iterable by the one from the supporting Map.
 *
 * Currently, detection of duplicates and acting appropriately are left
 * up to the client.  I could not see a strong argument to support push
 * and erase methods that are more intelligent/fail silently.
 */ 
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <utility>


namespace jsearch
{
	/**
	 * Provide access to Nodes on the queue by State or by lowest cost.
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
	 */

	template <class PriorityQueue, template <typename Key, typename Value> class Map>
	class queue_set
	{
	public:
		// This data structure should appear as identical to a Boost priority queue as possible.
		// The only real difference in interface should be that we have a custom member find().
		typedef typename PriorityQueue::size_type size_type;
		typedef typename PriorityQueue::value_type value_type;
		typedef typename PriorityQueue::difference_type difference_type;
		
		typedef typename PriorityQueue::reference reference;
		typedef typename PriorityQueue::const_reference const_reference;
		typedef typename PriorityQueue::pointer pointer;
		typedef typename PriorityQueue::const_pointer const_pointer;

		typedef typename PriorityQueue::handle_type handle_type;
		typedef typename value_type::element_type::State key_type;
		typedef handle_type mapped_type;

	private:
		// A mapping from State -> PriorityQueue::handle_type (a handle on an element).
		typedef Map<key_type, mapped_type> StateHandleMap;
		
	public:
		// The iterable interface for this structure, from the map.
		typedef typename StateHandleMap::iterator       iterator;
		typedef typename StateHandleMap::const_iterator const_iterator;
		typedef typename StateHandleMap::reference		 map_reference;
		typedef typename StateHandleMap::value_type     map_value_type;

		// Use the default constructors and destructors.
		
		/**
		 * Push @a node on to the priority queue.
		 *
		 * If a node with the same state is already on the queue, an exception is thrown.
		 *
		 * @param[in] NODE constant reference to a Node.
		 *
		 */
		void push(value_type const &NODE); // Customization point, defined out-of-class.
		
		/**
		 * See the documentation of Boost.Heap for these functions.
		 */
		const_reference top() const { return priority_queue.top(); }
		void pop(); // Customization point, defined out-of-class.
		bool empty() const { return priority_queue.empty(); }
		size_type size() const { return priority_queue.size(); }


		/** Mutable interface.
		 */
		void update(handle_type const &HANDLE, value_type const &NODE) { priority_queue.update(HANDLE, NODE); }
		void increase(handle_type const &HANDLE, value_type const &NODE) { priority_queue.increase(HANDLE, NODE); }
		void decrease(handle_type const &HANDLE, value_type const &NODE) { priority_queue.decrease(HANDLE, NODE); }


		/*****************************
		 *	Map (iterable) interface
		 *****************************/
		iterator begin() { return map.begin(); }
		const_iterator begin() const { return map.cbegin(); }
		const_iterator cbegin() const { return map.cbegin(); }
		iterator end() { return map.end(); }
		const_iterator end() const { return map.end(); }
		const_iterator cend() const { return map.cend(); }
		iterator find(key_type const &KEY) { return map.find(KEY); }
		const_iterator find(key_type const &KEY) const { return map.find(KEY); }

	private:
		StateHandleMap map; // State ↦ handle_type.
		PriorityQueue priority_queue;
	};

// ---------------------------------------------------------------------

	template <class PriorityQueue, template <typename Key, typename Value> class Map>
	inline void queue_set<PriorityQueue, Map>::push(const value_type& NODE)
	{
		// Client assumes that NODE is not on the queue and wants to push it on.
		// It is thus a precondition that NODE is not on the queue and we throw an exception if it is.

		auto const &STATE(NODE->state());

		if(map.find(STATE) == std::end(map))
		{
			auto const HANDLE(priority_queue.push(NODE));
			auto const P(std::make_pair(STATE, HANDLE));
			auto const INSERT_RESULT(map.insert(P));
			if(!INSERT_RESULT.second)
			{
				std::ostringstream tmp;
				tmp << "Priority queue failed to insert a state with this value: " << STATE;
				throw std::runtime_error(tmp.str());  // Mysterious internal error.
			}
		}
		else
		{
			std::ostringstream tmp;
			tmp << "Priority queue alreadys contains a state with this value: " << STATE;
			throw std::logic_error(tmp.str()); // Client error.
		}
	}


	template <class PriorityQueue, template <typename Key, typename Value> class Map>
	inline void queue_set<PriorityQueue, Map>::pop()
	{
		auto const &NODE(priority_queue.top());
		auto const &STATE(NODE->state());
		auto const REMOVED(map.erase(STATE));
		std::ostringstream tmp;
		switch(REMOVED)
		{
			case 0:
				tmp << STATE << " was not in the lookup table.";
				throw std::logic_error(tmp.str());
				break;

			case 1:
				priority_queue.pop();
				break;
				
			default: // REMOVED > 1
				tmp << "Removed multiple " << STATE << "entries from lookup table.";
				throw std::logic_error(tmp.str());
				break;
		}
	}
} // end namespace jsearch

#endif // JSEARCH_QUEUE_SET_HPP
