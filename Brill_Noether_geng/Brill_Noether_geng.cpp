// This program combines nauty's graph generator "geng" with the dgon-tools codebase.
// Given an integer n, it will generate all simple graphs on n vertices and test whether their gonality
// satisfies the Brill–Noether bound.
// 
// Technically, not all graphs on n vertices are generated, but only the graphs G with the following properties:
//     * G is connected;
//     * G has minimum degree 2 (in other words, no leaves);
//     * the number of edges is such that the Brill–Noether bound is at most n - 3.
// 
// The third assumption can be made because every independent set S yields a positive rank divisor of
// degree n - |S|. Apart from the complete graph, all graphs have an independent set of size at least 2,
// and therefore have gonality at most n - 2.
// 
// Before embarking on a brute force search for the gonality of a graph G, we do some quick tests to
// see if we can exclude this graph from our search. These tests include:
//     * test if G has a leaf (this is a bit superfluous now, as we only generate graphs of minimum degree 2
//       to begin with);
//     * use a (randomized) approximation algorithm to find a large independent set S, and see if the
//       Brill–Noether bound can be met by a divisor of degree n - |S|.
// 
// TODO: test if the graph has a bridge.
// 
// For a summary of the command-line options, run the following command:
//        ./Brill_Noether_geng -h
// 
// In time, we may want to take advantage of some of the other options supported by geng, as only minimal
// effort is required to enable these. Please refer to the documentation of nauty and geng for a list
// of all supported options.


// Graph limits (should be defined BEFORE loading graphs.h or divisors.h).
# ifndef __GRAPH_LIMITS__
# define __GRAPH_LIMITS__
const int MAX_N = 40;              // lower values here might speed up the program, but don't go too low or you'll hit an assertion!
const int MAX_M = 100000;
const int MAX_PARTS_PER_EDGE = 1;  // edges may be subdivided into at most 10 parts
# endif

#include "gtools.h"   // from nauty
#include "graphs.h"   // from dgon-tools codebase
#include "graph6.h"   // from dgon-tools codebase
#include "divisors.h" // from dgon-tools codebase
#include "approximate_independent_sets.h" // from dgon-tools codebase
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <string>
#include <cstdio>
#include <cctype>
#include <csignal>

const int MIN_N = 3;
const int MAX_MOD = 1234567; // geng.c does not specify a maximum, but requires that (PRUNEMULT * mod) / PRUNEMULT == mod (without overflow), where PRUNEMULT = 50.

#define USAGE \
"Brill_Noether_geng [-Cmqvv] n [res/mod]"

#define HELPTEXT \
" Test the Brill–Noether conjecture for all graphs of a specified number of vertices.\n\
\n\
      n    : the number of vertices\n\
   res/mod : only generate subset res out of subsets 0..mod-1\n\
\n\
     -C    : only test biconnected graphs\n\
     -m    : save memory at the expense of time\n\
     -v    : verbose\n\
     -vv   : extra verbose (outputs the conclusion for every graph)\n\
                           (WARNING: this produces a lot of output!)\n\
     -q    : suppress auxiliary output from geng (except from -v)\n\
\n\
  See program text for much more information.\n"

using namespace std;

// Signal handler; see [1], [2], [3], [4], [5].
//   [1]: https://stackoverflow.com/questions/1641182/how-can-i-catch-a-ctrl-c-event
//   [2]: http://lazarenko.me/signal-handler/
//   [3]: https://en.cppreference.com/w/cpp/utility/program/signal
//   [4]: https://en.cppreference.com/w/cpp/utility/program/sig_atomic_t
//   [5]: https://stackoverflow.com/questions/51920435/how-to-call-sigaction-from-c#51920564
static volatile sig_atomic_t got_signal = 0;

static void my_handler(int signo) {
	got_signal = (signo ? signo : -1);
}


// Brill–Noether code starts here
const int INDEPENDENT_SET_NUM_TRIES = 15; // experimental results indicate that 7 is the optimal number of tries, but a few extra doesn't hurt (these are pretty cheap, computationally)

long long tel = 0;
long long probs = 0;
bool badargs = false;
bool arg_C = false;
bool arg_m = false;
bool arg_v = false;
bool arg_q = false;
bool arg_h = false;
int verbosity = 0;

