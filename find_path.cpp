#include <vector>
#include <set>

#include "debug.h"
#include "find_path.h"
#include "ptree.h"
#include "util.h"
#include "colored_graph.h"

extern std::size_t peak_mem_usage;

std::size_t trials_for_prob(std::size_t path_length, std::size_t num_colors,
			    double success_prob);

struct PartialPath {
    weight_t weight;
    vertex_t vertices[];
};

static PartialPath* find_pp(PTree& t, colorset_t c) {
    return static_cast<PartialPath*>(t.find_or_insert(c));
}

class Bounds {
public:
    Bounds(const Problem& problem)
	: min_neighbor_weight(problem.g.num_vertices(), WEIGHT_MAX),
          min_to_goal(max_lb_edges, std::vector<weight_t>(problem.g.num_vertices(), WEIGHT_MAX)),
          min_to_anywhere(min_to_goal),
	  min_anywhere_to_goal(max_lb_edges, WEIGHT_MAX),
	  min_anywhere_to_anywhere(max_lb_edges, WEIGHT_MAX),
          min_weight(problem.path_length, problem.g.num_vertices()) {
	std::vector<weight_t> edge_weights;
	for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
	    for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
		 n != problem.g.neighbors_end(v); ++n)
		edge_weights.push_back(n->weight);
	std::sort(edge_weights.begin(), edge_weights.end());
	min_edge_weight = edge_weights.front();

