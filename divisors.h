// This header defines all divisor-related functions, and as such constitutes the core of our program.
// 
// This file defines the following functions:
// 
//      * int burn(const my_graph& G, const int* divisor, const int start)
//        Dhar's burning algorithm.
//      
//      * bool is_reduced(const my_graph& G, const int* divisor, const int target = -1)
//        Test whether a divisor is reduced with respect to a given vertex or with respect to any vertex.
//	
//	* void reduce(const my_graph& G, const int* divisor, const int target, int* script = NULL)
//        Reduce the given divisor to the given target vertex.
//	
//	* bool has_positive_rank(const my_graph& G, const int* divisor, bool check_graph_validity = true)
//        Test whether the given divisor has positive rank.
//	
//	* bool find_positive_rank_divisor(const my_graph& G, const int remaining_chips, const int finished_vertices = 0)
//        Brute force search for a positive rank effective divisor of prescribed degree. Somewhat optimized for performance.
//	
//	* void find_all_positive_rank_v0_reduced_divisors(const my_graph& G, const int remaining_chips, void (*const fn)(), const int finished_vertices = 0)
//        Brute force search for ALL positive rank v0-reduced divisors of prescribed degree. Somewhat optimized for performance.
//	
//      * int find_gonality(const my_graph& G)
//        Determine the (divisorial) gonality of G by brute force search.
// 

#ifndef __DIVISORS_H__
#define __DIVISORS_H__


#include <cassert>
#include <queue>
#include "graphs.h"



// Global variables.
// Do NOT use these to store valuable data, as their contents will be overwritten by the functions from this file.
bool __pushed_to_queue[MAX_N];
int __burnt_edges[MAX_N];
int __firing_set[MAX_N];
int __partial_divisor[MAX_N];
int __tmp_divisor[MAX_N];
bool __can_reach[MAX_N];



// Dhar's burning algorithm.
// 
// Input values:
//     * the graph is given as the first input (my_graph data structure; passed by const reference);
//     * the divisor is given as the second input (C array; passed as const pointer);
//     * the starting vertex is given as the third input.
// 
// Output values:
//     * the firing set is stored in the global array __firing_set;
//     * the size of the firing set is returned as an integer.
// 
// Changes global variables __pushed_to_queue, __burnt_edges, __firing_set.
int burn(const my_graph& G, const int* divisor, const int start) {
	assert(start >= 0 && start < G.n);
	for (int i = 0; i < G.n; i++) {
		__pushed_to_queue[i] = false;
		__burnt_edges[i] = 0;
		assert(i == start || divisor[i] >= 0);
	}
	std::queue<int> q;
	q.push(start);
	__pushed_to_queue[start] = true;
	while (!q.empty()) {
		int i = q.front();
		q.pop();
		for (auto j : G.neighbours[i]) {
			__burnt_edges[j]++;
			if (__burnt_edges[j] > divisor[j] && !__pushed_to_queue[j]) {
				q.push(j);
				__pushed_to_queue[j] = true;
			}
		}
	}
	int ret = 0;
	for (int i = 0; i < G.n; i++) {
		if (!__pushed_to_queue[i]) {
			__firing_set[ret] = i;
			ret++;
		}
	}
	return ret;
}



// Determine whether a given divisor is reduced with respect to a given vertex (use third argument)
// or with respect to any vertex (omit third argument).
// 
// Input values:
//     * the graph is given as the first input (my_graph data structure; passed by const reference);
//     * the divisor is given as the second input (C array; passed as const pointer);
//     * optionally, the target vertex can be given as the third input.
// 
// Output values:
//     * a boolean indicating whether or not the given divisor is reduced.
// 
// Changes global variables __pushed_to_queue, __burnt_edges, __firing_set.
bool is_reduced(const my_graph& G, const int* divisor, const int target = -1) {
	assert(target >= -1 && target < G.n);
	if (target == -1) {
		for (int i = 0; i < G.n; i++) {
			if (burn(G, divisor, i) == 0) {
				return true;
			}
		}
		return false;
	}
	else {
		return burn(G, divisor, target) == 0;
	}
}



