#ifndef BOUNDS_H
#define BOUNDS_H

#include <vector>

#include "problem.h"
#include "types.h"

class Bounds {
public:
    enum Mode { NONE, EDGE_WEIGHT, DYNPROG };

    Bounds(const Problem& problem, Mode mode, std::size_t max_lb_edges);
    weight_t h(vertex_t v, std::size_t edges_left, std::size_t deletions_left = 0) const {
	if (min_match_weights.size()) {
	    if (deletions_left >= edges_left)
		return 0;
	    else
		return min_weight[edges_left - deletions_left][v]
		     + min_match_weights[edges_left][deletions_left];
	} else {
	    return min_weight[edges_left][v];
	}
    }

private:
    void dynprog(const Problem& problem, std::size_t s);

    std::size_t max_lb_edges;
    weight_t min_edge_weight;
    std::vector<weight_t> min_neighbor_weight;
    std::vector<std::vector<weight_t> > min_to_goal;
    std::vector<std::vector<weight_t> > min_to_anywhere;
    std::vector<weight_t> min_anywhere_to_goal;
    std::vector<weight_t> min_anywhere_to_anywhere;
    std::vector<std::vector<weight_t> > min_weight;
    std::vector<std::vector<weight_t> > min_match_weights; // [left][deletions]
};

#endif	// BOUNDS_H
