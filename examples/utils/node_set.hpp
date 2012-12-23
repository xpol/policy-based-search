#ifndef JSEARCH_NODE_SET_HPP
#define JSEARCH_NODE_SET_HPP 1

/**
 * @file node_set.hpp
 *
 * Container suitable for A* searchs.
 *
 * Efficient implementation of A* requires access both by identity and
 * by (lowest) cost.  No single simple data structure provides both.
 *
 * @author Anthony Foiani <anthony.foiani@gmail.com>
 *
 * License: Boost License v1.0 -- http://www.boost.org/users/license.html
 */

#include <ostream>

#include <string>
#include <memory>
#include <map>
#include <vector>

#ifndef NDEBUG
#  include <iostream>
#  define DEBUG( x ) std::clog << "node_set: " << x << std::endl
#else
#  define DEBUG( x ) do {} while ( 0 )
#endif

namespace jsearch
{

namespace detail
{

template < typename T >
struct ptr_less_type
{
	bool operator () ( const T * lhs_ptr,
					   const T * rhs_ptr ) const
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
 * @tparam NodePtr pointer to node that we're indexing.
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

template < typename NodePtr >
class node_set
{

public:

	/** Type used to identify particular nodes. */
	typedef typename NodePtr::element_type::State Id;

	/** Type used to hold path cost (lower is better ). */
	typedef typename NodePtr::element_type::PathCost Cost;

	/** Create a new node_set. */
	node_set();

	/** Destroy this node_set. */
	~node_set();

	/**
	 * Insert @a node into the set or update existing node.
	 *
	 * If no node with same id exists, a new node is inserted.  If
	 * such a node does already exist, then the cost of the existing
	 * node is compared with the cost in @a node.  If the cost in @a
	 * node is lower, then the existing node has its cost and info
	 * replaced by that in @a node.
	 *
	 * @param[in] np pointer to node to add / update.
	 *
	 * @return true if inserted / updated, false otherwise.
	 */
	bool insert_or_update( NodePtr np );

	/**
	 * Retrieve the lowest-cost node in the set.
	 *
	 * @return pointer to lowest-cost node, or an unset pointer.
	 */
	NodePtr get_min_cost_node() const;

	/**
	 * Retrieve lowest-cost node, and remove it from the set.
	 *
	 * @return pointer to lowest-cost node, or an unset pointer.
	 */
	NodePtr get_and_pop_min_cost_node();

	/**
	 * Retrieve node with the given @a id (if any).
	 *
	 * @param[in] id identifier to search for.
	 *
	 * @return pointer to node with the given id, or an unset pointer.
	 */
	NodePtr get_node_by_id( const Id & id ) const;

	/**
	 * Retrieve and delete node with the given @a id (if any).
	 *
	 * @param[in] id identifier to search for
	 *
	 * @return pointer to node with the given id, or an unset pointer.
	 */
	NodePtr get_and_pop_node_by_id( const Id & id );

	/**
	 * Determine whether this set is empty.
	 *
	 * @returns true if empty, false otherwise.
	 */
	bool empty() const;

	/**
	 * Get number of elements in this set.
	 *
	 * @returns number of elements in the set (0 if empty).
	 */
	std::size_t size() const;

	/**
	 * Emit debugging information to @a os.
	 *
	 * @param[in] os stream to print information to.
	 */
	void dump( std::ostream & os ) const;

private:

	// that those have to be indexes, not pointers, since the
	// underlying storage can resize.
	typedef std::size_t node_index_t;

	// a type for looking up priv_value_types, indexed by Id
	typedef std::map< const Id *, node_index_t,
					  detail::ptr_less_type< Id > > id_to_nx_t;

	// ... and some convenience typedefs derived from it
	typedef typename id_to_nx_t::iterator       id_iter_t;
	typedef typename id_to_nx_t::const_iterator id_const_iter_t;
	typedef typename id_to_nx_t::value_type     id_item_t;

	// a type for a heap for finding current min cost
	typedef std::vector< node_index_t > cost_to_nx_heap_t;

	// and a location in that heap:
	typedef std::size_t heap_index_t;

