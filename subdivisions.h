// Helper function to create the k-th regular subdivision of a given graph.

#ifndef __SUBDIVISIONS_H__
#define __SUBDIVISIONS_H__

#include "graphs.h"

my_graph subdivide(const my_graph& G, int parts_per_edge) {
	assert(parts_per_edge >= 2 && parts_per_edge <= MAX_PARTS_PER_EDGE);
	assert(G.is_valid_undirected_graph());
	int m = G.count_edges();
	assert(G.n + m * (parts_per_edge - 1) <= MAX_N);
	int cur_node = G.n;
	my_graph H(G.n + m * (parts_per_edge - 1));
	int node_nums[MAX_PARTS_PER_EDGE + 1];
	for (int i = 0; i < G.n; i++) {
		for (int j : G.neighbours[i]) {
			if (i < j) {
				node_nums[0] = i;
				node_nums[parts_per_edge] = j;
				for (int k = 1; k < parts_per_edge; k++) {
					node_nums[k] = cur_node;
					cur_node++;
				}
				for (int k = 0; k < parts_per_edge; k++) {
					H.add_edge(node_nums[k], node_nums[k + 1]);
				}
			}
		}
	}
	assert(cur_node == H.n);
	return H;
}

#endif
