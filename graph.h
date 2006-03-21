#ifndef GRAPH_H
#define GRAPH_H

#include <cassert>
#include <iostream>
#include <map>
#include <string>

#include "types.h"

class Graph {
public:
    Graph() { }
    Graph(std::istream& in);

    std::size_t num_vertices() const { return m_neighbors.size(); }
    std::size_t num_edges() const;
    std::size_t deg(vertex_t u) const {
	assert(u < num_vertices());
	return m_neighbors[u].size();
    }
    const vertex_t* lookup_vertex(const std::string& name) const;
    const std::string& vertex_name(vertex_t u);

    struct Edge {
	Edge() { }
	Edge(vertex_t n_neighbor, weight_t n_w) : neighbor(n_neighbor), w(n_w) { }
	vertex_t neighbor;
	weight_t w;
    };

    vertex_t neighbor(vertex_t u, std::size_t i) const { return m_neighbors[u][i].neighbor; }
    weight_t edge_weight(vertex_t u, std::size_t i) const { return m_neighbors[u][i].w; }

    void clear_edges();
    void connect(vertex_t u, vertex_t v, weight_t weight);

private:
    std::vector<std::vector<Edge> > m_neighbors;
    std::vector<std::string> m_vertex_names;
    std::map<std::string, vertex_t> m_vertex_numbers;
};

#endif
