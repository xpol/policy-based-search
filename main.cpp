#include "examples/Romania.h"
#include "search.h"

using namespace jsearch;


int main(int argc, char **argv)
{
	Romania::state const Arad { "Arad" }, Sibiu("Sibiu");

	Problem<Romania, Distance, Neighbours, Visit, GoalTest> Bucharest(Sibiu);
	Evaluation<EuclideanDistance> evaluation;

	auto solution = jsearch::search(Bucharest, evaluation);
}
