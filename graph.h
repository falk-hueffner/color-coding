#ifndef _GRAPH_
#define _GRAPH_

#include <iostream>
#include <map>
#include <vector>
#include <stack>
#include <set>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <cassert>

#include "types.h"

using namespace std;

typedef string name;

typedef vector<vertex_t> Vertices_Vec;
typedef vector<int> Colors_Vec;
typedef vector<int> Numbers_Vec;
typedef vector<float> Weights_Vec;
typedef vector<name> Names_Vec;

typedef vector<Vertices_Vec> Vertices_Array;
typedef vector<Weights_Vec> Weights_Array;

typedef map<string, vertex_t> Name_Number_Map;
typedef Name_Number_Map::iterator Name_Number_Map_Iter;
typedef pair<string, vertex_t> Name_Number_Pair;

typedef map<vertex_t, string> Number_Name_Map;
typedef Number_Name_Map::iterator Number_Name_Map_Iter;
typedef pair<vertex_t, string> Number_Name_Pair;

typedef pair<colorset_t, vertex_t> Colorset_Vertex_Pair;
typedef pair<vertex_t, weight_t> Vertex_Weight_Pair;

typedef pair<Colorset_Vertex_Pair, Vertex_Weight_Pair> Matrix_Entry_Pair;
typedef map<Colorset_Vertex_Pair, Vertex_Weight_Pair> Matrix_Entry_Map;
typedef Matrix_Entry_Map::iterator Matrix_Entry_Map_Iter;

typedef pair<weight_t, Colorset_Vertex_Pair> Weight_Pathvertex_Pair;
typedef map<weight_t, Colorset_Vertex_Pair> Weight_Pathvertex_Map;
typedef Weight_Pathvertex_Map::iterator Weight_Pathvertex_Map_Iter;

typedef pair<weight_t, Vertices_Vec> Weight_Path_Pair;
typedef map<weight_t, Vertices_Vec> Weight_Path_Map;
typedef Weight_Path_Map::iterator Weight_Path_Map_Iter;

class Graph {
private:
    int number_nodes;

    Name_Number_Map node_list1;
    Number_Name_Map node_list2;
    Numbers_Vec number_neighbours;
    Colors_Vec colors;
    Vertices_Array neighbours_list;
    Weights_Array n_weights_list;
    Vertices_Array array_last_nodes;
    Weights_Array array_weights;
    Vertices_Array array_iterations;
    Weight_Path_Map results;

public:
    Vertices_Vec start_nodes;
    std::size_t num_vertices() const { return number_nodes; }
    std::size_t num_edges() const {
	std::size_t num = 0;
	for (std::size_t v = 0; v < num_vertices(); ++v)
	    num += deg(v);
	return num / 2;
    }
    std::size_t deg(vertex_t v) const {
	assert(v < num_vertices());
	return number_neighbours[v];
    }
    std::size_t color(vertex_t v) const {
	assert(v < num_vertices());
	return colors[v];
    }
    colorset_t color_set(vertex_t v) const {
	assert(v < num_vertices());
	return static_cast<colorset_t>(1) << colors[v];
    }
    std::string vertex_name(vertex_t v) const {
	assert(v < num_vertices());
	return node_list2.find(v)->second;
    }
    vertex_t neighbor(vertex_t v, std::size_t i) const {
	assert(v < num_vertices());
	assert(i < deg(v));	
	return neighbours_list[v][i];
    }
    weight_t edge_weight(vertex_t v, std::size_t i) const {
	assert(v < num_vertices());
	assert(i < deg(v));	
	return n_weights_list[v][i];
    }
    const Vertices_Vec& startnodes() const { return start_nodes; }
    void connect(vertex_t u, vertex_t v, weight_t weight) {
	assert(u < num_vertices());
	assert(v < num_vertices());
	// FIXME check for double edges
	neighbours_list[u].push_back(v);
	neighbours_list[v].push_back(u);
	n_weights_list[u].push_back(weight);
	n_weights_list[v].push_back(weight);
	++number_neighbours[u];
	++number_neighbours[v];
    }
    void clear_edges() {
	for (std::size_t v = 0; v < num_vertices(); ++v) {
	    number_neighbours[v] = 0;
	    neighbours_list[v].clear();
	    n_weights_list[v].clear();
	}
    }
    const vertex_t* lookup_vertex(const std::string& n) const {
	std::map<std::string, vertex_t>::const_iterator pv = node_list1.find(n);
	if (pv != node_list1.end())
	    return &pv->second;
	else
	    return NULL;
    }

    void read_graph(FILE* stream);
    void read_start_nodes(FILE* stream);
    void color_nodes(int number_colors);
    Weight_Path_Pair search_path(int path_length, float weight_border);
    Weight_Path_Pair search_path_array(int path_length, int number_colors,
				       float weight_border, int iteration);
    void compute_results(int number_colors, int path_length,
			 int number_iterations, int number_results);
    void display_results(int number_results);
    void analyse_graph();
    int get_comp_size(vertex_t v, set<vertex_t> &v_set);
};

#endif				// _GRAPH_
