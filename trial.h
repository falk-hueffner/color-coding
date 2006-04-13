#ifndef TRIAL_H
#define TRIAL_H

#include <vector>

#include "bounds.h"
#include "colored_graph.h"
#include "pathset.h"

std::size_t trials_for_prob(std::size_t path_length, std::size_t num_colors,
			    double success_prob);

bool dynprog_trial(const ColoredGraph& g,
		   const std::vector<bool>& is_start_vertex,
		   const std::vector<bool>& is_end_vertex,
		   bool find_trees,
		   std::size_t path_length,
		   std::size_t num_colors,
		   PathSet& paths,
		   const Bounds& bounds);

#endif	// TRIAL_H
