#include <vector>
#include <set>

#include "bounds.h"
#include "colored_graph.h"
#include "debug.h"
#include "find_path.h"
#include "trial.h"
#include "util.h"

extern std::size_t peak_mem_usage;

PathSet lightest_path(const Problem& problem,
		      std::size_t num_trials, std::size_t num_paths,
		      std::size_t max_common, std::size_t preheat_trials_ignored) {
    std::vector<weight_t> edge_weights;
    
    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
	for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
	     n != problem.g.neighbors_end(v); ++n)
	    edge_weights.push_back(n->weight);
    std::sort(edge_weights.begin(), edge_weights.end());

    PathSet paths(num_paths, max_common);
    Bounds bounds(problem);
    double last_printed = -1;

#if 1
    std::size_t colors = problem.path_length;
    std::size_t max_preheat_trials = 2;
    std::size_t preheat_trials = 0;
    std::size_t max_trials = trials_for_prob(problem.path_length, colors, 99.9);
    std::size_t trials = 0;
    std::size_t punt = 0;
    bool preheating = true;
    bool hit_color_ceiling = false;
    ColoredGraph g = problem.g;
    while (true) {
	if (preheating) {
	    g.clear_edges();
	    weight_t max_edge_weight =
		edge_weights[std::size_t((preheat_trials+1) * (double(edge_weights.size()) / max_preheat_trials))];
	    
	    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
		for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
		     n != problem.g.neighbors_end(v); ++n)
		    if (n->weight <= max_edge_weight)
			g.connect(v, n->neighbor, n->weight);
	}
	g.color_randomly(colors);
#if 1
	if (info.is_on())
	fprintf(stderr, "%7s %6zd/%6zd colors=%zd %zdM %zd:[%10.8f,%10.8f]\n",
		preheating ? "preheat" : "trial",
		preheating ? preheat_trials : trials,
		preheating ? max_preheat_trials : max_trials,
		colors, peak_mem_usage / 1024 / 1024,
		paths.size(), paths.best_weight(), paths.worst_weight()
	    );
#endif

	std::size_t old_paths_size = paths.size();
	weight_t old_worst_weight = paths.worst_weight();
	bool ok = dynprog_trial(g, problem.start_vertices, problem.is_end_vertex,
				problem.find_trees, problem.path_length, paths,
				bounds);
	if (ok) {
	    if (preheating) {
		if (old_paths_size == paths.size() && old_worst_weight == paths.worst_weight()) {
		    if (++punt > 10) {
			++preheat_trials;
			punt = 0;
		    }
		} else {
		    punt = 0;
		}
		if (preheat_trials >= max_preheat_trials) {
		    preheating = false;
		    max_preheat_trials *= 2;
		    preheat_trials = 0;
		    colors = MAX_COLORS;
		    max_trials = trials_for_prob(problem.path_length, colors, 99.9);
		    g = problem.g;
		}
	    } else {
		if (++trials >= max_trials)
		    break;
		if (!hit_color_ceiling && colors < MAX_COLORS) {
		    ++colors;
		    trials = 0;
		    max_trials = trials_for_prob(problem.path_length, colors, 99.9);
		}
	    }
	} else {
	    if (preheating) {
		max_preheat_trials *= 2;
		preheat_trials = 0;
	    } else {
		hit_color_ceiling = true;
		if (colors > problem.path_length) {
		    --colors;
		    max_trials = trials_for_prob(problem.path_length, colors, 99.9);
		} else {
		    preheating = true;
		}
	    }
	}
    }
#else
    for (std::size_t i = 1; i <= preheat_trials; ++i) {
	ColoredGraph g = problem.g;
	if (i < preheat_trials) {
	    g.clear_edges();
	    weight_t max_edge_weight =
		edge_weights[std::size_t(i * (double(edge_weights.size()) / preheat_trials))];
	    
	    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v) {
		for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
		     n != problem.g.neighbors_end(v); ++n)
		    if (n->weight <= max_edge_weight)
			g.connect(v, n->neighbor, n->weight);
	    }
	    
	}
	g.color_randomly(problem.path_length);
	dynprog_trial(g, problem.start_vertices, problem.is_end_vertex, problem.find_trees,
		      problem.path_length, paths, bounds);
	if (timestamp() - last_printed > 1) {
	    info << "Pre-heating " << i << "/" << preheat_trials
		 << " m=" << g.num_edges()
		 << " paths="  << paths.size()
		 << " best " << paths.best_weight()
		 << " worst " << paths.worst_weight()
		 << " peak mem " << peak_mem_usage / 1024 / 1024 << 'M'
		 << std::endl;
	    last_printed = timestamp();
	}
    }

    ColoredGraph g = problem.g;
    for (std::size_t i = 0; i < num_trials; ++i) {
	if (timestamp() - last_printed > 1) {
	    info << "Trial " << i << "/" << num_trials << ' '
		 << paths.size( ) << " paths; best " << paths.best_weight()
		 << " worst " << paths.worst_weight()
		 << " peak mem " << peak_mem_usage / 1024 / 1024 << "M "
		 << std::endl;
	    last_printed = timestamp();
	}
	g.color_randomly(problem.num_colors);
	dynprog_trial(g, problem.start_vertices, problem.is_end_vertex,
		      problem.find_trees, problem.path_length,
		      paths, bounds);
    }
#endif
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
