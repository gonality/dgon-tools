// Helper functions to read and write graphs in graph6 format.
// 
// Alternatively, use the human-readable "plain" format (implemented in graph_io.h).
// To convert between the two formats, use convert_to_graph6.cpp and convert_from_graph6.cpp.
// Note: the graph6 format does not allow parallel edges; for this the plain format must be used.

#ifndef __GRAPH6_H__
#define __GRAPH6_H__

#include "graphs.h"
#include <cassert>
#include <string>
#include <vector>


// READ FUNCTIONS

std::vector<bool> readR(const std::string& s, size_t& pos, size_t num) {
	assert(pos + num <= s.size());
	std::vector<bool> ret;
	ret.reserve(num * 6);
	for (size_t i = 0; i < num; i++) {
		int cur = s[pos + i] - 63;
		assert(cur >= 0 && cur < 64);
		for (int j = 5; j >= 0; j--) {
			ret.push_back((cur & (1 << j)) != 0);
		}
	}
	pos += num;
	return ret;
}

long long readN(const std::string& s, size_t& pos) {
	assert(pos < s.size());
	if (s[pos] < 126) {
		return s[pos++] - 63;
	}
	pos++;
	assert(pos < s.size());
	if (s[pos] < 126) {
		std::vector<bool> r = readR(s, pos, 3);
		assert(r.size() == 18);
		long long ret = 0;
		for (size_t i = 0; i < r.size(); i++) {
			ret <<= 1;
			ret += r[i];
		}
		return ret;
	}
	pos++;
	assert(pos < s.size());
	std::vector<bool> r = readR(s, pos, 6);
	assert(r.size() == 36);
	long long ret = 0;
	for (size_t i = 0; i < r.size(); i++) {
		ret <<= 1;
		ret += r[i];
	}
	return ret;
}

void parse_graph6(const std::string& s, my_graph& ret) {
	size_t pos = 0;
	for (size_t i = 0; i < s.size(); i++) {
		assert(s[i] >= 63 && s[i] <= 126);
	}
	long long n = readN(s, pos);
	assert(n <= MAX_N);
	ret.init();
	ret.setN(n);
	long long m = (n * (n - 1)) / 2;
	while (m % 6) m++;
	std::vector<bool> adj_v = readR(s, pos, m / 6);
	assert(pos == s.size());
	assert((long long) adj_v.size() == m);
	size_t k = 0;
	for (int j = 0; j < n; j++) {
		for (int i = 0; i < j; i++) {
			if (adj_v[k++]) {
				ret.add_edge(i,j);
			}
		}
	}
}

my_graph parse_graph6(const std::string& s) {
	my_graph ret;
	parse_graph6(s, ret);
	return ret;
}



// WRITE FUNCTIONS

std::string writeR(const std::vector<bool>& v) {
	unsigned l = (v.size() + 5) / 6;
	assert(6 * l >= v.size() && 6 * l < v.size() + 6);
	std::string ret;
	for (unsigned i = 0; i < l; i++) {
		int cur = 0;
		for (int j = 0; j < 6; j++) {
			unsigned pos = 6 * i + j;
			if (pos < v.size() && v[pos]) {
				cur |= (1 << (5 - j));
			}
		}
		assert(cur >= 0 && cur < 64);
		ret.push_back(63 + cur);
	}
	return ret;
}

std::string writeN(long long n) {
	assert(n >= 0 && n <= 68719476735); // maximum value: 2^36 - 1 (see nauty documentation)
	char tmp[30];
	if (n <= 62) {
		tmp[0] = n + 63;
		tmp[1] = '\0';
	}
	else {
		int pos;
		std::vector<bool> bin_repr;
		bin_repr.reserve(36);
		if (n <= 258047) {
			tmp[0] = 126;
			pos = 1;
			for (long long i = 17; i >= 0; i--) {
				bin_repr.push_back((n & (1 << i)) != 0);
			}
		}
		else {
			tmp[0] = tmp[1] = 126;
			pos = 2;
			for (long long i = 35; i >= 0; i--) {
				bin_repr.push_back((n & (1 << i)) != 0);
			}
		}
		std::string R = writeR(bin_repr);
		assert(R.size() == 3 || R.size() == 6);
		strcpy(tmp + pos, R.c_str());
	}
	size_t l = strlen(tmp);
	assert(l == 1 || l == 4 || l == 8);
	return std::string(tmp);
}

std::string write_graph6(const my_graph& G) {
	assert(G.is_valid_undirected_graph()); // check validy AND populate __adj_matr[][]
	// only simple graphs can be stored in graph6 format:
	for (int i = 0; i < G.n; i++) {
		if (__adj_matr[i][i] != 0) {
			throw 1;
		}
		for (int j = 0; j < G.n; j++) {
			if (__adj_matr[i][j] != 0 && __adj_matr[i][j] != 1) {
				throw 1;
			}
		}
	}
	// graph is simple; start actual conversion
	std::string ret = writeN(G.n);
	std::vector<bool> upp_triangle;
	for (int j = 0; j < G.n; j++) {
		for (int i = 0; i < j; i++) {
			upp_triangle.push_back(__adj_matr[i][j] != 0);
		}
	}
	ret.append(writeR(upp_triangle));
	return ret;
}


#endif
