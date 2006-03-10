#ifndef _GRAPH_
#define _GRAPH_

#include <iostream>
#include <map>
#include <vector>
#include <stack>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
using namespace std;

typedef uint32_t colorset;
typedef uint32_t vertex;
typedef float weight;
typedef string name;

typedef vector<int> Vertices_Vec;
typedef vector<int> Colors_Vec;
typedef vector<int> Numbers_Vec;
typedef vector<float> Weights_Vec;
typedef vector<name> Names_Vec;

typedef vector<Vertices_Vec> Vertices_Array;
typedef vector<Weights_Vec> Weights_Array;

typedef map<string, vertex> Name_Number_Map;
typedef Name_Number_Map::iterator Name_Number_Map_Iter;
typedef pair<string, vertex> Name_Number_Pair;

typedef map<vertex, string> Number_Name_Map;
typedef Number_Name_Map::iterator Number_Name_Map_Iter;
typedef pair<vertex, string> Number_Name_Pair;

typedef pair<colorset, vertex> Colorset_Vertex_Pair;
typedef pair<vertex,weight> Vertex_Weight_Pair;

typedef pair<Colorset_Vertex_Pair,Vertex_Weight_Pair> Matrix_Entry_Pair;
typedef map<Colorset_Vertex_Pair,Vertex_Weight_Pair> Matrix_Entry_Map;
typedef Matrix_Entry_Map::iterator Matrix_Entry_Map_Iter;

typedef pair<weight,Colorset_Vertex_Pair> Weight_Pathvertex_Pair;
typedef map<weight,Colorset_Vertex_Pair> Weight_Pathvertex_Map;
typedef Weight_Pathvertex_Map::iterator Weight_Pathvertex_Map_Iter;

typedef pair<weight,Vertices_Vec> Weight_Path_Pair;
typedef map<weight,Vertices_Vec> Weight_Path_Map;
typedef Weight_Path_Map::iterator Weight_Path_Map_Iter;

class graph
{
 private:
  int number_nodes;

  Name_Number_Map node_list1;
  Number_Name_Map node_list2;
  Numbers_Vec number_neighbours;
  Colors_Vec colors;
  Vertices_Array neighbours_list;
  Weights_Array n_weights_list;
  Vertices_Vec start_nodes;
  Vertices_Array array_last_nodes;
  Weights_Array array_weights;
  Weight_Path_Map results;

 public:
  void read_graph(char* filename);
  void read_start_nodes(char* filename);
  void color_nodes(int number_colors);
  Weight_Path_Pair search_path(int path_length,float weight_border);
  Weight_Path_Pair search_path_array(int path_length,int number_colors,float weight_border);
  void compute_results(int number_colors, int path_length, int number_iterations, int number_results);
  void display_results(int number_results);
};

#endif // _GRAPH_
