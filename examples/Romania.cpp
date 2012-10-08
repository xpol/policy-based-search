#include "Romania.h"
#include "search.h"

#include <iostream>
#include <algorithm>

using namespace jsearch;
using namespace std;

typedef Romania::state State;
typedef Romania::node Node;


template <class charT, class traits>
basic_ostream<charT, traits>& print(Node const &NODE, basic_ostream<charT, traits>& os);


int main(int, char **)
{
	State const Sibiu("Sibiu");

	Problem<Romania, Distance, Neighbours, Visit, GoalTest> const Bucharest(Sibiu); // Use Sibiu as initial state.
	Evaluation<EuclideanDistance> const evaluation;

	auto const solution = jsearch::search(Bucharest, evaluation);

	print(solution, cout) << endl;
}


template <class charT, class traits>
basic_ostream<charT, traits>& print(Node const &NODE, basic_ostream<charT, traits>& os)
{
	
	if(NODE.parent)
	{
		print(*NODE.parent, os);
		os << " => ";
	}
	
	os << NODE.state;
	
	return os;
}
