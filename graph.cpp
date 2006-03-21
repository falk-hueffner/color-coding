#include "graph.h"
#include "util.h"

std::size_t Graph::num_edges() const {
    std::size_t num = 0;
    for (std::size_t u = 0; u < num_vertices(); ++u)
	num += deg(u);
    assert((num % 2) == 0);
    return num / 2;
}

const std::string& Graph::vertex_name(vertex_t u) {
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

void Graph::clear_edges() {
    m_neighbors = std::vector<std::vector<Edge> >(num_vertices());
}

Graph::Graph(std::istream& in) {
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
	    std::cerr << "line " << lineno << ": error: syntax error\n";
	    exit(1);
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
	connect(v[0], v[1], atof(fields[2].c_str()));
    }
}
