#include <set>

#include "bounds.h"
#include "debug.h"
#include "util.h"

Bounds::Bounds(const Problem& problem, Bounds::Mode mode, std::size_t n_max_lb_edges)
    : max_lb_edges(n_max_lb_edges),
      min_neighbor_weight(problem.g.num_vertices(), WEIGHT_MAX),
      min_to_goal(max_lb_edges, std::vector<weight_t>(problem.g.num_vertices(), WEIGHT_MAX)),
      min_to_anywhere(min_to_goal),
      min_anywhere_to_goal(max_lb_edges, WEIGHT_MAX),
      min_anywhere_to_anywhere(max_lb_edges, WEIGHT_MAX),
      min_weight(problem.path_length, problem.g.num_vertices()) {
    if (mode == NONE)
	return;

    std::vector<weight_t> edge_weights;
    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
	for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
	     n != problem.g.neighbors_end(v); ++n)
	    edge_weights.push_back(n->weight);
    std::sort(edge_weights.begin(), edge_weights.end());
    min_edge_weight = edge_weights.front();

    if (mode == EDGE_WEIGHT) {
	for (vertex_t v = 0; v < problem.g.num_vertices(); ++v) 
	    for (std::size_t l = 0; l < problem.path_length - 1; ++l)
		min_weight[l+1][v] = min_edge_weight * l;
	return;
    }

    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
	for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
	     n != problem.g.neighbors_end(v); ++n)
	    if (n->weight < min_neighbor_weight[v])
		min_neighbor_weight[v] = n->weight;

    double last_printed = -1;
    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v) {
	if (info.is_on() && timestamp() - last_printed > 1) {
	    fprintf(stderr, "lower-b %6d/%6zu\n", int(v), problem.g.num_vertices());
	    last_printed = timestamp();
	}
	dynprog(problem, v);
    }

    for (std::size_t l = 0; l < max_lb_edges; ++l) {
	min_anywhere_to_goal[l] = *std::min_element(min_to_goal[l].begin(), min_to_goal[l].end());
	min_anywhere_to_anywhere[l] = *std::min_element(min_to_anywhere[l].begin(),
							min_to_anywhere[l].end());
    }

    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v) {
	for (std::size_t l = 0; l < problem.path_length - 1; ++l) {
	    for (std::size_t to_anywhere = 0;
		 to_anywhere <= std::min(l, max_lb_edges); ++to_anywhere) {
		weight_t min_w = 0;
		std::size_t edges_left = l;
		if (to_anywhere) {
		    min_w += min_to_anywhere[to_anywhere - 1][v];
		    edges_left -= to_anywhere;
		}
		while (edges_left > max_lb_edges) {
		    std::size_t step = std::min(max_lb_edges, edges_left - max_lb_edges);
		    min_w += min_anywhere_to_anywhere[step - 1];
		    edges_left -= step;
		}
		if (edges_left)
		    min_w += min_anywhere_to_goal[edges_left - 1];
		min_weight[l+1][v] = std::max(min_weight[l+1][v], min_w);
	    }
	}
    }
}

void Bounds::dynprog(const Problem& problem, std::size_t s) {
    typedef std::set<vertex_t> path_t;
    typedef std::map<path_t, weight_t> path_set_t;
    typedef std::map<vertex_t, path_set_t> path_sets_t;
    path_sets_t old_path_sets, new_path_sets;

    path_t start_path; start_path.insert(s);
    path_set_t start_set; start_set[start_path] = 0;
    old_path_sets[s] = start_set;

    for (std::size_t l = 0; l < max_lb_edges; ++l) {
	new_path_sets.clear();
	for (path_sets_t::const_iterator psi = old_path_sets.begin();
	     psi != old_path_sets.end(); ++psi) {
	    vertex_t v = psi->first;
	    const path_set_t& path_set = psi->second;
	    for (path_set_t::const_iterator pi = path_set.begin(); pi != path_set.end(); ++pi) {
		const path_t& path = pi->first;
		weight_t path_weight = pi->second;
		for (Graph::neighbor_it ni = problem.g.neighbors_begin(v);
		     ni != problem.g.neighbors_end(v); ++ni) {
		    vertex_t w = ni->neighbor;
		    weight_t edge_weight = ni->weight;
		    path_t extended_path = path; extended_path.insert(w);
		    weight_t& extended_weight = new_path_sets[w][extended_path];
		    if (extended_weight < path_weight + edge_weight)
			extended_weight = path_weight + edge_weight;
		}
	    }
	}
	for (path_sets_t::const_iterator psi = new_path_sets.begin();
	     psi != new_path_sets.end(); ++psi) {
	    vertex_t v = psi->first;
	    const path_set_t& path_set = psi->second;
	    for (path_set_t::const_iterator pi = path_set.begin(); pi != path_set.end(); ++pi) {
		weight_t path_weight = pi->second;
		if (path_weight < min_to_anywhere[l][s])
		    min_to_anywhere[l][s] = path_weight;
		if (problem.is_end_vertex[v])
		    if (path_weight < min_to_goal[l][s])
			min_to_goal[l][s] = path_weight;
	    }
	}
	std::swap(old_path_sets, new_path_sets);
    }
}
