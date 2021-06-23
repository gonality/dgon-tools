// This program reads a bunch of graphs from standard input, and converts them to graph6.
// Optionally, this program can be instructed to compute a regular subdivision before
// saving the graph in the graph6 format, so as to also accommodate graphs with parallel
// edges (which is not allowed in graph6 format).
// 
// In time, a better solution would be to shift to sparse6 format, but that would entail
// writing and testing extra code in the back-end. For now, this will do.
// 
// The input should consist of any number of blocks of the following form:
//     * One line indicating the name of the graph (will NOT be stored in graph6 file!);
//     * One line with two integers N and M, indicating the number of vertices and edges;
//     * M lines containing two integers v_i and w_i (0 ≤ v_i, w_i < N), indicating that
//       there is an (undirected) edge between v_i and w_i.
// 
// Empty lines in the input will be ignored.


#include "graphs.h"
#include "subdivisions.h"
#include "graph_io.h"
#include "graph6.h"
#include <iostream>
#include <cassert>
#include <cstdio>
#include <string>

using namespace std;

int subdiv_num = -1;

void solve(const my_graph& G) {
	assert(G.is_valid_undirected_graph());
	assert(subdiv_num == -1 || (subdiv_num >= 2 && subdiv_num <= MAX_PARTS_PER_EDGE));
	my_graph H = (subdiv_num == -1 ? G : subdivide(G, subdiv_num));
	assert(H.is_valid_undirected_graph()); // check validy AND populate __adj_matr[][]
	for (int i = 0; i < H.n; i++) {
		for (int j = 0; j < H.n; j++) {
			assert(__adj_matr[i][j] >= 0);
			if (__adj_matr[i][j] > 1) {
				cerr << "ERROR: graph must be simple (no parallel edges) to be stored in graph6 format! Skipping graph \"" << G.graph_name << "\"." << endl;
				return;
			}
		}
	}
	cout << write_graph6(H) << endl;
}

int parse_arg_as_int(char* argv1) {
	char tmp[200];
	int s;
	if (sscanf(argv1, "%d", &s) != 1) {
		throw 1;
	}
	sprintf(tmp, "%d", s);
	if (string(tmp) != string(argv1)) {
		throw 1;
	}
	return s;
}

void print_usage(char* argv0) {
	cerr << "USAGE: " << argv0 << " [s]" << endl;
	cerr << "       where the optional argument s is an integer in the range [2," << MAX_PARTS_PER_EDGE << "]" << endl;
	cerr << "       denoting the order of the subdivision to be taken." << endl;
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		try {
			subdiv_num = parse_arg_as_int(argv[1]);
			if (subdiv_num < 2 || subdiv_num > MAX_PARTS_PER_EDGE) {
				cerr << "Warning: given argument (" << argv[1] << ") out of range. Ignoring." << endl;
				print_usage(argv[0]);
				subdiv_num = -1;
			}
		}
		catch (int) {
			cerr << "Warning: failed to parse arg \"" << argv[1] << "\" as a positive integer. Ignoring." << endl;
			print_usage(argv[0]);
			subdiv_num = -1;
		}
	}
	if (argc > 2) {
		cerr << "Warning: ignoring excess args." << endl;
		print_usage(argv[0]);
	}
	assert(subdiv_num == -1 || (subdiv_num >= 2 && subdiv_num <= MAX_PARTS_PER_EDGE));
	read_plain_input_and_process(cin, solve);
	return 0;
}

