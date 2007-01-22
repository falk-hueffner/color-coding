#ifndef GRAPH_H
#define GRAPH_H

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "types.h"

class Graph {
public:
    Graph() { }
    Graph(std::istream& in, const bool type = true);

    std::size_t num_vertices() const { return m_neighbors.size(); }
    std::size_t num_edges() const;
    weight_t edge_weight(vertex_t u, vertex_t v) const;
    weight_t weight() const;
    std::size_t deg(vertex_t u) const {
	assert(u < num_vertices());
	return m_neighbors[u].size();
    }
    const vertex_t* lookup_vertex(const std::string& name) const;
    const std::string& vertex_name(vertex_t u) const;

    struct Edge {
	Edge() { }
	Edge(vertex_t n_neighbor, weight_t n_weight)
	    : neighbor(n_neighbor), weight(n_weight) { }
	vertex_t neighbor;
	weight_t weight;
    };

    typedef std::vector<Edge>::const_iterator neighbor_it;
    neighbor_it neighbors_begin(vertex_t u) const { return m_neighbors[u].begin(); }
    neighbor_it neighbors_end(vertex_t u) const { return m_neighbors[u].end(); }

    void clear_edges();
    Graph induced_subgraph(const std::vector<vertex_t>& vertices);
    void connect(vertex_t u, vertex_t v, weight_t weight);
    void set_weight(vertex_t u, vertex_t v, weight_t weight);

private:
    std::vector<std::vector<Edge> > m_neighbors;
    std::vector<std::string> m_vertex_names;
    std::map<std::string, vertex_t> m_vertex_numbers;
};

std::ostream& operator<<(std::ostream& out, const Graph& g);

#endif