void check_graph(const string& g6_graph) {
	tel++;
	const my_graph G = parse_graph6(g6_graph);
	assert(G.is_valid_undirected_graph());
	const int n = G.n;
	for (int i = 0; i < n; i++) {
		if (G.neighbours[i].size() <= 1) {
			if (verbosity >= 2) { // running in very verbose mode
				cout << "Graph " << tel << " (\"" << g6_graph << "\") has a vertex of degree 1. Skipping." << endl;
			}
			return;
		}
	}
	const long long m = G.count_edges();
	const long long algebraic_genus = m - n + 1;
	const long long Brill_Noether_bound = (algebraic_genus + 3) / 2;
	if (Brill_Noether_bound >= n - 2) {
		if (verbosity >= 2) { // running in very verbose mode
			cout << "Graph " << tel << " (\"" << g6_graph << "\") trivially meets the Brill–Noether bound (BN bound = " << Brill_Noether_bound << ", N - 2 = " << n - 2 << "). Skipping." << endl;
		}
		return;
	}
	// If we can find a sufficiently large independent set, the gonality will be small.
	// We use a randomized approximation algorithm, so we might not find the best independent set,
	// but that doesn't matter. We run the approximation algorithm a few times and then move on.
	for (int i = 0; i < INDEPENDENT_SET_NUM_TRIES; i++) {
		vertex_set indep = approximate_maximum_independent_set(G);
		assert(__check_indep(G, indep));
		int deg = 0;
		for (int i = 0; i < G.n; i++) {
			__partial_divisor[i] = (indep.test(i) ? 0 : 1);
			deg += __partial_divisor[i];
		}
		assert(deg + (int) indep.count() == G.n);
		assert(has_positive_rank(G, __partial_divisor));
		if (deg <= Brill_Noether_bound) {
			if (verbosity >= 2) { // running in very verbose mode
				cout << "Graph " << tel << " (\"" << g6_graph << "\") has a sufficiently large independent set. Skipping." << endl;
			}
			return;
		}
	}
	const int gon_g = find_gonality(G);
	if (gon_g > Brill_Noether_bound) {
		cout << "Graph " << tel << " (\"" << g6_graph << "\") fails Brill–Noether bound! Gonality: " << gon_g << ", bound: " << Brill_Noether_bound << "." << endl;
		probs++;
	}
	else {
		if (verbosity >= 2) { // running in very verbose mode
			cout << "Graph " << tel << " (\"" << g6_graph << "\"): OK." << endl;
		}
	}
}

extern "C" int GENG_MAIN(int argc, char *argv[]);

extern "C" void OUTPROC(FILE *outfile, graph *g, int n) {
	/* This will be called for each graph. */
	char* g6_graph = ntog6(g, 1, n);
	// This should be the right call, based on:
	//      geng.c:571      void writeg6x(FILE *f, graph *g, int n) {
	//                            writeg6(f,g,1,n); // see note [1] below
	//                      }
	//      gtools.c:2100   void writeg6(FILE *f, graph *g, int m, int n) {
	//                            writeline(f,ntog6(g,m,n));
	//                      }
	//      gtools.c:1608   char* ntog6(graph *g, int m, int n) {
	//                            /* convert nauty graph to graph6 string, including \n and \0 */
	//                            //... 
	//                      }
	// [1]: The fixed constant m = 1 is (presumably) due to the fact that geng.c requires MAX_N <= WORDSIZE,
	//      whereas other parts of the nauty program do not require this.
	//      In genreal, one takes m = ceil(n / WORDSIZE).
	// [2]: The function ntog6() allocates memory to store the returned string, but this memory
	//      will be reused on the next call. So we must not free it!
	string g6_string(g6_graph);
	assert(!g6_string.empty() && g6_string[g6_string.size() - 1] == '\n');
	g6_string.resize(g6_string.size() - 1);
	check_graph(g6_string);
	//cout << "I see a graph: \"" << g6_string << "\"." << endl;
	if (got_signal) {
		fprintf(stderr, "\n\nReceived %s; aborting...\n", (got_signal == SIGINT ? "SIGINT" : (got_signal == SIGTERM ? "SIGTERM" : "unknown signal")));
		cout << endl;
		cout << "Summary: tested " << tel << " graphs; found " << probs << " problems." << endl;
		exit(1);
	}
}

int main(const int argc, const char *argv[]) {
	// Setup signal handler
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	if (sigaction(SIGINT, &sigIntHandler, NULL) == -1) {
		perror("sigaction");
		fprintf(stderr, ">E Error: failed to set up signal handler.\n");
		exit(1);
	}
	if (sigaction(SIGTERM, &sigIntHandler, NULL) == -1) {
		perror("sigaction");
		fprintf(stderr, ">E Error: failed to set up signal handler.\n");
		exit(1);
	}
	
	// Parse args
	char tmp[50];
	int n = -1;
	int res = -1;
	int mod = -1;
	int arg_mode = 0;
	for (int i = 1; !badargs && i < argc; i++) {
		if (arg_mode == 0 && argv[i][0] == '-') {
			// argument is a switch
			unsigned l = strlen(argv[i]);
			for (unsigned j = 1; j < l; j++) {
				switch (argv[i][j]) {
					case 'C':
						arg_C = true;
						break;
					case 'h':
						arg_h = true;
						break;
					case 'm':
						arg_m = true;
						break;
					case 'q':
						arg_q = true;
						break;
					case 'v':
						arg_v = true;
						verbosity++;
						break;
					default:
						badargs = true;
				}
			}
		}
		else if (isdigit(argv[i][0])) {
			// argument is "n" or "res/mod"
			if (arg_mode == 0) {
				arg_mode++;
			}
			if (arg_mode == 1) {
				// expecting to see "n"
				if (sscanf(argv[i], "%d", &n) != 1) {
					badargs = true;
				}
				sprintf(tmp, "%d", n);
				if (strcmp(tmp, argv[i]) || n < 0 || n > MAX_N) {
					badargs = true;
				}
				if (n >= 0 && n < MIN_N) {
					fprintf(stderr, ">E Error: n must be at least 3.\n");
					badargs = true;
				}
				arg_mode++;
			}
			else if (arg_mode == 2) {
				// expecting to see "res/mod"
				if (sscanf(argv[i], "%d/%d", &res, &mod) != 2) {
					badargs = true;
				}
				sprintf(tmp, "%d/%d", res, mod);
				if (strcmp(tmp, argv[i])) {
					badargs = true;
				}
				else if (mod < 1 || mod > MAX_MOD) {
					fprintf(stderr, ">E Error: mod must be in the range [1,%d].\n", MAX_MOD);
					badargs = true;
				}
				else if (res < 0 || res >= mod) {
					fprintf(stderr, ">E Error: res must be in the range [0,mod).\n");
					badargs = true;
				}
				arg_mode++;
			}
			else {
				// not expecting any additional args
				badargs = true;
			}
		}
		else {
			badargs = true;
		}
	}
	if (arg_h || badargs) {
		fprintf(stderr, ">E Usage: %s\n", USAGE);
		PUTHELPTEXT;
		exit(badargs ? 1 : 0);
	}
	if (MAX_N > WORDSIZE) {
		fprintf(stderr, ">E Error: MAX_N too large; should be at most WORDSIZE.\n");
		fprintf(stderr, "   Please recompile the program.\n");
		exit(1);
	}
	
	// Prepare args to pass to geng
	assert((arg_mode == 3) == (mod >= 0 && mod <= MAX_MOD && res >= 0 && res < mod));
	const int ALL_ARGC = 5;
	int all_argc = ALL_ARGC;
	char* all_argv[ALL_ARGC + 1];
	all_argv[all_argc] = NULL;
	for (int i = 0; i < all_argc; i++) {
		all_argv[i] = new char[max((size_t) 100, strlen(argv[0]) + 3)];
		if (all_argv[i] == NULL) {
			fprintf(stderr, ">E Error: failed to allocate memory in function main.\n");
			exit(1);
		}
	}
	sprintf(all_argv[0], "geng");
	sprintf(all_argv[1], "-%s%s%s%sd2", (arg_C ? "C" : "c"), (arg_m ? "m" : ""), (arg_v ? "v" : ""), (arg_q ? "q" : ""));
	sprintf(all_argv[2], "%d", n);
	sprintf(all_argv[3], "%d:%d", n, max(n, 3 * n - 9)); // 3 * n - 9 edges results in the Brill–Noether bound n - 2.5 (every simple non-complete graph has gonality at most n - 2)
	if (arg_mode == 3) { // equivalently: if "res/mod" was set
		sprintf(tmp, "x%dX1000", 200 * mod);
		strcat(all_argv[1], tmp);
		sprintf(all_argv[4], "%d/%d", res, mod);
	}
	else {
		all_argc--;
		all_argv[4] = NULL;
	}
	
	// Call geng
	if (!arg_q) {
		fprintf(stderr, ">A Calling");
		for (int i = 0; i < all_argc; i++) {
			fprintf(stderr, " %s", all_argv[i]);
		}
		fprintf(stderr, "\n");
	}
	GENG_MAIN(all_argc, all_argv);
	
	// Print summary
	cout << endl;
	cout << "Summary: tested " << tel << " graphs; found " << probs << " problems." << endl;
	return 0;
}
