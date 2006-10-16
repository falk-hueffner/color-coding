#ifndef PROBLEM_H
#define PROBLEM_H

#include "graph.h"

struct Problem {
    Problem() : path_length(0),
		auto_preheat_trials(true), auto_trials(true), auto_colors(true),
		num_preheat_trials(0), num_colors(0) { }
    Graph g;
    std::vector<vertex_t> start_vertices;
    std::vector<bool> is_start_vertex, is_end_vertex;
    std::vector<std::vector<weight_t> > match_weights;
    std::size_t max_insertions;
    std::size_t max_deletions;
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
