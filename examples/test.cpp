#include <memory>
#include <queue>
#include <random>
#include <iostream>
#include <locale>
#include <set>
#include <sstream>
#include <boost/heap/pairing_heap.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/binomial_heap.hpp>
#include <boost/heap/priority_queue.hpp>

#include "problem.h"
#include "evaluation.h"
#include "random.h"

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



template <template <typename Key, typename Compare = std::less<Key>, typename Alloc = std::allocator<Key>> class Set, typename Key, class Compare>
Key pop(Set<Key, Compare> &s)
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


template <template <typename Key, typename Compare = std::less<Key>, typename Alloc = std::allocator<Key>> class Set, typename Key, class Compare>
void push(Set<Key, Compare> &s, Key const &E)
{
	s.insert(E);
}


template <template <class T, class ...Options> class PriorityQueue, class T, class ...Options>
void push(PriorityQueue<T, Options...> &pq, T const &E)
{
	pq.push(E);
}

template <typename T>
using PriorityQueue = boost::heap::fibonacci_heap<T, boost::heap::compare<Dijkstra<Random>>>;

int main(int argc, char **argv)
{
	typedef std::shared_ptr<Random::node> OpenListElement;
	// typedef PriorityQueue<OpenListElement> OpenList;
	typedef std::set<OpenListElement> OpenList;
	istringstream(argv[1]) >> max_nodes;
	cout << "max nodes: " << max_nodes << "\n";
	OpenList open;
	uniform_int_distribution<int> const distribution(0, 99);
	mt19937 const engine;
	auto generator(bind(distribution, engine));
	cout.imbue(locale(""));

	Random::state INITIAL(B);
	Problem<Random, Distance, Neighbours, Visit, GoalTest> const PROBLEM(INITIAL);
	
	push(open, std::make_shared<Random::node>(0, std::shared_ptr<Random::node>(), 0, 0));
	// Do dummy A*.
	while(!open.empty())
	{
		OpenListElement const S(pop(open));

		if(open.size() % 1000 == 0)
			cout << "size: " << open.size() << endl;
		
		if(PROBLEM.goal_test(S->state()))
			return 0;
		else
		{
			// No closed list, ignore parent.
			// size_t const ACTIONS(generator());
			auto const ACTIONS(PROBLEM.actions(S->state()));
			for(auto ACTION : ACTIONS)
			{
				// OpenListElement const CHILD(std::make_shared<Random::node>(ACTION, S, ACTION, S->path_cost() + ACTION));
				OpenListElement const CHILD(std::make_shared<Random::node>(PROBLEM.result(S->state(), ACTION), S, ACTION, S->path_cost() + PROBLEM.step_cost(S->state(), ACTION)));
				push(open, CHILD);
			}
		}
	}
}
