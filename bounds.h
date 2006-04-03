#ifndef BOUNDS_H
#define BOUNDS_H

#include <vector>

#include "problem.h"
#include "types.h"

class Bounds {
public:
    Bounds(const Problem& problem);
    weight_t h(vertex_t v, std::size_t edges_left) const {
	return min_weight[edges_left][v];
    }

private:
    void dynprog(const Problem& problem, std::size_t s);

    static const std::size_t max_lb_edges = 2;
    weight_t min_edge_weight;
    std::vector<weight_t> min_neighbor_weight;
    std::vector<std::vector<weight_t> > min_to_goal;
    std::vector<std::vector<weight_t> > min_to_anywhere;
    std::vector<weight_t> min_anywhere_to_goal;
    std::vector<weight_t> min_anywhere_to_anywhere;
    std::vector<std::vector<weight_t> > min_weight;
};

#endif	// BOUNDS_H
