#ifndef PATHSET_H
#define PATHSET_H

#include <set>
#include <vector>

#include "types.h"

class PathSet {
public:
    PathSet(std::size_t n_max_size, std::size_t n_max_common)
	: max_size(n_max_size),
	  max_common(n_max_common) {
	update_worst_weight();
    }

    std::size_t size() const { return std::min(entries.size(), max_size); }
    bool is_full() const { return entries.size() >= max_size; }
    void add(const std::vector<vertex_t>& p, weight_t weight);
    weight_t best_weight() const;
    weight_t worst_weight() const { return m_worst_weight; }
    
    class Entry {
	friend class PathSet;
    public:
	Entry(const std::vector<vertex_t>& n_p, weight_t n_weight);
	bool operator<(const Entry& other) const;
	const std::vector<vertex_t>& path() const { return p; }
	weight_t path_weight() const { return weight; }
    private:
	std::vector<vertex_t> p;
	weight_t weight;
	std::set<vertex_t> path_set;
    };

    typedef std::set<Entry>::const_iterator it;
    typedef std::set<Entry>::iterator m_it;

    it begin() const { return entries.begin(); }
    it end()   const;

private:
    void update_worst_weight();
    std::size_t max_size, max_common;
    std::set<Entry> entries;
    weight_t m_worst_weight;
    enum { EXTRA_KEEP = 20 };
};

#endif // PATHSET_H
