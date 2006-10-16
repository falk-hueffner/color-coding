#include <iostream>

#include "ptree.h"

void* PTree::find_or_insert(key_t c, Mempool& mempool, std::size_t leaf_size) {
    if (!root) {
	root = alloc_leaf(c, mempool, leaf_size);
	return root->data();
    }
    Node* node = root;
    Node** pparent = &root;
    while (1) {
	if (node->is_leaf || !node->branch_matches(c)) {
	    if (node->is_leaf && node->key == c)
		return node->data();
	    PTree::Node* leaf = alloc_leaf(c, mempool, leaf_size);
	    PTree::Node* branch = alloc_branch(c, node, leaf, mempool);
	    *pparent = branch;
	    return leaf->data();
	}
	pparent = node->pchild(c);
	node = *pparent;
    }
}
