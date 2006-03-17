#include <algorithm>

#include "pathset.h"

void PathSet::add(const Path& p, weight w) {
    if (size() < max_size || w < worst_weight()) {
	entries.insert(Entry(p, w));
    }
    if (size() > max_size)
	entries.erase(*entries.rbegin());
}

weight PathSet::worst_weight() {
    if (entries.empty())
	return 1e10;
    else
	return entries.rbegin()->path_weight();
}

weight PathSet::best_weight() {
    if (entries.empty())
	return -1e10;
    else
	return entries.begin()->path_weight();
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