// Reduce a given divisor to a given target vertex.
// 
// Input values:
//     * the graph is given as the first input (my_graph data structure; passed by const reference);
//     * the divisor is given as the second input (C array; passed as const pointer);
//     * the target vertex is given as the third input;
//     * the fourth argument is actually used for output; see below.
// 
// Output values:
//     * nothing is returned;
//     * the reduced divisor is stored in the global array __tmp_divisor;
//     * optionally, the "script" (i.e. the vector indicating how often every vertex was fired) is
//       stored in the array provided as the fourth argument.
// 
// Changes global variables __pushed_to_queue, __burnt_edges, __firing_set, __tmp_divisor.
void reduce(const my_graph& G, const int* divisor, const int target, int* script = NULL) {
	assert(target >= 0 && target < G.n);
	for (int i = 0; i < G.n; i++) {
		if (script != NULL) {
			script[i] = 0;
		}
		__tmp_divisor[i] = divisor[i];
	}
	while (true) {
		int firing_set_size = burn(G, __tmp_divisor, target);
		if (firing_set_size == 0) {
			break;
		}
		for (int i = 0; i < firing_set_size; i++) {
			int v = __firing_set[i];
			if (script != NULL) {
				script[v]++;
			}
			for (auto w : G.neighbours[v]) {
				__tmp_divisor[v]--;
				__tmp_divisor[w]++;
			}
		}
	}
	if (script != NULL) {
		assert(script[target] == 0);
	}
}



// Test whether a given divisor has positive rank.
// 
// Input values:
//     * the graph is given as the first input (my_graph data structure; passed by const reference);
//     * the divisor is given as the second input (C array; passed as const pointer);
//     * the option third argument enables or disables a sanity check of the input graph.
//       Set this option to false if you're doing a brute force search (e.g. find_positive_rank_divisor),
//       or you'll waste a lot of time!
// 
// Output values:
//     * the return value is a boolean indicating whether or not the divisor has positive rank.
// 
// Changes global variables __pushed_to_queue, __burnt_edges, __firing_set, __tmp_divisor, __can_reach.
bool has_positive_rank(const my_graph& G, const int* divisor, bool check_graph_validity = true) {
	if (check_graph_validity) {
		assert(G.is_valid_undirected_graph());
	}
	for (int i = 0; i < G.n; i++) {
		assert(divisor[i] >= 0);
		__tmp_divisor[i] = divisor[i];
		__can_reach[i] = (divisor[i] > 0);
	}
	for (int u = 0; u < G.n; u++) {
		while (!__can_reach[u]) {
			int firing_set_size = burn(G, __tmp_divisor, u);
			if (firing_set_size == 0) {
				return false;
			}
			for (int j = 0; j < firing_set_size; j++) {
				int v = __firing_set[j];
				for (auto w : G.neighbours[v]) {
					__tmp_divisor[v]--;
					__tmp_divisor[w]++;
				}
			}
			// record intermediate steps to save time
			for (int v = 0; v < G.n; v++) {
				if (__tmp_divisor[v] > 0) {
					__can_reach[v] = true;
				}
			}
		}
	}
	return true;
}



// Brute force search for a positive rank effective divisor of prescribed degree. Somewhat optimized for performance.
// 
// This function returns immediately after such a divisor is found; it does not proceed to find all such examples.
// To find all positive rank effective divisors, use the function find_all_positive_rank_v0_reduced_divisors() below.
// 
// Input values:
//     * the graph is given as the first input (my_graph data structure; passed by const reference);
//     * the requested degree is given as the second input;
//     * the third input is used for the purpose of recursion and should be omitted when calling this function.
// 
// Output values:
//     * the return value is a boolean indicating whether or not a positive rank divisor was found;
//     * in case of success, the found divisor is stored in the global variable __partial_divisor.
// 
// Changes global variables __pushed_to_queue, __burnt_edges, __firing_set, __partial_divisor, __tmp_divisor, __can_reach.
bool find_positive_rank_divisor(const my_graph& G, const int remaining_chips, const int finished_vertices = 0) {
	assert(remaining_chips >= 0);
	assert(finished_vertices >= 0 && finished_vertices <= G.n);
	if (finished_vertices == 0) {
		// Sanity check. Only carried out once at the very beginning, when finished_vertices == 0.
		// (Other initializations should also go here.)
		assert(G.is_valid_undirected_graph());
	}
	if (finished_vertices >= G.n) {
		// Found a divisor defined on all of G. Don't recurse any further.
		// Check whether this divisor has rank 1, but only if:
		//    * it has the right degree (i.e. all chips have been distributed);
		//    * there is at least one chip on v0;
		//    * it is already v0-reduced (to save time).
		// 
		// Note: logical and (&&) statements in C++ are short-circuiting, so the tests are carried out
		// from left to right and aborted as soon as any one of them returns false. This is especially
		// important because calls to the function has_positive_rank() dictate the total runtime.
		return remaining_chips == 0 && __partial_divisor[0] > 0 && burn(G, __partial_divisor, 0) == 0 && has_positive_rank(G, __partial_divisor, false);
	}
	
	// Recursively construct all possible effective divisors of the requested degree.
	// 
	// We start with as many chips as possible on the current vertex, and test all possible distributions
	// of the remaining chips over the remaining vertices before removing another chip from this vertex.
	// The advantage of this approach is that we will have dominated all effective divisors of degree k
	// before bringing the (k + 1)-th chip into play (i.e. putting it on another vertex than v0).
	// 
	// In particular, if you just want to know whether a positive rank divisor of degree d exists (i.e. if
	// you only want to know whether dgon(G) <= d), then instead of first calling this function for all
	// smaller degrees, it is just as fast to simply call find_positive_rank_divisor(G, d).
	// 
	// This function only looks for positive rank v0-reduced divisors, so we only need to consider
	// configurations with at least 1 chip on v0.
	const int stop = (finished_vertices == 0 ? 1 : 0);
	for (int i = remaining_chips; i >= stop; i--) {
		__partial_divisor[finished_vertices] = i;
		if (find_positive_rank_divisor(G, remaining_chips - i, finished_vertices + 1)) {
			return true;
		}
	}
	__partial_divisor[finished_vertices] = -1;
	return false;
}



