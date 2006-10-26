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
    uint8_t num_vertices;
    small_vertex_t vertices[1];
};

inline PartialPath* find_pp(PTree& t, colorset_t c, Mempool& mempool,
			    std::size_t num_vertices) {
    std::size_t size = sizeof (PartialPath) + (num_vertices - 1) * sizeof (small_vertex_t);
    return static_cast<PartialPath*>(t.find_or_insert(c, mempool, size));
}

bool qpath_trial(const ColoredGraph& g,
		 std::vector<std::vector<weight_t> > match_weights, // match_weights[l][v]
		 std::size_t max_insertions,
		 std::size_t max_deletions,
		 weight_t insertion_cost,
		 weight_t deletion_cost,
		 PathSet& paths,
		 const Bounds& bounds) {
    const std::size_t path_length = match_weights.size();

    Mempool* old_pool = new Mempool();
    std::vector<std::vector<PTree> >* old_colorsets
	= new std::vector<std::vector<PTree> >(max_deletions + 1, g.num_vertices());
    PTree::Node** pt_node_stack = new PTree::Node*[g.num_vertices()];

    // Initialize first layer
    for (vertex_t s = 0; s < g.num_vertices(); ++s) {
	PartialPath *pp = find_pp((*old_colorsets)[0][s], g.color_singleton(s), *old_pool, 0);
	pp->weight = match_weights[0][s];
	pp->insertions = 0;
	pp->num_vertices = 0;
	for (std::size_t deletions = 1; deletions <= max_deletions; ++deletions) {
	    PartialPath *pp = find_pp((*old_colorsets)[deletions][s], g.color_singleton(s),
				      *old_pool, 0);
	    pp->weight = deletions * deletion_cost + match_weights[deletions][s];
	    pp->insertions = 0;
	    pp->num_vertices = deletions;
	    for (std::size_t i = 0; i < deletions; ++i)
		pp->vertices[i] = DELETED_VERTEX;
	}
    }

    for (vertex_t l = 0; l < path_length - 1; ++l) {
	const std::size_t edges_left = (path_length - 1) - l - 1;
	Mempool* new_pool = new Mempool();
	std::vector<std::vector<PTree> >* new_colorsets
	    = new std::vector<std::vector<PTree> >(max_deletions + 1, g.num_vertices());
	std::size_t alloc_vertices = l + max_insertions + max_deletions + 1;

	for (std::size_t deletions = 0; deletions <= max_deletions; ++deletions) {
	    std::size_t deletions_left = max_deletions - deletions;
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
						+ match_weights[l + 1][w];
			    if (new_weight + bounds.h(w, edges_left, deletions_left)
				 < paths.worst_weight()) {
				std::size_t old_num_vertices = old_pp->num_vertices;
				if (l + 1 == path_length - 1) {
				    std::vector<vertex_t> p(old_pp->vertices,
							    old_pp->vertices + old_num_vertices);
				    p.push_back(v);
				    p.push_back(w);
				    paths.add(p, new_weight);
				} else {
				    PartialPath* new_pp = find_pp((*new_colorsets)[deletions][w],
								  pt_node->key | w_color,
								  *new_pool, alloc_vertices);
				    if (new_weight < new_pp->weight) {
					new_pp->weight = new_weight;
					new_pp->insertions = old_pp->insertions;
					new_pp->num_vertices = old_num_vertices + 1;
					memcpy(new_pp->vertices, old_pp->vertices,
					       old_num_vertices * sizeof old_pp->vertices[0]);
					new_pp->vertices[old_num_vertices] = v;
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
			    // No deletion after an insertion.
			    if (old_pp->num_vertices
				&& old_pp->vertices[old_pp->num_vertices - 1] < 0)
				continue;
			    
			    weight_t new_weight = old_pp->weight + deletion_cost;
			    std::size_t old_num_vertices = old_pp->num_vertices;
			    if (new_weight + bounds.h(v, edges_left, deletions_left)
				< paths.worst_weight()) {
				if (l + 1 == path_length - 1) {
				    std::vector<vertex_t> p(old_pp->vertices,
							    old_pp->vertices + old_num_vertices);
				    p.push_back(v);
				    p.push_back(DELETED_VERTEX);
				    paths.add(p, new_weight);
				} else {
				    assert (old_num_vertices + 1 <= alloc_vertices);
				    PartialPath* new_pp
					= find_pp((*new_colorsets)[deletions + 1][v],
						  pt_node->key, *new_pool, alloc_vertices);
				    if (new_weight < new_pp->weight) {
					new_pp->weight = new_weight;
					new_pp->insertions = old_pp->insertions;
					new_pp->num_vertices = old_num_vertices + 1;
					memcpy(new_pp->vertices, old_pp->vertices,
					       old_num_vertices * sizeof old_pp->vertices[0]);
					new_pp->vertices[old_num_vertices] = DELETED_VERTEX;
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
	if(1)
	for (std::size_t deletions = 0; deletions <= max_deletions; ++deletions) {
	    std::size_t deletions_left = max_deletions - deletions;
	    for (vertex_t v = 0; v < g.num_vertices(); ++v) {
		if (!(*new_colorsets)[deletions][v].root)
		    continue;

		for (Graph::neighbor_it n = g.neighbors_begin(v); n != g.neighbors_end(v); ++n) {
		    vertex_t w = n->neighbor;
		    colorset_t w_color = g.color_singleton(w);
		    std::size_t pt_node_stack_size = 0;
		    pt_node_stack[pt_node_stack_size++] = (*new_colorsets)[deletions][v].root;
		    /* We modify new_colorsets[deletions][v] while we iterate over it. We still
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
				&& (new_weight + bounds.h(w, edges_left, deletions_left)
				    < paths.worst_weight())) {
				std::size_t old_num_vertices = old_pp->num_vertices;
				PartialPath* new_pp = find_pp((*new_colorsets)[deletions][w],
							      pt_node->key | w_color, *new_pool,
							      alloc_vertices);
				if (new_weight < new_pp->weight) {
				    new_pp->weight = new_weight;
				    new_pp->insertions = old_pp->insertions + 1;
				    new_pp->num_vertices = old_num_vertices + 1;
				    memcpy(new_pp->vertices, old_pp->vertices,
					   old_num_vertices * sizeof old_pp->vertices[0]);
				    new_pp->vertices[old_num_vertices] = -v;
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
