#include <stdlib.h>

#include "graph.h"
#include "util.h"

#include <math.h>
#ifdef _GUI_
	#include "gui/guiApp.h"
#endif

std::size_t Graph::num_edges() const {
    std::size_t num = 0;
    for (std::size_t u = 0; u < num_vertices(); ++u)
	num += deg(u);
    assert((num % 2) == 0);
    return num / 2;
}

weight_t Graph::edge_weight(vertex_t u, vertex_t v) const {
    for (Graph::neighbor_it n = neighbors_begin(u); n != neighbors_end(u); ++n)
	if (n->neighbor == v)
	    return n->weight;
    abort();
}

weight_t Graph::weight() const {
    weight_t weight = 0;
    for (vertex_t u = 0; u < num_vertices(); ++u)
	for (Graph::neighbor_it n = neighbors_begin(u); n != neighbors_end(u); ++n)
	    if (u < n->neighbor)
		weight += n->weight;
    return weight;
}

const std::string& Graph::vertex_name(vertex_t u) const {
     assert(u < num_vertices());
     return m_vertex_names[u];
}

const vertex_t* Graph::lookup_vertex(const std::string& n) const {
    std::map<std::string, vertex_t>::const_iterator pv = m_vertex_numbers.find(n);
    if (pv != m_vertex_numbers.end())
	return &pv->second;
    else
	return NULL;
}

void Graph::connect(vertex_t u, vertex_t v, weight_t weight) {
    assert(u < num_vertices());
    assert(v < num_vertices());
    // FIXME check for double edges
    m_neighbors[u].push_back(Edge(v, weight));
    m_neighbors[v].push_back(Edge(u, weight));
}

void Graph::set_weight(vertex_t u, vertex_t v, weight_t weight) {
    assert(u < num_vertices());
    assert(v < num_vertices());
    for (std::vector<Edge>::iterator n = m_neighbors[u].begin(); n != m_neighbors[u].end(); ++n) {
	if (n->neighbor == v) {
	    n->weight = weight;
	    break;
	}
    }
    for (std::vector<Edge>::iterator n = m_neighbors[v].begin(); n != m_neighbors[v].end(); ++n) {
	if (n->neighbor == u) {
	    n->weight = weight;
	    return;
	}
    }
    abort();
}

void Graph::clear_edges() {
    m_neighbors = std::vector<std::vector<Edge> >(num_vertices());
}

Graph::Graph(std::istream& in, bool type) {
    std::string line;
    std::size_t lineno = 0;
    while (std::getline(in, line)) {
	++lineno;
	std::string::size_type p = line.find('#');
	if (p != std::string::npos)
	    line = line.substr(0, p);
	std::vector<std::string> fields = split(line);
	if (fields.empty())
	    continue;
	if (fields.size() != 3) {
#ifndef _GUI_
	    std::cerr << "line " << lineno << ": error: syntax error\n";
	    exit(1);
#else
		wxString errormsg;
		errormsg << wxT("Error while loading graph file!\nLine ") << lineno << wxT(": syntax error\nLoading aborted.");
		(wxMessageDialog(wxGetApp().frame, errormsg)).ShowModal();
		wxGetApp().graphload_ok = false;
		return;
#endif
	}
	vertex_t v[2];
	for (std::size_t i = 0; i < 2; i++) {
	    if (const vertex_t* pv = lookup_vertex(fields[i])) {
		v[i] = *pv;
	    } else {
		v[i] = num_vertices();
		m_neighbors.resize(num_vertices() + 1);
		m_vertex_names.push_back(fields[i]);
		m_vertex_numbers[fields[i]] = v[i];
	    }
	}
	if (type) connect(v[0], v[1], atof(fields[2].c_str()));
	else connect(v[0], v[1], -log(atof(fields[2].c_str())));
    }
}

Graph Graph::induced_subgraph(const std::vector<vertex_t>& vertices) {
    Graph g = *this;
    g.clear_edges();
    std::vector<bool> is_contained(num_vertices());
    for (std::size_t i = 0; i < vertices.size(); ++i)
	is_contained[vertices[i]] = true;

    for (vertex_t u = 0; u < num_vertices(); ++u)
	if (is_contained[u])
	    for (Graph::neighbor_it n = neighbors_begin(u); n != neighbors_end(u); ++n)
		if (u < n->neighbor && is_contained[n->neighbor])
		    g.connect(u, n->neighbor, n->weight);

    return g;
}

std::ostream& operator<<(std::ostream& out, const Graph& g) {
    for (vertex_t u = 0; u < g.num_vertices(); ++u)
	for (Graph::neighbor_it n = g.neighbors_begin(u); n != g.neighbors_end(u); ++n)
	    if (u < n->neighbor)
		out << g.vertex_name(u) << ' '
		    << g.vertex_name(n->neighbor) << ' '
		    << n->weight << std::endl;

    return out;
}
