#include <vector>
#include <set>

#include "debug.h"
#include "find_path.h"
#include "ptree.h"
#include "util.h"
#include "colored_graph.h"

extern std::size_t peak_mem_usage;

struct PartialPath {
    weight_t weight;
    vertex_t vertices[];
};

static PartialPath* find_pp(PTree& t, colorset_t c) {
    return static_cast<PartialPath*>(t.find_or_insert(c));
}

void dynprog_trial(const ColoredGraph& g,
		   const std::vector<vertex_t>& start_vertices,
		   const std::vector<bool>& is_end_vertex,
		   bool find_trees,
		   std::size_t path_length,
		   PathSet& paths,
		   weight_t min_edge_weight) {
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
	weight_t weight_threshold =
	    paths.worst_weight() - ((path_length - 1) - l - 1) * min_edge_weight;
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
			if (new_weight < weight_threshold) {
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
}

PathSet lightest_path(const Problem& problem,
		      std::size_t num_trials, std::size_t num_paths,
		      std::size_t max_common, std::size_t preheat_trials) {
    std::vector<weight_t> edge_weights;
    
    for (vertex_t v = 0; v < problem.g.num_vertices(); ++v)
	for (Graph::neighbor_it n = problem.g.neighbors_begin(v);
	     n != problem.g.neighbors_end(v); ++n)
	    edge_weights.push_back(n->weight);
    std::sort(edge_weights.begin(), edge_weights.end());
    weight_t min_edge_weight = edge_weights.front();

    PathSet paths(num_paths, max_common);
    double last_printed = -1;
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
		      problem.path_length, paths, min_edge_weight);
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
		      paths, min_edge_weight);
    }
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
