/*
    rgg.cpp: Random Graph Generator.
    Copyright (C) 2013  Jeremy W. Murphy <jeremy.william.murphy@gmail.com>

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

/**
 * @file rgg.cpp
 * @brief Generate randomly weighted graphs of arbitrary size and branching factor in DOT format.
 */

// #include <boost/program_options.hpp>
#include <iostream>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graphviz.hpp>
#include <random>
#include <chrono>
#include <type_traits>
#include <algorithm>

using namespace std;

namespace
{
	typedef unsigned cost_t;
	typedef boost::adjacency_matrix<boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, cost_t>> Graph;
	
	
	struct Options
	{
		unsigned b; // Branching factor.
		unsigned n; // Number of nodes.
		mt19937::result_type seed;
	};
	
	Options init(int argc, char **argv);


	template <typename Graph>
	bool correct(Graph const &G, unsigned const B)
	{
		auto const VI(boost::vertices(G));
		bool result(true);
		for(auto V(VI.first); V < VI.second && result; ++V)
			if(boost::out_degree(*V, G) != B)
				result = false;
		return result;
	}


	template <typename Graph, typename WeightGenerator>
	void construct(Graph &g, size_t const B, WeightGenerator &weight_generator)
	{
		typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
		auto const VP(boost::vertices(g));
		auto const N(boost::num_vertices(g));

		std::for_each(VP.first, VP.second, [&](vertex_descriptor const &V)
		{
			auto const BODD(B % 2);
			auto const	END(N / 2 + BODD + N % 2),
						START(END - B / 2 - BODD); // Intentional integer division rounding on B.
			for(unsigned i(START); i < END; ++i)
				boost::add_edge(V, (V + i) % N, weight_generator(), g);
		});
	}

}


int main(int argc, char **argv)
{
	Options const OPTIONS(init(argc, argv));
	Graph g(OPTIONS.n);

	conditional<is_integral<cost_t>::value, uniform_int_distribution<cost_t>, uniform_real_distribution<cost_t>>::type weight_dist(1, 500);
	auto weight_generator(bind(weight_dist, mt19937(OPTIONS.seed)));

	// Do the actual work.
	construct(g, OPTIONS.b, weight_generator);

	assert(correct(g, OPTIONS.b));
	boost::write_graphviz(cout, g, boost::default_writer(), boost::make_label_writer(boost::get(boost::edge_weight, g)));
}


namespace
{
	Options init(int argc, char **argv)
	{
		Options options;
		string const ARGV0(argv[0]);

		options.seed = chrono::high_resolution_clock::to_time_t(chrono::high_resolution_clock::now());

		// TODO: Convert to Boost.ProgramOptions.
		switch(argc)
		{
			case 4:
				istringstream(argv[3]) >> options.seed;
			case 3:
				istringstream(argv[2]) >> options.n;
				istringstream(argv[1]) >> options.b;
				break;

			case 2:
			case 1:
			case 0:
				cerr << "Invocation: " << ARGV0.substr(ARGV0.find_last_of('/') + 1) << " <(b)ranching factor> <(n)odes> [seed]\nSuch that: b < n\n";
				exit(EXIT_FAILURE);
				break;

			default:
				cerr << ARGV0 + ": Unexpected options on the commandline.\n";
				exit(EXIT_FAILURE);
		}

		if(options.b < 2)
		{
			cerr << "Branching factor must be greater than 1.\n";
			exit(EXIT_FAILURE);
		}

		if(options.n < 3)
		{
			cerr << "Number of nodes must be greater than 2.\n";
			exit(EXIT_FAILURE);
		}

		if(options.b >= options.n)
		{
			cerr << "Branching factor must be less than number of nodes in graph.\n";
			exit(EXIT_FAILURE);
		}

		if(options.b % 2 == 1 && options.n % 2 == 1)
		{
			cerr << "Sorry, an odd graph size with odd branching factor is not supported.\n";
			exit(EXIT_FAILURE);
		}

		return options;
	}
}
