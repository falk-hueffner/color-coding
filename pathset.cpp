#include <algorithm>

#include "pathset.h"

template<typename T>
struct counter {
    counter() : count(0) { }
    std::size_t count;
    void push_back(const T&) { ++count; }
    typedef const T& const_reference;
};

template<typename T>
std::size_t intersection_size(std::set<T> s1, std::set<T> s2) {
    counter<T> count;
    set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), std::back_inserter(count));
    return count.count;
}

void PathSet::add(const Path& p, weight w) {
    if (size() >= max_size && w >= worst_weight())
	return;

    Entry entry(p, w);
    for (it i = begin(); i != end(); ) {
	if (intersection_size(entry.path_set, i->path_set) > max_common) {
	    if (w < i->w)
		entries.erase(i++);
	    else
		return;
	} else {
	    ++i;
	}
    }
    entries.insert(entry);
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

PathSet::Entry::Entry(const Path& n_p, weight n_w)
    : p(n_p), w(n_w), path_set(p.begin(), p.end()) { }

bool PathSet::Entry::operator<(const Entry& other) const {
    if (w != other.w)
	return w < other.w;
    return p < other.p;
}
