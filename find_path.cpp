#include <vector>
#include <set>

#include "debug.h"
#include "find_path.h"
#include "ptree.h"
#include "util.h"

extern std::size_t peak_mem_usage;

struct PartialPath {
    weight w;
    vertex vertices[];
};

static PartialPath* find_pp(PTree& t, colorset c) {
    return static_cast<PartialPath*>(t.find_or_insert(c));
}

void dynprog_trial(const Graph& g, const std::vector<vertex>& start_vertices,
		   const std::vector<bool>& is_end_vertex,
		   std::size_t path_length, PathSet& paths,
		   weight min_edge_weight) {
    Mempool* old_pool = new Mempool();
    PTree* old_colorsets = new PTree[g.num_vertices()];
    std::size_t old_path_size = 0;
    for (std::size_t i = 0; i < g.num_vertices(); ++i)
	new (old_colorsets + i) PTree(old_pool, sizeof (PartialPath) + old_path_size);
    PTree::Node* pt_nodes[g.num_vertices()];

    for (std::size_t i = 0; i < start_vertices.size(); ++i) {
	vertex s = start_vertices[i];
	PartialPath *pp = find_pp(old_colorsets[s], g.color_set(s));
	pp->w = 0;
    }

    for (std::size_t l = 0; l < path_length - 1; ++l) {
	weight weight_threshold =
	    paths.worst_weight() - ((path_length - 1) - l - 1) * min_edge_weight;
	Mempool* new_pool = new Mempool();
	PTree* new_colorsets = new PTree[g.num_vertices()];
	std::size_t new_path_size = (l + 1) * sizeof (vertex);
	for (std::size_t i = 0; i < g.num_vertices(); ++i)
	    new (new_colorsets + i) PTree(new_pool, sizeof (PartialPath) + new_path_size);
#if 0
	std::size_t life = 0, dead = 0;
	for (vertex v = 0; v < g.num_vertices(); ++v) {
	    if (old_colorsets[v].root)
		++life;
	    else
		++dead;
	}
	std::cerr << "l = " << l << " life = " << life << " dead = " << dead << endl;
#endif
	for (vertex v = 0; v < g.num_vertices(); ++v) {
	    if (!old_colorsets[v].root)
		continue;
	    for (std::size_t i = 0; i < g.deg(v); ++i) {
		vertex w = g.neighbor(v, i);
		weight edge_weight = g.edge_weight(v, i);
		colorset w_color = g.color_set(w);
		std::size_t num_pt_nodes = 0;
		pt_nodes[num_pt_nodes++] = old_colorsets[v].root;
		while (num_pt_nodes) {
		    PTree::Node* pt_node = pt_nodes[--num_pt_nodes];
		    if (pt_node->key & w_color)
			continue;
		    if (pt_node->is_leaf) {
			PartialPath* old_pp = static_cast<PartialPath*>(pt_node->data());
			weight new_weight = old_pp->w + edge_weight;
			if (new_weight < weight_threshold) {
			    PartialPath* new_pp = find_pp(new_colorsets[w],
							  pt_node->key | w_color);
			    if (new_weight < new_pp->w) {
				new_pp->w = new_weight;
				memcpy(new_pp->vertices, old_pp->vertices, old_path_size);
				new_pp->vertices[l] = v;
			    }
			}
		    } else {
			pt_nodes[num_pt_nodes++] = pt_node->left;
			pt_nodes[num_pt_nodes++] = pt_node->right;
		    }
		}
	    }
	}
	std::size_t mem_usage = old_pool->mem_usage() + new_pool->mem_usage();
	if (mem_usage > peak_mem_usage)
	    peak_mem_usage = mem_usage;
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
    
    for (vertex v = 0; v < g.num_vertices(); ++v) {
	if (!is_end_vertex[v])
	    continue;
	std::size_t num_pt_nodes = 0;
	if (old_colorsets[v].root)
	    pt_nodes[num_pt_nodes++] = old_colorsets[v].root;
	while (num_pt_nodes) {
	    PTree::Node* pt_node = pt_nodes[--num_pt_nodes];
	    if (pt_node->is_leaf) {
		PartialPath* pp = static_cast<PartialPath*>(pt_node->data());
		if (pp->w < paths.worst_weight()) {
		    std::vector<vertex> p(pp->vertices, pp->vertices + path_length - 1);
		    p.push_back(v);
		    paths.add(p, pp->w);
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

PathSet lightest_path(const Graph& g_in, const std::vector<vertex>& start_vertices,
		      const std::vector<bool>& is_end_vertex,		      
		      std::size_t path_length, std::size_t num_colors,
		      std::size_t num_trials, std::size_t num_paths,
		      std::size_t max_common, std::size_t preheat_trials) {
    std::vector<weight> edge_weights;
    for (vertex v = 0; v < g_in.num_vertices(); ++v)
	for (std::size_t i = 0; i < g_in.deg(v); ++i)
	    edge_weights.push_back(g_in.edge_weight(v, i));
    std::sort(edge_weights.begin(), edge_weights.end());
    weight min_edge_weight = edge_weights.front();

    PathSet paths(num_paths, max_common);
    double last_printed = -1;
    for (std::size_t i = 1; i <= preheat_trials; ++i) {
	Graph g = g_in;
	if (i < preheat_trials) {
	    g.clear_edges();
	    weight max_edge_weight =
		edge_weights[std::size_t(i * (double(edge_weights.size()) / preheat_trials))];
	    for (vertex v = 0; v < g_in.num_vertices(); ++v) {
		for (std::size_t i = 0; i < g_in.deg(v); ++i) {
		    vertex w = g_in.neighbor(v, i);
		    weight edge_weight = g_in.edge_weight(v, i);
		    if (edge_weight <= max_edge_weight)
			g.connect(v, w, edge_weight);
		}
	    }
	}
	g.color_nodes(path_length);
	dynprog_trial(g, start_vertices, is_end_vertex, path_length, paths, min_edge_weight);
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

    Graph g = g_in;
    for (std::size_t i = 0; i < num_trials; ++i) {
	if (timestamp() - last_printed > 1) {
	    info << "Trial " << i << "/" << num_trials << ' '
		 << paths.size( ) << " paths; best " << paths.best_weight()
		 << " worst " << paths.worst_weight()
		 << " peak mem " << peak_mem_usage / 1024 / 1024 << "M "
		 << std::endl;
	    last_printed = timestamp();
	}
	g.color_nodes(num_colors);
	dynprog_trial(g, start_vertices, is_end_vertex, path_length, paths, min_edge_weight);
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
