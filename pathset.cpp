#include <algorithm>

#include "pathset.h"

template<typename T>
struct counter {
    counter() : count(0) { }
    std::size_t count;
    void push_back(const T&) { ++count; }
    typedef T value_type;
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

void PathSet::add(const std::vector<vertex_t>& p, weight_t weight) {
    if (is_full() && weight >= worst_weight())
	return;

    Entry entry(p, weight);
    std::vector<m_it> to_delete;
    for (m_it i = entries.begin(); i != entries.end(); ++i) {
	if (intersection_size(entry.path_set, i->path_set) > max_common) {
	    if (weight >= i->weight)
		return;
	    to_delete.push_back(i);
	}
    }

    for (std::size_t i = 0; i < to_delete.size(); ++i)
	entries.erase(to_delete[i]);

    entries.insert(entry);
    if (entries.size() > max_size + EXTRA_KEEP)
	entries.erase(*entries.rbegin());
    update_worst_weight();
}

void PathSet::update_worst_weight() {
    if (!is_full())
	m_worst_weight = 100;
    else {
	PathSet::it i = entries.begin();
	for (std::size_t j = 1; j < max_size; j++, i++) { }
	m_worst_weight = i->path_weight();
    }
}

weight_t PathSet::best_weight() const {
    if (entries.empty())
	return WEIGHT_MAX;
    else
	return entries.begin()->path_weight();
}

PathSet::Entry::Entry(const std::vector<vertex_t>& n_p, weight_t n_weight)
    : p(n_p), weight(n_weight) {
    for (std::size_t i = 0; i < p.size(); ++i)
	if (p[i] != DELETED_VERTEX)
	    path_set.insert(abs(p[i]));
}

bool PathSet::Entry::operator<(const Entry& other) const {
    if (weight != other.weight)
	return weight < other.weight;
    return p < other.p;
}
