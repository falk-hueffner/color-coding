#ifndef PATHSET_H
#define PATHSET_H

#include <set>

#include "types.h"

class PathSet {
public:
    PathSet(std::size_t n_max_size = 10) : max_size(n_max_size) { }

    std::size_t size() { return paths.size(); }
    void add(const Path& p, weight w);
    weight best_weight();
    weight worst_weight();
    
    struct Entry {
	Entry(const Path& n_p, weight n_w);	Path p;
	weight w;
	bool operator<(const Entry& other) const;
    };

    typedef std::set<Entry>::const_iterator it;

    it begin() const { return paths.begin(); }
    it end()   const { return paths.end();   }

private:
    std::size_t max_size;
    std::set<Entry> paths;
};

#endif // PATHSET_H
