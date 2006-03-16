#ifndef FIND_PATH_H
#define FIND_PATH_H

#include <set>

#include "graph.h"
#include "ptree.h"
#include "types.h"

class PathSet {
public:
    PathSet(std::size_t n_max_size = 10) : max_size(n_max_size) { }
    void add(const Path& p, weight w) {
	if (size() < max_size || w < worst_weight()) {
	    paths.insert(Entry(p, w));
	}
	if (size() > max_size)
	    paths.erase(*paths.rbegin());
    }
    weight worst_weight() {
	if (paths.empty())
	    return 1e10;
	else
	    return paths.rbegin()->w;
    }
    weight best_weight() {
	if (paths.empty())
	    return -1e10;
	else
	    return paths.begin()->w;
    }

    std::size_t size() { return paths.size(); }
    
    struct Entry {
	Entry(const Path& n_p, weight n_w) : p(n_p), w(n_w) {
	    if (p.front() > p.back())
		std::reverse(p.begin(), p.end());
	}
	Path p;
	weight w;
	bool operator<(const Entry& other) const {
	    if (w != other.w)
		return w < other.w;
	    return p < other.p;
	}
    };

    typedef std::set<Entry>::const_iterator it;

    it begin() const { return paths.begin(); }
    it end()   const { return paths.end();   }

private:
    std::size_t max_size;
    std::set<Entry> paths;
};

PathSet lightest_path(/*const*/ Graph& g, const VertexSet& start_nodes,
		      std::size_t path_length, std::size_t num_colors,
		      std::size_t num_trials, std::size_t num_paths);

#endif	// FIND_PATH_H
