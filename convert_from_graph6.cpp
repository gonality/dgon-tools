// Convert graph6 file to my own custom graph format, as documented in the following programs:
//     * graph_io.h
//     * find_gonality.cpp
//     * convert_to_graph6.cpp
// 
// Alternatively, one may use the program listg from nauty to convert graph6 strings to
// adjacency matrices.
// 
// WARNING: don't try to do this for large graph6 files, as the output will be huge!

#include "graphs.h"
#include "graph6.h"
#include "graph_io.h"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <cstdio>

using namespace std;

int count_graphs = 0;

void parseGraph(const string& s) {
	my_graph G = parse_graph6(s);
	assert(G.is_valid_undirected_graph());
	char* tmp = new char[300 + s.size()];
	assert(tmp != NULL);
	sprintf(tmp, "Graph %d (\"%s\")", ++count_graphs, s.c_str());
	G.graph_name = string(tmp);
	print_plain_output(cout, G);
	delete[] tmp;
}

int main() {
	string s;
	while (getline(cin, s)) {
		parseGraph(s);
	}
	return 0;
}
