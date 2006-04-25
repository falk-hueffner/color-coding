#ifndef COLORED_GRAPH_H
#define COLORED_GRAPH_H

#include <stdlib.h>

#include "types.h"
#include "graph.h"

class ColoredGraph : public Graph {
public:
    ColoredGraph(const Graph& g) : Graph(g), m_colors(num_vertices()) { }
    void color_randomly(std::size_t num_colors) {
	for (std::size_t i = 0; i < num_vertices(); ++i)
	    m_colors[i] = rand() % num_colors;
    }
    color_t color(vertex_t u) const {
	assert(u < num_vertices());
	return m_colors[u];
    }
    colorset_t color_set(vertex_t u) const {
	assert(u < num_vertices());
	return static_cast<colorset_t>(1) << m_colors[u];
    }

private:
    std::vector<color_t> m_colors;
};

#endif
