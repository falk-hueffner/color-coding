#ifndef FIND_PATH_H
#define FIND_PATH_H

#include "pathset.h"
#include "problem.h"

PathSet lightest_path(const Problem& problem, std::size_t num_paths, std::size_t max_common);

#endif	// FIND_PATH_H
