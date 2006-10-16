#include <vector>
#include <stdint.h>

#include "bounds.h"
#include "colored_graph.h"
#include "pathset.h"
#include "ptree.h"
#include "types.h"

struct PartialPath {
    weight_t weight;
    uint8_t insertions;
    small_vertex_t vertices[1];
};

inline PartialPath* find_pp(PTree& t, colorset_t c, Mempool& mempool,
			    std::size_t num_vertices) {
    std::size_t size = num_vertices * sizeof (small_vertex_t);
    return static_cast<PartialPath*>(t.find_or_insert(c, mempool, size));
}

bool dynprog_trial(const ColoredGraph& g,
		   std::vector<std::vector<weight_t> > match_weights, // match_weights[l][v]
		   std::size_t max_insertions,
		   std::size_t max_deletions,
		   weight_t insertion_cost,
		   weight_t deletion_cost,
		   PathSet& paths,
		   const Bounds& bounds) {
    const std::size_t path_length = match_weights.size();

    Mempool* old_pool = new Mempool();
    //PTree* old_colorsets = new PTree[g.num_vertices()];
    std::vector<std::vector<PTree> >* old_colorsets
	= new std::vector<std::vector<PTree> >(max_deletions + 1, g.num_vertices());
    PTree::Node** pt_node_stack = new PTree::Node*[g.num_vertices()];

    // Initialize first layer
    for (vertex_t s = 0; s < g.num_vertices(); ++s) {
	PartialPath *pp = find_pp((*old_colorsets)[0][s], g.color_singleton(s), *old_pool, 1);
	pp->weight = match_weights[0][s];
	pp->insertions = 0;
	pp->vertices[0] = s;
	for (std::size_t deletions = 1; deletions <= max_deletions; ++deletions) {
	    PartialPath *pp = find_pp((*old_colorsets)[deletions][s], g.color_singleton(s),
				      *old_pool, 1);
	    pp->weight = deletions * deletion_cost;
	    pp->insertions = 0;
	    pp->vertices[0] = s;
	}
    }

    for (vertex_t l = 0; l < path_length - 1; ++l) {
	const std::size_t edges_left = (path_length - 1) - l - 1;
	Mempool* new_pool = new Mempool();
	std::vector<std::vector<PTree> >* new_colorsets
	    = new std::vector<std::vector<PTree> >(max_deletions + 1, g.num_vertices());

	for (std::size_t deletions = 0; deletions <= max_deletions; ++ deletions) {
	    for (vertex_t v = 0; v < g.num_vertices(); ++v) {
		if (!(*old_colorsets)[deletions][v].root)
		    continue;

		// Try a normal match
		for (Graph::neighbor_it n = g.neighbors_begin(v); n != g.neighbors_end(v); ++n) {
		    vertex_t w = n->neighbor;
		    colorset_t w_color = g.color_singleton(w);
		    std::size_t pt_node_stack_size = 0;
		    pt_node_stack[pt_node_stack_size++] = (*old_colorsets)[deletions][v].root;
		    while (pt_node_stack_size) {
			PTree::Node* pt_node = pt_node_stack[--pt_node_stack_size];
			if (pt_node->contains(w_color))
			    continue;
			if (pt_node->is_leaf) {
			    PartialPath* old_pp = static_cast<PartialPath*>(pt_node->data());
			    weight_t new_weight = old_pp->weight + n->weight
						+ match_weights[l][w];
			    if (new_weight + bounds.h(w, edges_left) < paths.worst_weight()) {
				if (l + 1 == path_length - 1) {
				    std::vector<vertex_t> p(old_pp->vertices,
							    old_pp->vertices
							    + (l + old_pp->insertions));
				    p.push_back(w);
				    paths.add(p, new_weight);
				} else {
				    std::size_t old_num_vertices = l + old_pp->insertions + 1;
				    PartialPath* new_pp = find_pp((*new_colorsets)[deletions][w],
								  pt_node->key | w_color,
								  *new_pool,
								  old_num_vertices + 1);
				    if (new_weight < new_pp->weight) {
					new_pp->weight = new_weight;
					new_pp->insertions = old_pp->insertions;
					memcpy(new_pp->vertices, old_pp->vertices,
					       old_num_vertices * sizeof old_pp->vertices[0]);
					new_pp->vertices[l] = v;
				    }
				}
			    }
			} else {
			    pt_node_stack[pt_node_stack_size++] = pt_node->left;
			    pt_node_stack[pt_node_stack_size++] = pt_node->right;
			}
		    }
		}

		if (deletions < max_deletions) {
		    // Try deletion.
		    std::size_t pt_node_stack_size = 0;
		    pt_node_stack[pt_node_stack_size++] = (*old_colorsets)[deletions][v].root;
		    while (pt_node_stack_size) {
			PTree::Node* pt_node = pt_node_stack[--pt_node_stack_size];
			if (pt_node->is_leaf) {
			    PartialPath* old_pp = static_cast<PartialPath*>(pt_node->data());
			    weight_t new_weight = old_pp->weight + deletion_cost;
			    std::size_t old_num_vertices = l + 1 + old_pp->insertions;
			    if (new_weight + bounds.h(v, edges_left) < paths.worst_weight()) {
				if (l + 1 == path_length - 1) {
				    std::vector<vertex_t> p(old_pp->vertices,
							    old_pp->vertices + old_num_vertices);
				    paths.add(p, new_weight);
				} else {
				    PartialPath* new_pp
					= find_pp((*new_colorsets)[deletions + 1][v],
						  pt_node->key, *new_pool, old_num_vertices + 1);
				    if (new_weight < new_pp->weight) {
					new_pp->weight = new_weight;
					new_pp->insertions = old_pp->insertions;
					memcpy(new_pp->vertices, old_pp->vertices,
					       old_num_vertices * sizeof old_pp->vertices[0]);
					new_pp->vertices[l] = v;
				    }
				}
			    }
			} else {
			    pt_node_stack[pt_node_stack_size++] = pt_node->left;
			    pt_node_stack[pt_node_stack_size++] = pt_node->right;
			}
		    }
		}
	    }
	}

	if (l + 1 == path_length - 1)
	    continue;

	// add insertions

	
	for (std::size_t deletions = 0; deletions <= max_deletions; ++ deletions) {
	    for (vertex_t v = 0; v < g.num_vertices(); ++v) {
		if (!(*old_colorsets)[deletions][v].root)
		    continue;

		for (Graph::neighbor_it n = g.neighbors_begin(v); n != g.neighbors_end(v); ++n) {
		    vertex_t w = n->neighbor;
		    colorset_t w_color = g.color_singleton(w);
		    std::size_t pt_node_stack_size = 0;
		    pt_node_stack[pt_node_stack_size++] = (*old_colorsets)[deletions][v].root;
		    /* We modify old_colorsets[deletions][v] while we iterate over it. We still
		       don't miss any newly inserted color set, since any newly inserted color
		       set has 1 bit more set than an existing entry, and therefore will be
		       encountered later in iteration, since the zero branch of a branching bit
		       is always traversed first.  */
		    while (pt_node_stack_size) {
			PTree::Node* pt_node = pt_node_stack[--pt_node_stack_size];
			if (pt_node->contains(w_color))
			    continue;
			if (pt_node->is_leaf) {
			    PartialPath* old_pp = static_cast<PartialPath*>(pt_node->data());
			    weight_t new_weight = old_pp->weight + n->weight + insertion_cost;
			    if (old_pp->insertions + 1 <= max_insertions
				&& new_weight + bounds.h(w, edges_left) < paths.worst_weight()) {
				std::size_t old_num_vertices = l + old_pp->insertions + 1;
				PartialPath* new_pp = find_pp((*new_colorsets)[deletions][w],
							      pt_node->key | w_color, *new_pool,
							      old_num_vertices + 1);
				if (new_weight < new_pp->weight) {
				    new_pp->weight = new_weight;
				    new_pp->insertions = old_pp->insertions + 1;
				    memcpy(new_pp->vertices, old_pp->vertices,
					   old_num_vertices * sizeof old_pp->vertices[0]);
				    new_pp->vertices[l] = v;
				}
			    }
			} else {
			    pt_node_stack[pt_node_stack_size++] = pt_node->left;
			    pt_node_stack[pt_node_stack_size++] = pt_node->right;
			}
		    }
		}
	    }
	}
	delete old_pool;
	delete old_colorsets;
	old_pool = new_pool;
	old_colorsets = new_colorsets;
    }

    delete old_pool;
    delete old_colorsets;
    delete[] pt_node_stack;

    return true;
}
