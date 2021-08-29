// Graph data structure.

#ifndef __GRAPHS_H__
#define __GRAPHS_H__

#include <cassert>
#include <cstring>
#include <vector>
#include <string>


// Default global graph limits. These can be overwritten by defining different values
// BEFORE you #include this file.

# ifndef __GRAPH_LIMITS__
# define __GRAPH_LIMITS__
const int MAX_N = 1500;            // lower values here might save some time and memory
const int MAX_M = 100000;
const int MAX_PARTS_PER_EDGE = 10; // edges may be subdivided into at most 10 parts
# endif

int __adj_matr[MAX_N][MAX_N];

// Graph data structure.
struct my_graph {
	int n;
	std::string graph_name;
	std::vector<int> neighbours[MAX_N];
	my_graph() : n(0) {}
	my_graph(int _n) : n(0) {
		setN(_n);
	}
	void setN(int _n) {
		assert(_n >= 0 && _n >= n && _n <= MAX_N);
		n = _n;
	}
	void add_edge(int a, int b) {
		assert(a >= 0 && a < n);
		assert(b >= 0 && b < n);
		assert(a != b);
		neighbours[a].push_back(b);
		neighbours[b].push_back(a);
	}
	int count_edges() const {
		int ret = 0;
		for (int i = 0; i < n; i++) {
			ret += neighbours[i].size();
		}
		assert(ret % 2 == 0);
		return ret / 2;
	}
	void init() {
		for (int i = 0; i < n; i++) {
			neighbours[i].clear();
		}
		n = 0;
	}
	bool is_valid_undirected_graph(bool simple = false) const {
		#ifdef EXTRA_CHECKS
		memset(__adj_matr, 0, sizeof __adj_matr);
		for (int i = 0; i < MAX_N; i++) {
		#else
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				__adj_matr[i][j] = 0;
			}
		}
		for (int i = 0; i < n; i++) {
		#endif
			for (int j : neighbours[i]) {
				assert(j >= 0 && j < n);
				__adj_matr[i][j]++;
			}
		}
		for (int i = 0; i < n; i++) {
			if (__adj_matr[i][i] != 0) return false;
			for (int j = 0; j < i; j++) {
				assert(__adj_matr[i][j] >= 0 && __adj_matr[j][i] >= 0);
				if (__adj_matr[i][j] != __adj_matr[j][i]) {
					return false;
				}
				if (simple && __adj_matr[i][j] > 1) {
					return false;
				}
			}
		}
		return true;
	}
};


#endif
