#ifndef FIND_PATH_H
#define FIND_PATH_H

#include "bounds.h"
#include "pathset.h"
#include "problem.h"

#ifndef _GUI_
PathSet lightest_path(const Problem& problem, std::size_t num_paths,
		      std::size_t max_common, Bounds::Mode mode, std::size_t max_lb_edges);
#else
#include "gui/searchthread.h"
PathSet lightest_path(const Problem& problem, std::size_t num_paths,
		      std::size_t max_common, Bounds::Mode mode, std::size_t max_lb_edges, SearchThread *search);
#endif

#endif	// FIND_PATH_H
