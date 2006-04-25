#include <math.h>
#include <queue>

#include "ptree.h"
#include "trial.h"
#include "util.h"

extern std::size_t peak_mem_usage;

#define STORE_ONLY_COLORS 1

// returns ln(n!)
inline double lfact(std::size_t n) {
    return lgamma(n + 1);
}

std::size_t trials_for_prob(std::size_t path_length, std::size_t num_colors,
			    double success_prob) {
    std::size_t k = path_length;
    std::size_t x = num_colors - path_length;
    double colorful_prob = exp(lfact(k + x) - lfact(x) - k * log(double(k + x)));
    return std::size_t(ceil(log1p(-success_prob) / log1p(-colorful_prob)));
}

struct PartialPath {
    weight_t weight;
#if STORE_ONLY_COLORS
    unsigned char vertices[];
#else
    small_vertex_t vertices[];
#endif
};

inline PartialPath* find_pp(PTree& t, colorset_t c) {
    return static_cast<PartialPath*>(t.find_or_insert(c));
}

inline std::vector<vertex_t>
recover_path(const ColoredGraph& g, const std::vector<bool>& is_start_vertex,
	     vertex_t v, const unsigned char *vertices,
	     std::size_t color_size, std::size_t path_length) {
    std::queue<vertex_t> q;
    std::vector<weight_t> wt(g.num_vertices(), WEIGHT_MAX);
    std::vector<vertex_t> pred(g.num_vertices(), (vertex_t) -1);
    std::size_t color_v = g.color(v);
    std::size_t layer = path_length - 1 - 1;
    std::size_t color_neigh = peek_bits(vertices, color_size, layer);
    wt[v] = 0;
    q.push(v);
    while (!q.empty()) {
	vertex_t v = q.front();
	q.pop();
	if (g.color(v) != color_v) {
	    --layer;
	    color_v = g.color(v);
	    color_neigh = peek_bits(vertices, color_size, layer);
	}
	for (Graph::neighbor_it n = g.neighbors_begin(v); n != g.neighbors_end(v); ++n) {
	    vertex_t w = n->neighbor;
	    if (g.color(w) == color_neigh) {
		weight_t edge_weight = n->weight;
		if (wt[v] + edge_weight < wt[w]) {
		    if (pred[w] == -1 && layer > 0)
			q.push(w);
		    wt[w] = wt[v] + edge_weight;
		    pred[w] = v;
		}
	    }
	}
    }
    assert(layer == 0);
    std::size_t start_color = peek_bits(vertices, color_size, 0);
    vertex_t start = (vertex_t) -1;
    weight_t best_weight = WEIGHT_MAX;
    for (std::size_t i = 0; i < g.num_vertices(); ++i) {
	if (is_start_vertex[i] && g.color(i) == start_color) {
	    if (wt[i] < best_weight) {
		start = i;
		best_weight = wt[i];
	    }
	}
    }
    
    std::vector<vertex_t> p;
    for (std::size_t i = 0; i < path_length; ++i) {
	p.push_back(start);
	start = pred[start];
    }
    return p;
}

bool dynprog_trial(const ColoredGraph& g,
		   const std::vector<bool>& is_start_vertex,
		   const std::vector<bool>& is_end_vertex,
		   std::size_t path_length,
		   std::size_t num_colors,
		   PathSet& paths,
		   const Bounds& bounds) {
    std::size_t max_mem_usage = 768 * 1024 * 1024;
    std::size_t color_size = bits_needed(num_colors);
    Mempool* old_pool = new Mempool();
    PTree* old_colorsets = new PTree[g.num_vertices()];
    std::size_t old_path_size = 0;
    for (std::size_t i = 0; i < g.num_vertices(); ++i)
	new (old_colorsets + i) PTree(old_pool, sizeof (PartialPath) + old_path_size);
    PTree::Node* pt_nodes[g.num_vertices()];

    for (vertex_t s = 0; s < is_start_vertex.size(); ++s) {
	if (is_start_vertex[s]) {
	    PartialPath *pp = find_pp(old_colorsets[s], g.color_set(s));
	    pp->weight = 0;
	}
    }

    for (std::size_t l = 0; l < path_length - 1; ++l) {
	std::size_t edges_left = (path_length - 1) - l - 1;
	Mempool* new_pool = new Mempool();
	PTree* new_colorsets = new PTree[g.num_vertices()];
#if STORE_ONLY_COLORS
	std::size_t new_path_size = (((l + 1) * color_size) + 7) / 8;
#else
	std::size_t new_path_size = (l + 1) * sizeof (small_vertex_t);
#endif
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
		if (edges_left == 0 && !is_end_vertex[w])
		    continue;
		colorset_t w_color = g.color_set(w);
		std::size_t num_pt_nodes = 0;
		pt_nodes[num_pt_nodes++] = old_colorsets[v].root;
		while (num_pt_nodes) {
		    PTree::Node* pt_node = pt_nodes[--num_pt_nodes];
		    if (pt_node->contains(w_color))
			continue;
		    if (pt_node->is_leaf) {
			PartialPath* old_pp = static_cast<PartialPath*>(pt_node->data());
			weight_t new_weight = old_pp->weight + n->weight;
			if (new_weight + bounds.h(w, edges_left) < paths.worst_weight()) {
			    if (edges_left == 0) {
#if STORE_ONLY_COLORS
				std::vector<vertex_t> p
				    = recover_path(g, is_start_vertex, v, old_pp->vertices,
						   color_size, path_length - 1);
				p.push_back(w);
#else
				std::vector<vertex_t> p(old_pp->vertices,
							old_pp->vertices
							+ path_length - 2);
				p.push_back(v);
				p.push_back(w);
#endif
				paths.add(p, new_weight);
			    } else {
				PartialPath* new_pp = find_pp(new_colorsets[w],
							      pt_node->key | w_color);
				if (new_weight < new_pp->weight) {
				    new_pp->weight = new_weight;
				    memcpy(new_pp->vertices, old_pp->vertices, old_path_size);
#if STORE_ONLY_COLORS
				    poke_bits(new_pp->vertices, color_size, l, g.color(v));
#else
				    new_pp->vertices[l] = v;
#endif
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

    delete[] old_colorsets;
    delete old_pool;
    return true;
}
