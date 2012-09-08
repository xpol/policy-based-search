
#include "problem.h"

#include <queue>
#include <set>

namespace search
{
	template <typename Traits>
	typename Traits::node bestfirstsearch(Problem const &PROBLEM)
	{
		typedef typename Traits::node Node;
		Node solution;

		std::priority_queue<Node> open;
		std::set<Node> closed;
		open.add(typename Traits::node(null, null, 0, INITIAL));

		while(!solution && !open.empty())
		{

		}

		return solution;
	}
}