// Brute force search for ALL positive rank v0-reduced divisors of prescribed degree. Somewhat optimized for performance.
// 
// This function will not stop until all possible chip configurations have been tried. In particular, it can be MUCH
// slower than the function find_positive_rank_divisor() listed above. (Of course, if no such divisors exist, then both
// functions are equally fast.)
// 
// When a positive rank v0-reduced divisor is found, the function fn (provided as the third argument) will be called.
// This should be a function of type "void fn(void)"; that is, it should not take or return any arguments.
// The function fn can read off the present divisor from the global variable __partial_divisor, but it must not modify it!
// It may modify the other global variables; this won't affect the execution of the algorithm.
// 
// Input values:
//     * the graph is given as the first input (my_graph data structure; passed by const reference);
//     * the requested degree is given as the second input;
//     * the third input is a pointer to a function which will be called when a positive rank v0-rediced divisor is found;
//     * the fourth input is used for the purpose of recursion and should be omitted when calling this function.
// 
// Output values:
//     * nothing is returned, and no positive rank divisor is stored in the global variables.
// 
// Changes global variables __pushed_to_queue, __burnt_edges, __firing_set, __partial_divisor, __tmp_divisor, __can_reach.
void find_all_positive_rank_v0_reduced_divisors(const my_graph& G, const int remaining_chips, void (*const fn)(), const int finished_vertices = 0) {
	assert(remaining_chips >= 0);
	assert(finished_vertices >= 0 && finished_vertices <= G.n);
	if (finished_vertices == 0) {
		// Sanity check. Only carried out once at the very beginning, when finished_vertices == 0.
		// (Other initializations should also go here.)
		assert(G.is_valid_undirected_graph());
	}
	if (finished_vertices >= G.n) {
		// Found a divisor defined on all of G. Don't recurse any further.
		// Check whether this divisor has rank 1, but only if:
		//    * it has the right degree (i.e. all chips have been distributed);
		//    * there is at least one chip on v0;
		//    * it is already v0-reduced (to save time).
		// 
		// Note: logical and (&&) statements in C++ are short-circuiting, so the tests are carried out
		// from left to right and aborted as soon as any one of them returns false. This is especially
		// important because calls to the function has_positive_rank() dictate the total runtime.
		if (remaining_chips == 0 && __partial_divisor[0] > 0 && burn(G, __partial_divisor, 0) == 0 && has_positive_rank(G, __partial_divisor, false)) {
			fn();
		}
		return;
	}
	
	// Recursively construct all possible effective divisors of the requested degree.
	// 
	// Contrary to the preceding function find_positive_rank_divisor(), this function has no real
	// reason to prefer any order of generating the divisors, since we have to test them all anyway.
	// For compatibility and ease of debugging, we use the same ordering of the divisors.
	// 
	// This function only looks for positive rank v0-reduced divisors, so we only need to consider
	// configurations with at least 1 chip on v0.
	const int stop = (finished_vertices == 0 ? 1 : 0);
	for (int i = remaining_chips; i >= stop; i--) {
		__partial_divisor[finished_vertices] = i;
		find_all_positive_rank_v0_reduced_divisors(G, remaining_chips - i, fn, finished_vertices + 1);
	}
	__partial_divisor[finished_vertices] = -1;
}



// Determine the (divisorial) gonality by brute force search.
// 
// Input values:
//     * the graph is given as the first and only input (my_graph data structure; passed by const reference).
// 
// Output values:
//     * the gonality of the graph is returned;
//     * a positive rank effective divisor of minimal degree is stored in the global variable __partial_divisor.
// 
// Changes global variables __pushed_to_queue, __burnt_edges, __firing_set, __partial_divisor, __tmp_divisor, __can_reach.
int find_gonality(const my_graph& G) {
	assert(G.is_valid_undirected_graph());
	for (int deg = 1; true; deg++) {
		if (find_positive_rank_divisor(G, deg)) {
			return deg;
		}
		assert(deg <= G.n);
	}
}


#endif
