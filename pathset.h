#ifndef PATHSET_H
#define PATHSET_H

#include <set>

#include "types.h"

class PathSet {
public:
    PathSet(std::size_t n_max_size, std::size_t n_max_common)
	: max_size(n_max_size),
	  max_common(n_max_common) { }

    std::size_t size() const { return std::min(entries.size(), max_size); }
    bool is_full() const { return entries.size() >= max_size; }
    void add(const Path& p, weight w);
    weight best_weight() const;
    weight worst_weight() const;
    
    class Entry {
	friend class PathSet;
    public:
	Entry(const Path& n_p, weight n_w);
	bool operator<(const Entry& other) const;
	const Path& path() const { return p; }
	weight path_weight() const { return w; }
    private:
	Path p;
	weight w;
	std::set<vertex> path_set;
    };

    typedef std::set<Entry>::const_iterator it;

    it begin() const { return entries.begin(); }
    it end()   const;

private:
    std::size_t max_size, max_common;
    std::set<Entry> entries;
    enum { EXTRA_KEEP = 500 };
};

#endif // PATHSET_H
