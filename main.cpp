#include "examples/Romania.h"
#include "search.h"

#include <iostream>

using namespace jsearch;
using namespace std;

int main(int argc, char **argv)
{
	Romania::state const Arad { "Arad" }, Sibiu("Sibiu");

	Problem<Romania, Distance, Neighbours, Visit, GoalTest> Bucharest(Sibiu);
	Evaluation<EuclideanDistance> evaluation;

	auto solution = jsearch::search(Bucharest, evaluation);

	cout << "SOLUTION >>> State: " << solution.state << ", Parent: " << solution.parent->state << ", path cost: " << solution.path_cost << endl;
}
