#include <iostream>

#include "ptree.h"

bool PTree::contains(key_t c) const {
    if (!root)
	return false;

    Node* node = root;
    while (!node->is_leaf) {
	if (((node->branch_bit - 1) & c) != node->key)
	    return false;
	else
	    node = c & node->branch_bit ? node->right : node->left;
    }
    return node->key == c;
}

void* PTree::find_or_insert(key_t c) {
    if (!root) {
	root = alloc_leaf(c);
	return root->data();
    }
    Node* node = root;
    Node** pparent = &root;
    while (1) {
	if (node->is_leaf || ((node->branch_bit - 1) & c) != node->key) {
	    if (node->is_leaf && node->key == c)
		return node->data();
	    PTree::Node* leaf = alloc_leaf(c);
	    PTree::Node* branch = alloc_branch();
	    PTree::key_t cmp = c ^ node->key;
	    branch->branch_bit = cmp & -cmp;  // extract trailing bit
	    branch->key = c & (branch->branch_bit - 1);
	    if (c & branch->branch_bit) {
		branch->left  = node;
		branch->right = leaf;
	    } else {
		branch->left  = leaf;
		branch->right = node;
	    }
	    *pparent = branch;
	    return leaf->data();
	}
	pparent = (c & node->branch_bit) ? &node->right : &node->left;
	node = *pparent;
    }
}

void PTree::dump() const {
    std::cerr << "{\n";
    if (root)
	root->dump();
    std::cerr << "}\n";
}

void dump_key(PTree::key_t k) {
    for (int i = sizeof (key_t) * 8 - 1; i >= 0; i--)
	std::cerr << ((k & (1u << i)) ? '1' : '0');
}

void PTree::Node::dump(int indent) const {
    for (int i = 0; i < indent; i++)
	std::cerr << ' ';
    std::cerr << (is_leaf ? '*' : '|');
    dump_key(key);
    if (!is_leaf) {
	std::cerr << ' ';
	dump_key(branch_bit);
	std::cerr << std::endl;
	left->dump(indent + 1);
	right->dump(indent + 1);
    } else
	std::cerr << std::endl;
}
