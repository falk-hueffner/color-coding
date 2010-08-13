#include <stdlib.h>

#include <vector>

#include "bounds.h"
#include "colored_graph.h"
#include "debug.h"
#include "pathset.h"
#include "problem.h"
#include "trial.h"
#include "types.h"

/* random number from 0..n-1  */
std::size_t random(std::size_t n) {
    return std::size_t(double(n) * rand() / (RAND_MAX + 1.0));
}

std::vector<vertex_t> random_path(const Graph& g0, std::size_t path_length) {
    Graph g(g0);
    for (vertex_t v = 0; v < g.num_vertices(); ++v)
	for (Graph::neighbor_it n = g.neighbors_begin(v); n != g.neighbors_end(v) ; ++n)
	    g.set_weight(v, n->neighbor, 1);
    Problem problem;
    problem.g = g;
    problem.path_length = path_length;
    Bounds bounds(problem, Bounds::NONE, 0);
    std::vector<bool> is_end_vertex(g.num_vertices(), true);
    ColoredGraph cg(g);
    PathSet paths(1, 0);
    std::size_t pass = 0;
    do {
	cg.color_randomly(path_length);
	std::vector<bool> is_start_vertex(g.num_vertices());
	is_start_vertex[random(g.num_vertices())] = true;
	if (info.is_on())
	    std::cerr << "Finding random path pass " << pass++ << std::endl;
	dynprog_trial(cg, is_start_vertex, is_end_vertex, path_length, path_length,
		      paths, bounds);
    } while (paths.size() == 0);
    assert (paths.best_weight() == path_length - 1);
    return paths.begin()->path();
}
