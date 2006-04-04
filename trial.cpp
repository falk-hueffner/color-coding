#include "trial.h"
#include "ptree.h"

extern std::size_t peak_mem_usage;

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
    small_vertex_t vertices[];
};

inline PartialPath* find_pp(PTree& t, colorset_t c) {
    return static_cast<PartialPath*>(t.find_or_insert(c));
}

bool dynprog_trial(const ColoredGraph& g,
		   const std::vector<vertex_t>& start_vertices,
		   const std::vector<bool>& is_end_vertex,
		   bool find_trees,
		   std::size_t path_length,
		   PathSet& paths,
		   const Bounds& bounds) {
    std::size_t max_mem_usage =768 * 1024 * 1024;
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
	std::size_t edges_left = (path_length - 1) - l - 1;
	weight_t paths_worst_weight = paths.worst_weight();
	Mempool* new_pool = new Mempool();
	PTree* new_colorsets = new PTree[g.num_vertices()];
	std::size_t new_path_size = (l + 1) * sizeof (small_vertex_t);
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
		    if (pt_node->key & w_color)
			continue;
		    if (pt_node->is_leaf) {
			PartialPath* old_pp = static_cast<PartialPath*>(pt_node->data());
			weight_t new_weight = old_pp->weight + n->weight;
			if (new_weight + bounds.h(w, edges_left) < paths_worst_weight) {
			    if (edges_left == 0) {
				std::vector<vertex_t> p(old_pp->vertices,
							old_pp->vertices
							+ path_length - 2);
				p.push_back(v);
				p.push_back(w);
				paths.add(p, new_weight);
			    } else {
				PartialPath* new_pp = find_pp(new_colorsets[w],
							      pt_node->key | w_color);
				if (new_weight < new_pp->weight) {
				    new_pp->weight = new_weight;
				    memcpy(new_pp->vertices, old_pp->vertices, old_path_size);
				    new_pp->vertices[l] = v;
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
