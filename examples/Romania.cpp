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
	State const INITIAL("Foo");

	Problem<Romania, Distance, Neighbours, Visit, GoalTest> const BUCHAREST(INITIAL); // The problem is to get to Bucharest.
	Evaluation<EuclideanDistance> const EVALUATION;

	try
	{
		auto const SOLUTION = jsearch::best_first_search(BUCHAREST, EVALUATION);
		
		print(SOLUTION, cout) << endl;
	}
	catch (goal_not_found const &ex)
	{
		cout << "No path from " << INITIAL << " to the goal could be found!\n";
	}
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
