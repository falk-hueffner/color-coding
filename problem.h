#ifndef PROBLEM_H
#define PROBLEM_H

#include "graph.h"

struct Problem {
    Problem() : find_trees(false), path_length(0), num_colors(0) { }
    Graph g;
    std::vector<vertex_t> start_vertices;
    std::vector<bool> is_end_vertex;
    bool find_trees;
    std::size_t path_length;
    std::size_t num_colors;
};

#endif	// PROBLEM_H
