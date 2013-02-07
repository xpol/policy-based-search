/*
    gg.hpp: Graph Generator.
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
 * @file gg.hpp
 * @brief Generate randomly weighted graphs of arbitrary size and branching factor in DOT format.
 * Currently limited to the adjacency matrix type.
 */

#include <boost/graph/adjacency_matrix.hpp>
#include <random>
#include <chrono>
#include <type_traits>
#include <algorithm>
#include <stdexcept>

#ifndef NDEBUG
#include <boost/graph/graphviz.hpp>
#include <fstream>
#include <iostream>
#endif


namespace gg
{
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

	#ifndef NDEBUG
		std::cerr << "Generating graph";
	#endif

		std::for_each(VP.first, VP.second, [&](vertex_descriptor const &V)
		{
	#ifndef NDEBUG
			std::cerr << ".";
	#endif
			auto const BODD(B % 2);
			auto const	END(N / 2 + BODD + N % 2),
						START(END - B / 2 - BODD); // Intentional integer division rounding on B.
			for(unsigned i(START); i < END; ++i)
				boost::add_edge(V, (V + i) % N, weight_generator(), g);
		});
	#ifndef NDEBUG
		std::cerr << "done.\n";
	#endif
	}


	void check_preconditions(unsigned const N, unsigned const B)
	{
		if(B < 2)
		{
			throw std::logic_error("Branching factor must be greater than 1.");
		}

		if(N < 3)
		{
			throw std::logic_error("Number of nodes must be greater than 2.");
		}

		if(B >= N)
		{
			throw std::logic_error("Branching factor must be less than number of nodes in graph.");
		}

		if(B % 2 == 1 && N % 2 == 1)
		{
			throw std::logic_error("Sorry, an odd graph size with odd branching factor is not supported.");
		}
	}
}


template <typename Graph, typename Engine>
Graph generate_graph(Graph &g, unsigned const B, Engine engine)
{
	typedef typename boost::property_traits<typename boost::property_map<Graph, boost::edge_weight_t>::type>::value_type Weight;
	
	gg::check_preconditions(boost::num_vertices(g), B);

	typename std::conditional<std::is_integral<Weight>::value, std::uniform_int_distribution<Weight>, std::uniform_real_distribution<Weight>>::type weight_dist(1, 500);
	auto weight_generator(std::bind(weight_dist, engine));

	// Do the actual work.
	gg::construct(g, B, weight_generator);

	assert(gg::correct(g, B));
#ifndef NDEBUG
	std::ofstream gv_output("Random.dot");
	boost::write_graphviz(gv_output, g, boost::default_writer(), boost::make_label_writer(boost::get(boost::edge_weight, g)));
#endif
	return g;
}
