#ifndef PROBLEM_H
#define PROBLEM_H

#include "graph.h"

struct Problem {
    Problem() : find_trees(false),
		path_length(0),
		auto_preheat_trials(true), auto_trials(true), auto_colors(true),
		num_preheat_trials(0), num_colors(0) { }
    Graph g;
    std::vector<vertex_t> start_vertices;
    std::vector<bool> is_start_vertex, is_end_vertex;
    bool find_trees;
    std::size_t path_length;
    bool auto_preheat_trials;
    bool auto_trials;
    bool auto_colors;
    std::size_t num_preheat_trials;
    std::size_t num_trials;
    double success_prob;
    std::size_t num_colors;
};

#endif	// PROBLEM_H
