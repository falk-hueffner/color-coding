#ifndef _GRAPH_
#define _GRAPH_

#include <iostream>
#include <map>
#include <vector>
#include <stack>
#include <string>
#include <stdio.h>
#include <time.h>
#include <math.h>
using namespace std;

typedef vector<int> VEC_INT;
typedef vector<VEC_INT> VEC_VEC_INT;
typedef vector<float> VEC_FLT;
typedef vector<VEC_FLT> VEC_VEC_FLT;
typedef map<string, int> MAP_STR_INT;
typedef MAP_STR_INT::iterator MAP_STR_INT_ITER;
typedef pair<string, int> PAIR_STR_INT;

typedef vector<string> VEC_STR;

typedef map<int, string> MAP_INT_STR;
typedef MAP_INT_STR::iterator MAP_INT_STR_ITER;
typedef pair<int, string> PAIR_INT_STR;

typedef pair<long, int> PAIR_LNG_INT;
typedef pair<int,float> PAIR_INT_FLT;
typedef pair<PAIR_LNG_INT,PAIR_INT_FLT> PAIR_PLI_PIF;
typedef map<PAIR_LNG_INT,PAIR_INT_FLT> MAP_PLI_PIF;
typedef MAP_PLI_PIF::iterator MAP_PLI_PIF_ITER;

typedef pair<float,PAIR_LNG_INT> PAIR_FLT_PLI;
typedef map<float,PAIR_LNG_INT> MAP_FLT_PLI;
typedef MAP_FLT_PLI::iterator MAP_FLT_PLI_ITER;

typedef pair<float,VEC_INT> PAIR_FLT_VIN;
typedef map<float,VEC_INT> MAP_FLT_VIN;
typedef MAP_FLT_VIN::iterator MAP_FLT_VIN_ITER;

class graph
{
 private:
  int number_nodes;
  MAP_STR_INT node_list1;
  MAP_INT_STR node_list2;

  VEC_INT number_neighbours;
  VEC_INT colors;

  VEC_VEC_INT neighbours_list;
  VEC_VEC_FLT n_weights_list;

  VEC_INT start_nodes;

  VEC_VEC_INT array_last_nodes;
  VEC_VEC_FLT array_weights;
  //int* array_last_nodes;
  //float* array_weights;

  MAP_FLT_VIN results;

 public:
  void read_graph(char* filename);
  void read_start_nodes(char* filename);
  void color_nodes(int number_colors);
  PAIR_FLT_VIN search_path(int path_length,float weight_border);
  PAIR_FLT_VIN search_path_array(int path_length,int number_colors,float weight_border);
  void compute_results(int number_colors, int path_length, int number_iterations, int number_results);
  void display_results(int number_results);
};

#endif // _GRAPH_
