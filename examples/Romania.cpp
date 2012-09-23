#include "examples/Romania.h"
#include "search.h"

#include <iostream>
#include <algorithm>

using namespace jsearch;
using namespace std;

typedef Romania::state State;
typedef Romania::node Node;


void print_path(Node const &NODE, std::ostream* out);


int main(int, char **)
{
	State const Sibiu("Sibiu");

	Problem<Romania, Distance, Neighbours, Visit, GoalTest> const Bucharest(Sibiu); // Use Sibiu as initial state.
	Evaluation<EuclideanDistance> const evaluation;

	auto const solution = jsearch::search(Bucharest, evaluation);

	print_path(solution, &cout);
}


void print_path(Node const &NODE, std::ostream* out)
{
	if(NODE.parent)
	{
		print_path(*NODE.parent, out);
		*out << " => ";
	}
	
	*out << NODE.state;
}
