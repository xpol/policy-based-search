#include "problem.h"
#include "evaluation.h"
#include "random.h"

#include <memory>
#include <queue>
#include <random>
#include <iostream>
#include <locale>
#include <set>
#include <sstream>
#include <limits>

#include <boost/heap/pairing_heap.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/binomial_heap.hpp>
#include <boost/heap/priority_queue.hpp>


using namespace std;
using namespace jsearch;

struct Test;
struct Test
{
	typedef int action;
	typedef long state;
	typedef long unsigned int pathcost;
	typedef DefaultNode<Test> node;
	static bool const combinatorial = true;
};


// allocator originally copied from:
//   http://devnikhilk.blogspot.com/2011/08/sample-stl-allocator.html

size_t used;

template < typename T >
class MyAlloc
{
	
public:
	
	typedef T                  value_type;
	typedef value_type *       pointer;
	typedef const value_type * const_pointer;
	typedef value_type &       reference;
	typedef const value_type & const_reference;
	typedef std::size_t        size_type;
	typedef std::ptrdiff_t     difference_type;
	
	template < typename U >
	struct rebind {
		typedef MyAlloc< U > other;
	};
	
	MyAlloc() {}
	~MyAlloc() {}
	explicit MyAlloc( MyAlloc const & ) {}
	template < typename U >
	explicit MyAlloc( MyAlloc< U > const & ) {}
	
	pointer       address( reference r       ) { return &r; }
	const_pointer address( const_reference r ) { return &r; }
	
	pointer allocate( size_type n, std::allocator< void >::const_pointer hint = 0 )
	{
		const size_type bytes( n * sizeof( T ) );
		used += bytes;
		pointer new_memory = reinterpret_cast<pointer>( ::operator new( bytes ) );
		return new_memory;
	}
	void deallocate( pointer p, size_type n )
	{
		const size_type bytes( n * sizeof( T ) );
		used -= bytes;
		::operator delete( p );
	}
	
	size_type max_size() const
	{
		return std::numeric_limits< size_type >::max() / sizeof( T );
	}
	
	void construct( pointer p, const T & t )
	{
		new( p ) T( t );
	}
	void destroy( pointer p )
	{
		p->~T();
	}
	
	bool operator == ( MyAlloc const & other ) { return this == &other; }
	bool operator != ( MyAlloc const & other ) { return ! ( *this == other ); }
};



template <template <typename Key, typename Compare = std::less<Key>, typename Alloc = std::allocator<Key>> class Set, typename Key, typename Compare, typename Alloc>
Key pop(Set<Key, Compare, Alloc> &s)
{
	auto const IT(std::begin(s));
	Key const E(*IT);
	s.erase(IT);
	return E;
}


template <template <class T, class ...Options> class PriorityQueue, class T, class ...Options>
T pop(PriorityQueue<T, Options...> &pq)
{
	auto const E(pq.top());
	pq.pop();
	return E;
}


template <template <typename Key, typename Compare = std::less<Key>, typename Alloc = std::allocator<Key>> class Set, typename Key, typename Compare, typename Alloc>
void push(Set<Key, Compare, Alloc> &s, Key const &E)
{
	s.insert(E);
}


template <template <class T, class ...Options> class PriorityQueue, class T, class ...Options>
void push(PriorityQueue<T, Options...> &pq, T const &E)
{
	pq.push(E);
}


typedef std::shared_ptr<Random::node> OpenListElement;
typedef MyAlloc<OpenListElement> Alloc;

template <typename T>
using PriorityQueue = boost::heap::priority_queue<T, boost::heap::compare<Dijkstra<Random>>, boost::heap::allocator<Alloc>>;

int main(int argc, char **argv)
{
	/***********************************************
	 * 	Switch between these two OpenList typedefs.
	 ***********************************************/
	// typedef PriorityQueue<OpenListElement> OpenList;
	typedef std::set<OpenListElement, Dijkstra<Random>, Alloc> OpenList;


	istringstream(argv[1]) >> max_nodes;
	OpenList open;
	auto generator(bind(uniform_int_distribution<int>(0, 99), mt19937()));
	cout.imbue(locale(""));
	cout << "max nodes: " << max_nodes << "\n";
	cout << "sizeof(" << typeid(Random::node).name() << "): " << sizeof(Random::node) << "\n";
	cout << "sizeof(" << typeid(shared_ptr<Random::node>).name() << "): " << sizeof(shared_ptr<Random::node>) << "\n";
	cout << "typeid(OpenList): " << typeid(OpenList).name() << "\n";
	
	Random::state INITIAL(B);
	Problem<Random, Distance, Neighbours, Visit, GoalTest> const PROBLEM(INITIAL);
	
	push(open, std::make_shared<Random::node>(0, std::shared_ptr<Random::node>(), 0, 0));
	// Do dummy A*.
	while(!open.empty())
	{
		OpenListElement const S(pop(open));

		if(generated % 10000 == 0)
			cout << "generated: " << generated << ", used: " << used << endl;
		
		if(PROBLEM.goal_test(S->state()))
		{
			cout << "Done.\n";
			return 0;
		}
		else
		{
			auto const ACTIONS(PROBLEM.actions(S->state()));
			for(auto ACTION : ACTIONS)
			{
				OpenListElement const CHILD(std::make_shared<Random::node>(PROBLEM.result(S->state(), ACTION), S, ACTION, S->path_cost() + PROBLEM.step_cost(S->state(), ACTION)));
				push(open, CHILD);
			}
		}
	}
}
