#ifndef QPATH_TRIAL_H
#define QPATH_TRIAL_H

#include <vector>

#include "bounds.h"
#include "colored_graph.h"
#include "pathset.h"
#include "types.h"

bool qpath_trial(const ColoredGraph& g,
		 std::vector<std::vector<weight_t> > match_weights, // match_weights[l][v]
		 std::size_t max_insertions,
		 std::size_t max_deletions,
		 weight_t insertion_cost,
		 weight_t deletion_cost,
		 PathSet& paths,
		 const Bounds& bounds);

#endif  // QPATH_TRIAL_H
