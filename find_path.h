#ifndef FIND_PATH_H
#define FIND_PATH_H

#include "pathset.h"
#include "problem.h"

PathSet lightest_path(const Problem& problem,
		      std::size_t num_trials, std::size_t num_paths,
		      std::size_t max_common, std::size_t preheat_trials);

#endif	// FIND_PATH_H
