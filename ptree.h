#ifndef PTREE_HH
#define PTREE_HH

#include <stdint.h>
#include <cstddef>

#include "mempool.h"
#include "types.h"
#include "util.h"

class PTree {
public:
    typedef colorset_t key_t;

    PTree() { }
    PTree(Mempool* n_mempool, std::size_t n_leaf_size = 0)
	: mempool(n_mempool), root(NULL), leaf_size(n_leaf_size) {
	// ensure alignment
	leaf_size += sizeof (void*) - 1;
	leaf_size -= leaf_size % sizeof (void*);
    }

    bool contains(key_t k) const;
    void* find_or_insert(key_t k);
    void set_leaf_size(std::size_t n_leaf_size) { leaf_size = n_leaf_size; }
    void dump() const;

//private:
    // little-endian patricia trees
    struct Leaf {
	bool  is_leaf : 1;	// 1 in leafs, 0 in branches
	key_t key : MAX_COLORS; // key
    };
    struct Node : public Leaf {
	Node* left;		// subtree where branchBit is 0
	Node* right;		// subtree where branchBit is 1

	bool branch_matches(key_t k) const {
	    key_t cmp = k ^ key;
	    cmp ^= cmp - 1;
	    return cmp >= key;
	}
	Node** pchild(key_t k) {
	    return ((k ^ key) & key) ? &left : &right;
	}
	bool contains(key_t k) {
	    if (is_leaf)
		return (key & k) != 0;
	    else
		return k < (key >> 1) && (key & k) != 0;
	}
	void* data() {
	    assert(is_leaf);
	    return reinterpret_cast<unsigned char*>(this) + sizeof (PTree::Leaf);
	}
	void dump(int indent = 0) const;
    };

    inline Node* alloc_leaf(key_t c) {
	Node* leaf = static_cast<PTree::Node*>(mempool->alloc(sizeof (Leaf) + leaf_size));
	leaf->is_leaf = true;
	leaf->key = c;
	// Application specific hack: initialize first entry to a large FP number
	*(float*) (leaf->data()) = WEIGHT_MAX;
	return leaf;
    }
    inline Node* alloc_branch(key_t k, Node* node, Node* leaf) {
	key_t branch_bit = isolate_lowest_bit(k ^ node->key);
	Node* branch = static_cast<PTree::Node*>(mempool->alloc(sizeof (Node)));
	branch->is_leaf = false;
	branch->key = k & (branch_bit - 1);
	branch->key |= branch_bit;
	if (k & branch_bit) {
	    branch->left  = node;
	    branch->right = leaf;
	} else {
	    branch->left  = leaf;
	    branch->right = node;
	}
	return branch;
    }

    Mempool* mempool;
    Node* root;
    std::size_t leaf_size;
};

#endif	// PTREE_HH
