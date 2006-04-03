#ifndef FIND_PATH_H
#define FIND_PATH_H

#include "bounds.h"
#include "pathset.h"
#include "problem.h"

PathSet lightest_path(const Problem& problem, std::size_t num_paths,
		      std::size_t max_common, Bounds::Mode mode, std::size_t max_lb_edges);

#endif	// FIND_PATH_H
