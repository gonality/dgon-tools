// This program reads a bunch of graphs from standard input, and computes their gonality.
// 
// Usage:
//       ./find_gonality [-gavv] [k] < infile.in
// 
//       Numerical argument k: if this is specified, the program will take the k-regular
//                             subdivision of every graph before computing the gonality.
//                             (By this we mean that every edge is divided into k equal
//                             parts. So the 1-regular subdivision corresponds with the
//                             original graph.)
// 
//       Input options:
//       -g  : use graph6 input instead of plain input
// 
//       Output options:
//       -a  : find (and show) all optimal v0-reduced divisors
//       -v  : verbose (show the optimal v0-reduced divisor)
//       -vv : extra verbose (show the reduced divisor for every vertex in the graph)
// 
// 
// By default, the input should use the following "plain" format:
//     * One line indicating the name of the graph;
//     * One line with two integers N and M, indicating the number of vertices and edges;
//     * M lines containing two integers v_i and w_i (0 ≤ v_i, w_i < N), indicating that
//       there is an (undirected) edge between v_i and w_i.
// In the plain format, empty lines in the input will be ignored.
// 
// Use the -g option to specify input in graph6 format. This is recommended when processing
// large numbers of graphs, as otherwise the input files get very big.
// 
// Use the auxiliary programs convert_[to/from]_graph6 to convert between the plain format
// and the graph6 format.


#define USAGE_STRING \
"find_gonality [-gavv] [k] < infile.in"

#define HELPTEXT \
" Find the gonality of the graphs specified in the file \"infile.in\".\n\
\n\
\n\
    Numerical arguments:\n\
        k    : number of parts into which every should be divided (default: 1)\n\
\n\
    Input options:\n\
       -g    : use graph6 input instead of plain input\n\
\n\
    Output options:\n\
       -a    : find (and show) all optimal v0-reduced divisors\n\
       -v    : verbose (show the optimal v0-reduced divisor)\n\
       -vv   : extra verbose (show the reduced divisor for every vertex in the graph)\n\
\n\
  See program text for much more information.\n"


// Graph limits (should be defined BEFORE loading graphs.h or divisors.h).
# ifndef __GRAPH_LIMITS__
# define __GRAPH_LIMITS__
const int MAX_N = 1500;            // lower values here might speed up the program!
const int MAX_M = 100000;
const int MAX_PARTS_PER_EDGE = 10; // edges may be subdivided into at most 10 parts
# endif

#include "graphs.h"
#include "subdivisions.h"
#include "graph6.h"
#include "graph_io.h"
#include "divisors.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cctype>

using namespace std;

bool arg_a = false;
int verbosity = 0;
int arg_k = 1;

my_graph H;
bool found_something = false;

void show_divisor() {
	if (arg_a || verbosity >= 1) {
		int target = 0;
		assert(target >= 0 && target < H.n);
		reduce(H, __partial_divisor, target);
		assert(is_reduced(H, __tmp_divisor, target)); // reduce() stores the reduced divisor in __tmp_divisor.
		for (int i = 0; i < H.n; i++) {
			cout << (i ? ", " : "  Positive rank divisor: [") << __tmp_divisor[i];
		}
		cout << "]" << endl;
	}
	if (verbosity >= 2) {
		for (int target = 0; target < H.n; target++) {
			reduce(H, __partial_divisor, target);
			assert(is_reduced(H, __tmp_divisor, target)); // reduce() stores the reduced divisor in __tmp_divisor.
			cout << "    Reduced to vertex " << target << ":" << (target < 10 ? "  " : " ") << "[";
			for (int i = 0; i < H.n; i++) {
				cout << (i ? ", " : "") << __tmp_divisor[i];
			}
			cout << "]" << endl;
		}
	}
	found_something = true;
}

void solve(const my_graph& G) {
	assert(arg_k >= 1 && arg_k <= MAX_PARTS_PER_EDGE);
	assert(G.is_valid_undirected_graph());
	cout << G.graph_name << ":";
	cout.flush();
	H = (arg_k == 1 ? G : subdivide(G, arg_k));
	if (arg_a) {
		found_something = false;
		cout << endl;
		for (int deg = 1; deg <= H.n; deg++) {
			find_all_positive_rank_v0_reduced_divisors(H, deg, show_divisor);
			if (found_something) {
				break;
			}
		}
		assert(found_something);
	}
	else {
		cout << ' ' << find_gonality(H) << endl;
		show_divisor();
	}
}

void usage() {
	cerr << endl;
	cerr << "Usage: " << USAGE_STRING << endl;
	cerr << endl;
	cerr << HELPTEXT << endl;
}

int main(int argc, char* argv[]) {
	// Parse command-line arguments
	bool badargs = false;
	bool arg_g = false;
	char tmp[30];
	for (int i = 1; i < argc && !badargs; i++) {
		unsigned l = strlen(argv[i]);
		assert(l >= 1);
		if (argv[i][0] == '-') {
			for (unsigned j = 1; j < l; j++) {
				switch (argv[i][j]) {
					case 'g':
						arg_g = true;
						break;
					case 'a':
						arg_a = true;
						break;
					case 'v':
						verbosity++;
						break;
					default:
						badargs = true;
						break;
				}
			}
		}
		else if (isdigit(argv[i][0])) {
			int k;
			if (sscanf(argv[i], "%d", &k) != 1) {
				badargs = true;
				break;
			}
			sprintf(tmp, "%d", k);
			if (strcmp(argv[i], tmp)) {
				badargs = true;
				break;
			}
			if (k < 1 || k > MAX_PARTS_PER_EDGE) {
				cerr << "Error: invalid value of k (should be between 1 and " << MAX_PARTS_PER_EDGE << ")." << endl;
				cerr << "(Try changing compile-time standards.)" << endl;
				cerr << endl;
				badargs = true;
				break;
			}
			arg_k = k;
		}
		else {
			badargs = true;
		}
	}
	if (badargs) {
		cerr << "Invalid argument(s)." << endl;
		usage();
		exit(1);
	}
	
	// Read and process input
	if (arg_g) {
		string s;
		while (getline(cin, s)) {
			my_graph G = parse_graph6(s);
			G.graph_name = s;
			solve(G);
		}
	}
	else {
		read_plain_input_and_process(cin, solve);
	}
	return 0;
}

