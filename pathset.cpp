#include <algorithm>

#include "pathset.h"

void PathSet::add(const Path& p, weight w) {
    if (size() < max_size || w < worst_weight()) {
	paths.insert(Entry(p, w));
    }
    if (size() > max_size)
	paths.erase(*paths.rbegin());
}

weight PathSet::worst_weight() {
    if (paths.empty())
	return 1e10;
    else
	return paths.rbegin()->w;
}

weight PathSet::best_weight() {
    if (paths.empty())
	return -1e10;
    else
	return paths.begin()->w;
}

PathSet::Entry::Entry(const Path& n_p, weight n_w) : p(n_p), w(n_w) {
    if (p.front() > p.back())
	std::reverse(p.begin(), p.end());
}

bool PathSet::Entry::operator<(const Entry& other) const {
    if (w != other.w)
	return w < other.w;
    return p < other.p;
}
