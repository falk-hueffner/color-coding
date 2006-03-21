#ifndef COLORED_GRAPH_H
#define COLORED_GRAPH_H

#include "types.h"
#include "graph.h"

class ColoredGraph : public Graph {
public:
    ColoredGraph(const Graph& g) : Graph(g), m_colors(num_vertices()) { }
    void color_randomly(unsigned num_colors) {
	for (std::size_t i = 0; i < num_vertices(); ++i)
	    m_colors[i] = rand() % num_colors;
    }
    color_t color(vertex u) const { return m_colors[u]; }
    colorset color_set(vertex u) const { return colorset(1) << m_colors[u]; }    

private:
    std::vector<color_t> m_colors;
};

#endif
