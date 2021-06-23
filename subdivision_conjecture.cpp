// This program reads a bunch of graphs from standard input, and tests the subdivision and
// Brill–Noether conjectures for these graphs.
// 
// Usage:
//       ./subdivision_conjecture [-gfvv] [k] < infile.in
// 
//       Numerical argument k: number of parts into which every edge must be subdivided
//                             before comparing the gonality of the subdivision to the
//                             gonality of the original graph. Default value: k = 2.
// 
//       Input options:
//       -g  : use graph6 input instead of plain input
// 
//       Computational options:
//       -f  : fast test routine (do not compute gonality of subdivision; only try
//             to find a positive rank divisor of smaller degree) (about 20% faster)
// 
//       Output options:
//       -v  : verbose (also print gonality of non-counterexamples)
//       -vv : extra verbose (also print optimal divisor for non-counterexamples)
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
"subdivision_conjecture [-gfvv] [k] < infile.in"

#define HELPTEXT \
" Compares the gonality of every graph specified in the file \"infile.in\" to the\n\
 gonality of its k-regular subdivision.\n\
\n\
\n\
    Numerical arguments:\n\
        k    : number of parts into which every should be divided (default: 2)\n\
\n\
    Input options:\n\
       -g    : use graph6 input instead of plain input\n\
\n\
    Computational options:\n\
       -f    : fast test routine (do not compute gonality of subdivision; only try\n\
               to find a positive rank divisor of smaller degree) (about 20% faster)\n\
\n\
    Output options:\n\
       -v    : verbose (also print gonality of non-counterexamples)\n\
       -vv   : extra verbose (also print optimal divisor for non-counterexamples)\n\
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
#include <string>
#include <cassert>

using namespace std;

int __my_divisor[MAX_N];

int arg_k = 2;
bool arg_g = false;
bool arg_f = false;
int verbosity = 0;

int count_graphs = 0;
int count_probs = 0;

// Extended graph test routine (also computes the gonality of the subdivision).
void check_graph_extended(const my_graph& G) {
	// Compute constants
	const int n = G.n;
	const int m = G.count_edges();
	const int algebraic_genus = m - n + 1;
	const int Brill_Noether_bound = (algebraic_genus + 3) / 2;
	const double Brill_Noether_bound_double = (algebraic_genus + 3.0) / 2.0;
	count_graphs++;
	
	// Compute gonality of original graph
	const int gon_G = find_gonality(G);
	
	// Compute gonality of subdivided graph
	my_graph H = subdivide(G, arg_k);
	const int gon_H = find_gonality(H);
	bool is_counterexample = gon_G != gon_H || gon_G > Brill_Noether_bound || gon_H > Brill_Noether_bound;
	if (is_counterexample) {
		count_probs++;
	}
	
	// Print output if necessary
	if (is_counterexample || verbosity >= 1) {
		cout << "Graph " << count_graphs << " (\"" << G.graph_name << "\"): (original gonality, subdivided gonality, Brill–Noether bound) = (" << gon_G << ", " << gon_H << ", " << Brill_Noether_bound_double << ").";
		if (is_counterexample || verbosity >= 2) {
			cout << " Divisor: [";
			for (int i = 0; i < H.n; i++) {
				cout << (i ? ", " : "") << __partial_divisor[i]; // find_gonality(H) stores the optimal divisor in __partial_divisor.
			}
			cout << ']';
		}
		cout << endl;
		cout.flush();
	}
}

// Fast graph test routine (doesn't compute the gonality of the subdivision; only tries to find a positive
// rank effective divisor of degree gon(G) - 1).
void check_graph_fast(const my_graph& G) {
	// Compute constants
	const int n = G.n;
	const int m = G.count_edges();
	const int algebraic_genus = m - n + 1;
	const int Brill_Noether_bound = (algebraic_genus + 3) / 2;
	count_graphs++;
	
	// Compute gonality of original graph
	const int gon_G = find_gonality(G);
	bool is_BN_counterexample = (gon_G > Brill_Noether_bound);
	if (is_BN_counterexample) {
		cout << "Graph " << count_graphs << " (\"" << G.graph_name << "\") fails Brill–Noether bound! Gonality: " << gon_G << ", bound: " << Brill_Noether_bound << "." << endl;
	}
	if (verbosity >= 2) {
		// Make a backup of the divisor found by find_gonality(G).
		// Due to the verbosity level, this may be needed at a later time.
		for (int i = 0; i < G.n; i++) {
			__my_divisor[i] = __partial_divisor[i]; // find_gonality(G) stores the optimal divisor in __partial_divisor.
		}
	}
	
	// Compute gonality of subdivided graph
	my_graph H = subdivide(G, arg_k);
	bool is_subdiv_counterexample = find_positive_rank_divisor(H, gon_G - 1);
	if (is_BN_counterexample || is_subdiv_counterexample) {
		count_probs++;
	}
	
	// Print output if necessary
	if (is_subdiv_counterexample || verbosity >= 1) {
		cout << "Graph " << count_graphs << " (\"" << G.graph_name << "\")" << (is_subdiv_counterexample ? " fails subdivision conjecture!" : ": all OK.");
		if (is_subdiv_counterexample || verbosity >= 2) {
			if (!is_subdiv_counterexample) {
				// No positive rank divisor on H was found (because we only searched up to degree gon_G - 1).
				// However, the current verbosity level requests that we output a positive rank divisor on H.
				// Earlier, we stored a backup of the optimal divisor on G, which we now restore and extend to H.
				int deg = 0;
				for (int i = 0; i < H.n; i++) {
					__partial_divisor[i] = (i < G.n ? __my_divisor[i] : 0);
					assert(__partial_divisor[i] >= 0);
					deg += __partial_divisor[i];
				}
				assert(deg == gon_G);
				assert(has_positive_rank(H, __partial_divisor));
			}
			cout << " Divisor: [";
			for (int i = 0; i < H.n; i++) {
				cout << (i > 0 ? ", " : "") << __partial_divisor[i]; // find_positive_rank_divisor stores the optimal divisor in __partial_divisor.
			}
			cout << ']';
		}
		cout << endl;
		cout.flush();
	}
}

void solve(const my_graph& G) {
	assert(arg_k >= 1 && arg_k <= MAX_PARTS_PER_EDGE);
	assert(G.is_valid_undirected_graph());
	if (arg_f) {
		check_graph_fast(G);
	}
	else {
		check_graph_extended(G);
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
					case 'f':
						arg_f = true;
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
			if (k < 2 || k > MAX_PARTS_PER_EDGE) {
				cerr << "Error: invalid value of k (should be between 2 and " << MAX_PARTS_PER_EDGE << ")." << endl;
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
	
	// Print summary
	cout << endl;
	cout << "Summary: found " << count_probs << " counterexample" << (count_probs == 1 ? "." : "s.") << endl;
	return 0;
}

