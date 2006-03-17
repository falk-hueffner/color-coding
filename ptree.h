#ifndef PTREE_HH
#define PTREE_HH

#include <stdint.h>
#include <cstddef>

#include "types.h"
#include "mempool.h"


class PTree {
public:
    typedef uint32_t key_t;

    PTree() { }
    PTree(Mempool* n_mempool, std::size_t n_leaf_size = 0)
	: mempool(n_mempool), root(NULL), leaf_size(n_leaf_size) {
	// ensure alignment
	leaf_size += sizeof (key_t) - 1;
	leaf_size -= leaf_size % sizeof (key_t);
    }

    bool contains(key_t k) const;
    void* find_or_insert(key_t k);
    void set_leaf_size(std::size_t n_leaf_size) { leaf_size = n_leaf_size; }
    void dump() const;

//private:
    // little-endian patricia trees
    struct Leaf {
	bool  is_leaf;		// 1 in leafs, 0 in branches
	key_t key;		// key
    };
    struct Node : public Leaf {
	key_t branch_bit;	// as a power of two
	Node* left;		// subtree where branchBit is 0
	Node* right;		// subtree where branchBit is 1

	void* data() {
	    assert(is_leaf);
	    return reinterpret_cast<unsigned char*>(this) + sizeof (PTree::Leaf);
	}
	void dump(int indent = 0) const;
    };

    Node* alloc_leaf(key_t c) {
	Node* leaf = static_cast<PTree::Node*>(mempool->alloc(sizeof (Leaf) + leaf_size));
	leaf->is_leaf = true;
	leaf->key = c;
	// Application specific hack: initialize first entry to a large FP number
	*(float*) (leaf->data()) = 1e8;
	return leaf;
    }
    Node* alloc_branch() {
	Node* branch = static_cast<PTree::Node*>(mempool->alloc(sizeof (Node)));
	branch->is_leaf = false;
	return branch;
    }

    Mempool* mempool;
    Node* root;
    std::size_t leaf_size;
};

#endif	// PTREE_HH
