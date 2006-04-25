#include <math.h>
#include <vector>
#include <set>

#include "bounds.h"
#include "colored_graph.h"
#include "debug.h"
#include "find_path.h"
#include "trial.h"
#include "util.h"

extern std::size_t peak_mem_usage;

PathSet lightest_path(const Problem& problem, std::size_t num_paths,
		      std::size_t max_common, Bounds::Mode mode, std::size_t max_lb_edges) {
    std::vector<weight_t> edge_weights;
    
    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
	for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
	     n != problem.g.neighbors_end(v); ++n)
	    edge_weights.push_back(n->weight);
    std::sort(edge_weights.begin(), edge_weights.end());

    PathSet paths(num_paths, max_common);
    Bounds bounds(problem, mode, max_lb_edges);
    std::size_t colors;
    std::size_t max_preheat_trials =
	(problem.auto_preheat_trials ? 10 : problem.num_preheat_trials);
    std::size_t preheat_trials = 0;
    std::size_t trials = 0;
    std::size_t punt = 0;
    std::size_t color_ceil = problem.auto_colors ? MAX_COLORS : problem.num_colors;
    std::size_t max_trials;
    do {
	ColoredGraph g = problem.g;
	bool preheating = preheat_trials < max_preheat_trials;
	if (preheating) {
	    if (preheat_trials + 1 < max_preheat_trials) {
		g.clear_edges();
		double ratio = double(preheat_trials + 1) / (max_preheat_trials);
		weight_t max_edge_weight = edge_weights[lrint(edge_weights.size() * ratio)];
		for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
		    for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
			 n != problem.g.neighbors_end(v); ++n)
			if (v < n->neighbor && n->weight <= max_edge_weight)
			    g.connect(v, n->neighbor, n->weight);
	    }
	    colors = problem.path_length;
	} else {
	    colors = color_ceil;
	}
	max_trials =
	    problem.auto_trials
	    ? trials_for_prob(problem.path_length, colors, problem.success_prob)
	    : problem.num_trials;
	g.color_randomly(colors);
	if (info.is_on())
	    fprintf(stderr, "%7s %6zd/%6zd edges=%zd colors=%zd %zdM %zd:[%10.8f,%10.8f]\n",
		    preheating ? "preheat" : "trial",
		    preheating ? preheat_trials : trials,
		    preheating ? max_preheat_trials : max_trials,
		    g.num_edges(),
		    colors, peak_mem_usage / 1024 / 1024,
		    paths.size(), paths.best_weight(), paths.worst_weight());
		std::size_t old_paths_size = paths.size();
	weight_t old_worst_weight = paths.worst_weight();
	bool ok = dynprog_trial(g, problem.is_start_vertex, problem.is_end_vertex,
				problem.path_length, colors,
				paths, bounds);
	bool bound_improved = old_paths_size != paths.size()
	    || old_worst_weight != paths.worst_weight();
	if (preheating) {
	    if (!ok) {
		if (!problem.auto_preheat_trials) {
		    std::cerr << "error: out of memory while preheating\n";
		    exit(1);
		}
		max_preheat_trials *= 2;
		preheat_trials = 0;
	    } else {
		if (problem.auto_preheat_trials) {
		    if (!bound_improved) {
			if (++punt > 1) {
			    ++preheat_trials;
			    punt = 0;
			}
		    } else {
			punt = 0;
		    }
		} else {
		    ++preheat_trials;
		}
	    }
	} else {
	    if (!ok) {
		if (!problem.auto_colors || colors == problem.path_length) {
		    std::cerr << "error: out of memory\n";
		    exit(1);
		}
		--color_ceil;
		trials = 0;
	    } else
		++trials;
	}
    } while (trials < max_trials);

    return paths;
}

#if 0
static void debug_colorset(colorset c) {
    debug << '{';
    std::size_t i = 0;
    colorset m = 1;
    bool did_print = false;
    while (c) {
	if (m & c) {
	    if (did_print)
		debug << ',';
	    debug << i;
	    did_print = true;
	}
	c &= ~m;
	m <<= 1;
	i++;
    }
    debug << '}';
}
#endif