	for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
	    for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
		 n != problem.g.neighbors_end(v); ++n)
		if (n->weight < min_neighbor_weight[v])
		    min_neighbor_weight[v] = n->weight;

	double last_printed = -1;
	for (vertex_t v = 0; v < problem.g.num_vertices(); ++v) {
	    if (info.is_on() && timestamp() - last_printed > 1) {
		fprintf(stderr, "lower-b %6zd/%6zd\n", v, problem.g.num_vertices());
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

    weight_t h(vertex_t v, std::size_t edges_left) const {
	return min_weight[edges_left][v];
    }

private:
    void dynprog(const Problem& problem, std::size_t s) {
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

    static const std::size_t max_lb_edges = 2;
    weight_t min_edge_weight;
    std::vector<weight_t> min_neighbor_weight;
    std::vector<std::vector<weight_t> > min_to_goal;
    std::vector<std::vector<weight_t> > min_to_anywhere;
    std::vector<weight_t> min_anywhere_to_goal;
    std::vector<weight_t> min_anywhere_to_anywhere;
    std::vector<std::vector<weight_t> > min_weight;
};
const std::size_t Bounds::max_lb_edges;

bool dynprog_trial(const ColoredGraph& g,
		   const std::vector<vertex_t>& start_vertices,
		   const std::vector<bool>& is_end_vertex,
		   bool find_trees,
		   std::size_t path_length,
		   PathSet& paths,
		   const Bounds& bounds) {
    std::size_t max_mem_usage = 768 * 1024 * 1024;
    Mempool* old_pool = new Mempool();
    PTree* old_colorsets = new PTree[g.num_vertices()];
    std::size_t old_path_size = 0;
    for (std::size_t i = 0; i < g.num_vertices(); ++i)
	new (old_colorsets + i) PTree(old_pool, sizeof (PartialPath) + old_path_size);
    PTree::Node* pt_nodes[g.num_vertices()];

    for (std::size_t i = 0; i < start_vertices.size(); ++i) {
	vertex_t s = start_vertices[i];
	PartialPath *pp = find_pp(old_colorsets[s], g.color_set(s));
	pp->weight = 0;
    }

    for (std::size_t l = 0; l < path_length - 1; ++l) {
	weight_t paths_worst_weight = paths.worst_weight();
	Mempool* new_pool = new Mempool();
	PTree* new_colorsets = new PTree[g.num_vertices()];
	std::size_t new_path_size = (l + 1) * sizeof (vertex_t);
	for (std::size_t i = 0; i < g.num_vertices(); ++i)
	    new (new_colorsets + i) PTree(new_pool, sizeof (PartialPath) + new_path_size);
#if 0
	std::size_t life = 0, dead = 0;
	for (vertex_t v = 0; v < g.num_vertices(); ++v) {
	    if (old_colorsets[v].root)
		++life;
	    else
		++dead;
	}
	std::cerr << "l = " << l << " life = " << life << " dead = " << dead << endl;
#endif
	for (vertex_t v = 0; v < g.num_vertices(); ++v) {
	    if (!old_colorsets[v].root)
		continue;
	    for (Graph::neighbor_it n = g.neighbors_begin(v); n != g.neighbors_end(v); ++n) {
		vertex_t w = n->neighbor;
		colorset_t w_color = g.color_set(w);
		std::size_t num_pt_nodes = 0;
		pt_nodes[num_pt_nodes++] = old_colorsets[v].root;
		while (num_pt_nodes) {
		    PTree::Node* pt_node = pt_nodes[--num_pt_nodes];
		    if (pt_node->key & w_color)
			continue;
		    if (pt_node->is_leaf) {
			PartialPath* old_pp = static_cast<PartialPath*>(pt_node->data());
			weight_t new_weight = old_pp->weight + n->weight;
			std::size_t edges_left = (path_length - 1) - l - 1;
			if (new_weight + bounds.h(w, edges_left) < paths_worst_weight) {
			    PartialPath* new_pp = find_pp(new_colorsets[w],
							  pt_node->key | w_color);
			    if (new_weight < new_pp->weight) {
				new_pp->weight = new_weight;
				memcpy(new_pp->vertices, old_pp->vertices, old_path_size);
				new_pp->vertices[l] = v;
			    }
			    if (find_trees) {
				PartialPath* new_pp = find_pp(new_colorsets[v],
							      pt_node->key | w_color);
				if (new_weight < new_pp->weight) {
				    new_pp->weight = new_weight;
				    memcpy(new_pp->vertices, old_pp->vertices, old_path_size);
				    new_pp->vertices[l] = w;
				}
			    }
			    if (old_pool->mem_usage() + new_pool->mem_usage() > max_mem_usage) {
				peak_mem_usage = std::max(peak_mem_usage,
							  old_pool->mem_usage()
							  + new_pool->mem_usage());
				delete[] old_colorsets;
				delete old_pool;
				delete[] new_colorsets;
				delete new_pool;
				return false;
			    }
			}
		    } else {
			pt_nodes[num_pt_nodes++] = pt_node->left;
			pt_nodes[num_pt_nodes++] = pt_node->right;
		    }
		}
	    }
	}
	peak_mem_usage = std::max(peak_mem_usage, old_pool->mem_usage() + new_pool->mem_usage());
#if 0
	std::size_t old_mem = old_pool->mem_usage(), new_mem = new_pool->mem_usage();
	std::cerr << "l=" << l << ": " << old_mem << " + l=" << l+1 << ": " << new_mem
		  << " = " << (old_mem + new_mem) / 1024 / 1024 << "M\n";
#endif
	delete[] old_colorsets;
	delete old_pool;
	old_colorsets = new_colorsets;
	old_pool = new_pool;
	old_path_size = new_path_size;
    }
    
    for (vertex_t v = 0; v < g.num_vertices(); ++v) {
	if (!is_end_vertex[v])
	    continue;
	std::size_t num_pt_nodes = 0;
	if (old_colorsets[v].root)
	    pt_nodes[num_pt_nodes++] = old_colorsets[v].root;
	while (num_pt_nodes) {
	    PTree::Node* pt_node = pt_nodes[--num_pt_nodes];
	    if (pt_node->is_leaf) {
		PartialPath* pp = static_cast<PartialPath*>(pt_node->data());
		if (pp->weight < paths.worst_weight()) {
		    std::vector<vertex_t> p(pp->vertices, pp->vertices + path_length - 1);
		    p.push_back(v);
		    paths.add(p, pp->weight);
		}
	    } else {
		pt_nodes[num_pt_nodes++] = pt_node->left;
		pt_nodes[num_pt_nodes++] = pt_node->right;
	    }
	}
    }
    delete[] old_colorsets;
    delete old_pool;
    return true;
}

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
