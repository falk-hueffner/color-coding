#ifndef PATHSET_H
#define PATHSET_H

#include <set>

#include "types.h"

class PathSet {
public:
    PathSet(std::size_t n_max_size = 10, std::size_t n_max_common = 7)
	: max_size(n_max_size),
	  max_common(n_max_common) { }

    std::size_t size() { return entries.size(); }
    void add(const Path& p, weight w);
    weight best_weight();
    weight worst_weight();
    
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
    it end()   const { return entries.end();   }

private:
    std::size_t max_size, max_common;
    std::set<Entry> entries;
};

#endif // PATHSET_H
