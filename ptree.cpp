#include <iostream>

#include "ptree.h"

void* PTree::find_or_insert(key_t c) {
    if (!root) {
	root = alloc_leaf(c);
	return root->data();
    }
    Node* node = root;
    Node** pparent = &root;
    while (1) {
	if (node->is_leaf || !node->branch_matches(c)) {
	    if (node->is_leaf && node->key == c)
		return node->data();
	    PTree::Node* leaf = alloc_leaf(c);
	    PTree::Node* branch = alloc_branch(c, node, leaf);
	    *pparent = branch;
	    return leaf->data();
	}
	pparent = node->pchild(c);
	node = *pparent;
    }
}
