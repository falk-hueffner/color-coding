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

PathSet::it PathSet::end()   const {
    if (!is_full()) {
	return entries.end();
    } else {
	PathSet::it i = entries.begin();
	for (std::size_t j = 0; j < max_size; j++, i++) { }
	return i;
    }
}

void PathSet::add(const std::vector<vertex>& p, weight w) {
    if (is_full() && w >= worst_weight())
	return;

    Entry entry(p, w);
    for (it i = entries.begin(); i != entries.end(); ++i)
	if (intersection_size(entry.path_set, i->path_set) > max_common)
	    if (w >= i->w)
		return;

    for (it i = entries.begin(); i != entries.end(); )
	if (intersection_size(entry.path_set, i->path_set) > max_common)
	    entries.erase(i++);
	else 
	    ++i;

    entries.insert(entry);
    if (entries.size() > max_size + EXTRA_KEEP)
	entries.erase(*entries.rbegin());
}

weight PathSet::worst_weight() const {
    if (!is_full())
	return 1e10;
    else {
	PathSet::it i = entries.begin();
	for (std::size_t j = 1; j < max_size; j++, i++) { }
	return i->path_weight();
    }
}

weight PathSet::best_weight() const {
    if (entries.empty())
	return 1e10;
    else
	return entries.begin()->path_weight();
}

PathSet::Entry::Entry(const std::vector<vertex>& n_p, weight n_w)
    : p(n_p), w(n_w), path_set(p.begin(), p.end()) { }

bool PathSet::Entry::operator<(const Entry& other) const {
    if (w != other.w)
	return w < other.w;
    return p < other.p;
}
