#include <algorithm>
#include <set>
#include <vector>

#include <math.h>
#include <stdio.h>

#include "bounds.h"
#include "colored_graph.h"
#include "debug.h"
#include "find_path.h"
#include "trial.h"
#include "qpath_trial.h"
#include "util.h"

#ifdef _GUI_
	#include "gui/guiApp.h"
#endif

extern std::size_t peak_mem_usage;

#ifndef _GUI_
PathSet lightest_path(const Problem& problem, std::size_t num_paths,
		      std::size_t max_common, Bounds::Mode mode, std::size_t max_lb_edges) {
#else
PathSet lightest_path(const Problem& problem, std::size_t num_paths,
		      std::size_t max_common, Bounds::Mode mode, std::size_t max_lb_edges, SearchThread *search) {
#endif
    std::vector<weight_t> edge_weights;
    
    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
	for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
	     n != problem.g.neighbors_end(v); ++n)
	    edge_weights.push_back(n->weight);
    std::sort(edge_weights.begin(), edge_weights.end());

#ifdef _GUI_
	wxMutexGuiEnter();
	wxGetApp().frame->StatusBar->SetStatusText(wxT("Bounds computation"),1);
	wxMutexGuiLeave();
#endif
    PathSet paths(num_paths, max_common);
    if (problem.g.num_vertices() == 0)
	return paths;
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
		weight_t max_edge_weight = edge_weights[int(edge_weights.size() * ratio + 0.5)];
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

#ifdef _GUI_
	wxString statustext;
	statustext << (preheating ? wxT("Pre-heating") : wxT("Final search"))  << wxT(" - trial ")
		<< ((unsigned long) (preheating ? preheat_trials : trials) + 1) << wxT(" / ")
		<< (unsigned long) (preheating ? max_preheat_trials : max_trials);
	wxMutexGuiEnter();
	wxGetApp().frame->StatusBar->SetStatusText(statustext,1);
	wxMutexGuiLeave();

	if (search->TestDestroy()) return paths;
#endif

	if (info.is_on())
	    fprintf(stderr, "%7s %6lu/%6lu edges=%lu colors=%lu %luM %lu:[%10.8f,%10.8f]\n",
		    preheating ? "preheat" : "trial",
		    (unsigned long) (preheating ? preheat_trials : trials),
		    (unsigned long) (preheating ? max_preheat_trials : max_trials),
		    (unsigned long) g.num_edges(),
		    (unsigned long) colors,
		    (unsigned long) (peak_mem_usage / 1024 / 1024),
		    (unsigned long) paths.size(),
		    paths.best_weight(),
		    paths.worst_weight());
		std::size_t old_paths_size = paths.size();
	weight_t old_worst_weight = paths.worst_weight();
	bool ok;
	if (problem.match_weights.size()) {
	    ok = qpath_trial(g,
			     problem.match_weights,
			     problem.max_insertions,
			     problem.max_deletions,
			     problem.insertion_cost,
			     problem.deletion_cost,
			     paths,
			     bounds);
	} else {
	    ok = dynprog_trial(g, problem.is_start_vertex, problem.is_end_vertex,
			       problem.path_length, colors,
			       paths, bounds);
	}
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
