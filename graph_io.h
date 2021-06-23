// Helper functions to read graphs from human-readable ("plain") input.
// 
// The input should consist of any number of blocks of the following form:
//     * One line indicating the name of the graph;
//     * One line with two integers N and M, indicating the number of vertices and edges;
//     * M lines containing two integers v_i and w_i (0 ≤ v_i, w_i < N), indicating that
//       there is an (undirected) edge between v_i and w_i. Parallel edges are allowed.
// 
// This input format is used in the following programs:
//     * find_gonality.cpp
//     * convert_to_graph6.cpp
// 
// All other programs use the more compact graph6 file format (implemented in graph6.h).
// To convert between the two formats, use convert_to_graph6.cpp and convert_from_graph6.cpp.
// Note: the graph6 format does not allow parallel edges; for this the plain format must be used.

#ifndef __GRAPH_IO_H__
#define __GRAPH_IO_H__

#include "graphs.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>


std::vector<std::string> __all_lines;
my_graph __G;

void __parse_next_graph(size_t& pos, void (*process_function)(const my_graph& G)) {
	int n, m;
	assert(pos <= __all_lines.size() - 2); // at least 2 lines of input remaining
	__G.graph_name = __all_lines[pos];
	pos++;
	assert(sscanf(__all_lines[pos].c_str(), "%d %d", &n, &m) == 2);
	assert(n >= 1 && n <= MAX_N);
	assert(m >= 0 && m <= MAX_M);
	pos++;
	__G.setN(n);
	assert(pos <= __all_lines.size() - m); // at least m lines of input remaining
	for (int i = 0; i < m; i++) {
		int a, b;
		assert(sscanf(__all_lines[pos].c_str(), "%d %d", &a, &b) == 2);
		assert(a >= 0 && a < n);
		assert(b >= 0 && b < n);
		assert(a != b);
		pos++;
		__G.add_edge(a, b);
	}
	process_function(__G);
	__G.init();
}

void read_plain_input_and_process(std::istream& is, void (*process_function)(const my_graph& g)) {
	std::string tmp_str;
	__G.init();
	while (std::getline(is, tmp_str)) {
		if (!tmp_str.empty()) {
			__all_lines.push_back(tmp_str);
		}
	}
	size_t pos = 0;
	while (pos < __all_lines.size()) {
		__parse_next_graph(pos, process_function);
	}
}

void print_plain_output(std::ostream& os, const my_graph& G) {
	assert(G.is_valid_undirected_graph());
	os << G.graph_name << std::endl;
	os << G.n << ' ' << G.count_edges() << std::endl;
	for (int i = 0; i < G.n; i++) {
		for (auto j : G.neighbours[i]) {
			if (i < j) {
				os << i << ' ' << j << std::endl;
			}
		}
	}
}

#endif
