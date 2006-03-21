#ifndef FIND_PATH_H
#define FIND_PATH_H

#include <set>

#include "graph.h"
#include "pathset.h"
#include "ptree.h"
#include "types.h"

PathSet lightest_path(const Graph& g_in,
		      const std::vector<vertex>& start_vertices,
		      const std::vector<bool>& is_end_vertex,
		      bool find_trees,
		      std::size_t path_length, std::size_t num_colors,
		      std::size_t num_trials, std::size_t num_paths,
		      std::size_t max_common, std::size_t preheat_trials);

#endif	// FIND_PATH_H