	// for internal use, we need to add a index into our heap
	struct priv_value_t
	{
		NodePtr np;         // "node pointer"
		heap_index_t hx;    // "heap index"
		id_iter_t ii;       // "id iterator"
	};

	// a type for main storage for the info
	typedef std::vector< priv_value_t > priv_value_vec_t;

	// actual storage for values.  these aren't in any particular
	// order.
	priv_value_vec_t nodes;

	// set of indexes into nodes, locatable by id
	id_to_nx_t ids;

	// heap of node indexes, ordered by cost
	cost_to_nx_heap_t heap;

	// switch the entries at hx1 and hx2 in the heap
	void swap_heap( const heap_index_t hx1, const heap_index_t hx2 );

	// repair the heap after inserting/adjusting an element at heap
	// index hx
	void fix_heap( heap_index_t hx );

	// erase the node at node index nx
	NodePtr erase_node( const node_index_t nx );
};

// ---------------------------------------------------------------------

template < typename NodePtr >
node_set< NodePtr >::node_set()
	: nodes(),
	  ids(),
	  heap()
{
}

template < typename NodePtr >
node_set< NodePtr >::~node_set()
{
}

// ---------------------------------------------------------------------

template < typename NodePtr >
inline bool
node_set< NodePtr >::insert_or_update( NodePtr np )
{
	const Id * idp( &( np->state() ) );

	id_iter_t ii( ids.find( idp ) );

	if ( ii == ids.end() )
	{
		// this is where the new node will live (eventually)
		const node_index_t nx( nodes.size() );

		// we now add it to the id lookup structure
		bool inserted;
		std::tie( ii, inserted ) = ids.insert( id_item_t( idp, nx ) );

		// and put it at the end of the heap
		const heap_index_t hx( heap.size() );
		heap.push_back( nx );

		// finally, we store the actual node data:
		nodes.push_back( { np, hx, ii } );

		fix_heap( hx );
		return true;
	}

	const node_index_t nx( ii->second );
	priv_value_t & node( nodes[ nx ] );
	if ( node.np->path_cost() > np->path_cost() )
	{
		node.np = np;
		ids.erase( node.ii );
		ids.insert( { &( np->state() ), nx } );
		fix_heap( node.hx );
		return true;
	}

	return false;
}

// ---------------------------------------------------------------------

template < typename NodePtr >
inline NodePtr
node_set< NodePtr >::get_min_cost_node() const
{
	if ( heap.empty() )
		return NodePtr();
	else
		return nodes[ heap[ 0 ] ].np;
}

template < typename NodePtr >
inline NodePtr
node_set< NodePtr >::get_and_pop_min_cost_node()
{
	if ( heap.empty() )
		return NodePtr();
	else
		return erase_node( heap[ 0 ] );
}

// ---------------------------------------------------------------------

template < typename NodePtr >
inline NodePtr
node_set< NodePtr >::get_node_by_id( const Id & id ) const
{
	id_const_iter_t cit( ids.find( &id ) );
	if ( cit == ids.end() )
		return NodePtr();
	else
		return nodes[ cit->second ].np;
}

template < typename NodePtr >
inline NodePtr
node_set< NodePtr >::get_and_pop_node_by_id( const Id & id )
{
	id_const_iter_t cit( ids.find( &id ) );
	if ( cit == ids.end() )
		return NodePtr();
	else
		return erase_node( cit->second );
}

// ---------------------------------------------------------------------

template < typename NodePtr >
inline bool
node_set< NodePtr >::empty() const
{
	return nodes.empty();
}

template < typename NodePtr >
inline std::size_t
node_set< NodePtr >::size() const
{
	return nodes.size();
}

template < typename NodePtr >
inline void
node_set< NodePtr >::dump( std::ostream & os ) const
{
	os << "| priv nodes:" << std::endl;
	for ( node_index_t i = 0; i < nodes.size(); ++i )
		os << "|   "
		  "node[" << i << "]: "
		  "id='" << nodes[i].np->state() << "', "
		  "cost=" << nodes[i].np->path_cost() << ", "
		  "hx=" << nodes[i].hx << ", "
		  "ii=" << reinterpret_cast< const void * >( &( *( nodes[i].ii ) ) ) << std::endl;

	os << "| id map:" << std::endl;
	for ( const id_item_t & i : ids )
		os << "|   "
		  "id='" << *( i.first ) << "', "
		  "nx=" << i.second << std::endl;

	os << "| heap:" << std::endl;
	for ( heap_index_t hx = 0; hx < heap.size(); ++hx )
		os << "|   "
		  "hx=" << hx << ", "
		  "cost=" << nodes[ heap[ hx ] ].np->path_cost() << ", "
		  "nx=" << heap[ hx ] << std::endl;
}

template < typename NodePtr >
inline void
node_set< NodePtr >::swap_heap( const heap_index_t hx1,
								const heap_index_t hx2 )
{
	DEBUG( "swap_heap: hx1=" << hx1 << ", hx2=" << hx2 );

	// where are the two heap entries currently pointing?
	const node_index_t nx1 = heap[ hx1 ];
	const node_index_t nx2 = heap[ hx2 ];

	// swap the heap to node storage pointers
	heap[ hx1 ] = nx2;
	heap[ hx2 ] = nx1;

	// then update the pointers from node storage to heap
	nodes[ nx1 ].hx = hx2;
	nodes[ nx2 ].hx = hx1;
}

template < typename NodePtr >
inline void
node_set< NodePtr >::fix_heap( heap_index_t hx )
{
	DEBUG( "fix_heap: hx=" << hx );

	if ( hx >= heap.size() )
		return;

	// cheat sheet:
	//   hx:     0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
	//   parent: .  0  0  1  1  2  2  3  3  4  4  5  5  6  6  7  7  8  8

	// push new value down
	if ( hx > 0 )
	{
		heap_index_t parent = ( hx - 1 ) / 2;

		while ( parent < hx &&
				nodes[ heap[ hx     ] ].np->path_cost() <
				nodes[ heap[ parent ] ].np->path_cost() )
		{
			DEBUG( "fix_heap: down: hx=" << hx << ", parent=" << parent );
			swap_heap( hx, parent );
			hx = parent;
			parent = ( hx - 1 ) / 2;
		}
	}

	// and then bubble it back up to the top
	while ( true )
	{
		const heap_index_t child1 = hx * 2 + 1;
		if ( child1 >= heap.size() )
			break;

		const heap_index_t child2 = child1 + 1;

		std::size_t child = child1;
		if ( child2 < heap.size() &&
			 nodes[ heap[ child2 ] ].np->path_cost() <
			 nodes[ heap[ child1 ] ].np->path_cost() )
			child = child2;

		DEBUG( "fix_heap: up: hx=" << hx << ", child=" << child );

		if ( nodes[ heap[ hx    ] ].np->path_cost() <
			 nodes[ heap[ child ] ].np->path_cost() )
			break;

		swap_heap( hx, child );
		hx = child;
	}
}

template < typename NodePtr >
inline NodePtr
node_set< NodePtr >::erase_node( const node_index_t nx )
{
	priv_value_t & node( nodes[ nx ] );

	NodePtr rv( node.np );

	// if this is the last node, just blow everything away.
	if ( nodes.size() == 1 )
	{
		nodes.clear();
		ids.clear();
		heap.clear();
		return rv;
	}

	// remove from id mapping
	ids.erase( node.ii );

	// remove from heap
	if ( node.hx + 1 < heap.size() )
	{
		const heap_index_t hx( node.hx );
		heap[ hx ] = heap.back();
		heap.pop_back();
		nodes[ heap[ hx ] ].hx = hx;
		fix_heap( hx );
	}
	else
	{
		heap.pop_back();
	}

	if ( nx + 1 < nodes.size() )
	{
		// copy last element over now-vacant slot
		node = nodes.back();

		// update the id pointer for the last node
		node.ii->second = nx;

		// and update the heap to point at the correct slot
		heap[ node.hx ] = nx;
	}

	// finally, blow away the item in the last slot
	nodes.pop_back();

	return rv;
}

} // end namespace jsearch

#undef DEBUG

#endif // JSEARCH_NODE_SET_HPP

// Local Variables:
// mode: c++
// tab-width: 4
// indent-tabs-mode: t
// End:
