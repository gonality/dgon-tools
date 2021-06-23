// This file contains helper functions to quickly find upper bounds on the gonality of a graph.
// For this we use the Boppana–Halldórsson algorithm [1], which is a fast randomized approximation
// algorithm for the independent set problem. We use this in two ways:
// 
//      * If A is an independent set in the simple graph G = (V, E), then the divisor with 1 chip
//        on every vertex in V \ A and 0 chips on every vertex of A has positive rank.
//      
//      * Let G = (V,E) be a simple graph, and let G' = (V', E') the smallest simple graph such
//        that G is a subdivision of G'. In other words, G' is formed by contracting all degree 2
//        vertices of G, except when this would create a loop. If A is an independent set in G',
//        then the corresponding divisor on G' (as constructed above) has positive rank on G as
//        well, because its support is a strong separator.
// 
// Note that the independent set trick cannot be used on non-simple graphs (with parallel edges)!
// 
// Since the algorithm is randomized, it can be worthwhile to execute it a few times. Experimental
// results indicate good results when calling it around 7 times, but a few extra doesn't hurt
// as this is computationally much cheaper than computing the gonality.
// 
// These functions are only used in the code to test the Brill–Noether conjecture. There we always
// verify the result by checking (i) that the resulting set is indeed independent; (ii) that the
// resulting divisor does indeed have positive rank. If any of these checks fails, a runtime error
// is triggered via the assertion mechanism from <cassert>. This has never happened for us when we
// tested all simple graphs of up to 13 vertices.
// 
// References:
// 
//    [1]: Ravi Boppana and Magnús M. Halldórsson (1992), Approximating Maximum Independent Sets
//         by Excluding Subgraphs, BIT Numerical Mathematics 32(2):180-196, doi:10.1007/BF01994876
// 

#ifndef __APPROXIMATE_INDEPENDENT_SETS_H__
#define __APPROXIMATE_INDEPENDENT_SETS_H__

#define EXTRA_CHECKS

#include "graphs.h"
#include <cstdlib>
#include <utility>
#include <bitset>

typedef std::bitset<MAX_N> vertex_set;

#define is_subset(x, y) (((x) & (y)) == x)

// Check if the vertex set is independent.
bool __check_indep(const my_graph& G, const vertex_set& S) {
	for (int i = 0; i < G.n; i++) {
		if (!S.test(i)) {
			continue;
		}
		for (int j = 0; j < G.n; j++) {
			if (i != j && S.test(j) && __adj_matr[i][j] == 1) {
				return false;
			}
		}
	}
	return true;
}

// Check if the vertex set is a clique.
bool __check_cliq(const my_graph& G, const vertex_set& S) {
	for (int i = 0; i < G.n; i++) {
		if (!S.test(i)) {
			continue;
		}
		for (int j = 0; j < G.n; j++) {
			if (i != j && S.test(j) && __adj_matr[i][j] == 0) {
				return false;
			}
		}
	}
	return true;
}

// Boppana–Halldórsson algorithm.
// 
// This is a randomized approximation algorithm for the clique and independent set problems.
// At the core lies the following recursive function, which finds a large independent set and
// a large clique in the subgraph of G induced by the vertex set S. This is done as follows:
// 
//      * Choose a random vertex v0 in S;
//      
//      * Divide the remaining vertices of S into neighbours and non-neighbours of v0;
//      
//      * Recursively find a large independent set and a large clique in each of these two
//        subsets (formed by the neighbours and non-neighbours of v0);
//      
//      * Add v0 to the largest independent set among the non-neighbours, and to the largest
//        clique among the neighbours;
//      
//      * Compare the two candidates for an independent set and choose the largest.
//        Likewise, compare the two candidates for a clique and choose the largest.
//      
//      * Return the chosen independent set and the chosen clique.
// 
// This is the function that Boppana and Halldórsson in their paper [1] call "Ramsey", due to
// the role of this algorithm in Ramsey theory.
// 
std::pair<vertex_set, vertex_set> Boppana_Halldorsson_Ramsey(const my_graph& G, const vertex_set& S) {
	// Check if S is empty.
	if (S.none()) {
		return std::make_pair(vertex_set(0ul), vertex_set(0ul));
	}
	
	// Pick one random vertex v0, and divide the remaining vertices into neighbours and non-neighbours of v0.
	std::vector<int> S_vect;
	for (int i = 0; i < G.n; i++) {
		if (S.test(i)) {
			S_vect.push_back(i);
		}
	}
	assert(!S_vect.empty());
	assert(S_vect.size() == S.count());
	int v0 = S_vect[rand() % S_vect.size()];
	assert(S.test(v0));
	vertex_set neighbs, non_neighbs;
	for (auto i : S_vect) {
		if (i == v0) {
			continue;
		}
		(__adj_matr[v0][i] ? neighbs : non_neighbs).set(i);
	}
	assert((neighbs & non_neighbs) == 0);
	vertex_set tmp = S;
	tmp.reset(v0);
	assert((neighbs | non_neighbs) == tmp);
	
	// Recursively call this function for the smaller sets neighbs and non_neighbs.
	std::pair<vertex_set, vertex_set> p_neighbs = Boppana_Halldorsson_Ramsey(G, neighbs);
	std::pair<vertex_set, vertex_set> p_non_neighbs = Boppana_Halldorsson_Ramsey(G, non_neighbs);
	
	// Use the four sets returned by the recursive calls to build a candidate
	// independent set and a candidate clique.
	vertex_set A_indep = p_neighbs.first;
	vertex_set B_indep = p_non_neighbs.first;   B_indep.set(v0);
	vertex_set A_cliq  = p_neighbs.second;      A_cliq.set(v0);
	vertex_set B_cliq  = p_non_neighbs.second;
	
	// Sanity checks, just to be sure.
	// First check that each of the returned sets is a subset of S.
	assert(is_subset(A_indep, S));
	assert(is_subset(B_indep, S));
	assert(is_subset(A_cliq, S));
	assert(is_subset(B_cliq, S));
	// Then check that each of the candidates is indeed an independent set / clique.
	#ifdef EXTRA_CHECKS
	assert(__check_indep(G, A_indep));
	assert(__check_indep(G, B_indep));
	assert(__check_cliq(G, A_cliq));
	assert(__check_cliq(G, B_cliq));
	#endif
	
	// Out of these 2x2 candidates, return the best pair.
	vertex_set best_indep = (A_indep.count() > B_indep.count() ? A_indep : B_indep);
	vertex_set best_cliq = (A_cliq.count() > B_cliq.count() ? A_cliq : B_cliq);
	return std::make_pair(best_indep, best_cliq);
}


// Main Boppana–Halldórsson algorithm, called "Clique Removal" in their paper [1].
// 
// This algorithm repeatedly calls the preceding "Ramsey" function and removes the clique
// that is returned, until the graph is exhausted. It returns the largest of the independent
// sets that were found.
// 
// For an analysis of the approximation rate of this algorithm, see the paper [1].
// 
vertex_set approximate_maximum_independent_set(const my_graph& G) {
	assert(G.is_valid_undirected_graph()); // check validy AND populate __adj_matr[][]
	assert(G.n <= MAX_N);
	vertex_set S, best_indep;
	for (int i = 0; i < G.n; i++) {
		S.set(i);
	}
	while (S.any()) {
		std::pair<vertex_set, vertex_set> cur = Boppana_Halldorsson_Ramsey(G, S);
		assert(is_subset(cur.first, S));
		assert(is_subset(cur.second, S));
		S ^= cur.second; // remove clique
		if (cur.first.count() > best_indep.count()) {
			best_indep = cur.first;
		}
	}
	assert(G.n == 0 || best_indep.any());
	return best_indep;
}

#endif
