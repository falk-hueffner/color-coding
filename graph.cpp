#include "debug.h"
#include "graph.h"

using namespace std;

colorset ins[] = {
    1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7, 1 << 8, 1 << 9, 1 << 10,
    1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15, 1 << 16, 1 << 17, 1 << 18, 1 << 19,
    1 << 20, 1 << 21, 1 << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28,
    1 << 29, 1 << 30, 1 << 31    
};
colorset del[] = {
    ~1, ~(1 << 1), ~(1 << 2), ~(1 << 3), ~(1 << 4), ~(1 << 5), ~(1 << 6), ~(1 << 7),
    ~(1 << 8), ~(1 << 9), ~(1 << 10), ~(1 << 11), ~(1 << 12), ~(1 << 13), ~(1 << 14),
    ~(1 << 15), ~(1 << 16), ~(1 << 17), ~(1 << 18), ~(1 << 19), ~(1 << 20), ~(1 << 21),
    ~(1 << 22), ~(1 << 23), ~(1 << 24), ~(1 << 25), ~(1 << 26), ~(1 << 27), ~(1 << 28),
    ~(1 << 29), ~(1 << 30), ~(1 << 31)
};

//-------------------------------------------------------------------------------
// Read_Graph
// -> reads a graph out of a file
// 
// Inputparameter: filename -> name of the file, which contains the graph
// Returnparamter: none
//-------------------------------------------------------------------------------

void Graph::read_graph(char *filename) {
    FILE *datei;
    int i, j;
    vertex n1i, n2i;
    char n1[10], n2[10];
    name n1s, n2s;
    weight w;

    number_nodes = 0;
    number_neighbours.clear();
    colors.clear();
    node_list1.clear();
    node_list2.clear();
    neighbours_list.clear();
    n_weights_list.clear();

    datei = fopen(filename, "r");
    if (!datei) {
	cerr << "Error while reading graph-file!\n";
	exit(1);
    }

    debug << endl << endl << "Read graph-file:" << endl;
    i = 0;
    while (feof(datei) == 0) {
	fscanf(datei, "%s %s %f", n1, n2, &w);
	//printf("%s\t%s\t%f\n", n1,n2,w);

	n1s = string(n1);
	n2s = string(n2);

	if (node_list1.count(n1s) == 0) {
	    node_list1.insert(Name_Number_Pair(n1s, i));
	    node_list2.insert(Number_Name_Pair(i, n1s));
	    neighbours_list.push_back(Vertices_Vec());
	    n_weights_list.push_back(Weights_Vec());
	    n1i = i;
	    i++;
	} else
	    n1i = node_list1.find(n1s)->second;

	if (node_list1.count(n2s) == 0) {
	    node_list1.insert(Name_Number_Pair(n2s, i));
	    node_list2.insert(Number_Name_Pair(i, n2s));
	    neighbours_list.push_back(Vertices_Vec());
	    n_weights_list.push_back(Weights_Vec());
	    n2i = i;
	    i++;
	} else
	    n2i = node_list1.find(n2s)->second;

	(neighbours_list[n1i]).push_back(n2i);
	(neighbours_list[n2i]).push_back(n1i);
	(n_weights_list[n1i]).push_back(-log(w));
	(n_weights_list[n2i]).push_back(-log(w));

    }

    fclose(datei);

    j = 0;
    number_nodes = node_list1.size();
    number_neighbours = Numbers_Vec(number_nodes, 0);
    colors = Colors_Vec(number_nodes, 0);
    for (i = 0; i < node_list1.size(); i++) {
	number_neighbours[i] = (neighbours_list[i]).size();
	j += number_neighbours[i];
    }
    debug << "Number of vertices: " << number_nodes << endl << "Number of edges: "
	  << j / 2 << endl;
}

//-------------------------------------------------------------------------------
// Read_Start_Nodes
// -> reads the startnodes out of a file
// 
// Inputparameter: filename -> name of the file, which contains the startnodes
// Returnparamter: none
//-------------------------------------------------------------------------------

void Graph::read_start_nodes(char *filename) {
    char n1[10];
    vertex n1i;
    name n1s;
    Names_Vec start_vec;

    FILE *datei;

    start_nodes.clear();

    datei = fopen(filename, "r");
    if (!datei) {
	cerr << "Error while reading start-node-file!\n";
	exit(1);
    }

    //cout << endl << "Startvertices:  ";
    while (feof(datei) == 0) {
	fscanf(datei, "%s", n1);

	n1s = string(n1);
	if (node_list1.count(n1s) == 1) {
	    //cout << n1s << "  ";
	    n1i = node_list1.find(n1s)->second;
	    start_nodes.push_back(n1i);
	}
    }
    fclose(datei);

}

//-------------------------------------------------------------------------------
// Color nodes
// -> colors the vertices of a graph with different colors by random
// 
// Inputparameter: number_colors -> number of the different colors, which are used for coloring
// Returnparamter: none
//-------------------------------------------------------------------------------

void Graph::color_nodes(int number_colors)
{
    int i;

    for (i = 0; i < number_nodes; i++) {
	colors[i] = rand() % number_colors;
    }
}

//-------------------------------------------------------------------------------
// Search Path
// -> searchs for the best path on the current coloring
//    Data structure: Map
// 
// Inputparameter: path_length   -> length of the path to search for
//                 weight_border -> abortcriteria for path-search
// Returnparamter: Pair(weight of the best path, vertices of the best path)
//-------------------------------------------------------------------------------

Weight_Path_Pair Graph::search_path(int path_length, float weight_border)
{
    Matrix_Entry_Map paths[path_length];
    Matrix_Entry_Map_Iter listpos, listpos2;

    Weight_Pathvertex_Map result;
    Weight_Pathvertex_Map_Iter respos;

    Colorset_Vertex_Pair entry;

    int i, j, nn;
    vertex node, actn;
    weight weight_sum;

    for (i = 0; i < start_nodes.size(); i++) {
	paths[0].insert(Matrix_Entry_Pair(Colorset_Vertex_Pair(ins[colors[start_nodes[i]]],
							       start_nodes[i]),
					  Vertex_Weight_Pair(start_nodes[i], 0.0)));
    }

    for (i = 0; i < path_length - 1; i++) {
	//cout<<"i: "<<i<<endl;
	for (listpos = paths[i].begin(); listpos != paths[i].end(); listpos++) {
	    node = (listpos->first).second;
	    nn = number_neighbours[node];
	    //cout<<"node: "<<node<<"  anz nachbarn:"<<nn<<endl;
	    for (j = 0; j < nn; j++) {
		actn = neighbours_list[node][j];
		if ((ins[colors[actn]] & ((listpos->first).first)) != 0)
		    continue;
		entry = Colorset_Vertex_Pair(ins[colors[actn]] | ((listpos->first).first), actn);
		weight_sum = n_weights_list[node][j] + ((listpos->second).second);
		if (weight_sum > weight_border)
		    continue;
		if (paths[i + 1].count(entry) == 0) {
		    paths[i + 1].insert(Matrix_Entry_Pair(entry,
							  Vertex_Weight_Pair(node, weight_sum)));
		} else {
		    listpos2 = paths[i + 1].find(entry);
		    if (((listpos2->second).second) > weight_sum)
			(listpos2->second) = Vertex_Weight_Pair(node, weight_sum);
		}
	    }
	}
    }

    for (listpos = paths[path_length - 1].begin();
	 listpos != paths[path_length - 1].end(); listpos++) {
	result. insert(Weight_Pathvertex_Pair((listpos->second).second, listpos->first));
    }

    Vertices_Vec res_path(path_length, 0);
    Weight_Path_Pair compl_res;

    if (result.begin() != result.end()) {
	colorset act_color = (result.begin()->second).first;
	vertex act_node = (result.begin()->second).second;
	vertex last_node = ((paths[path_length - 1].find(result.begin()->second))->second).first;

	res_path[path_length - 1] = act_node;

	for (i = path_length - 2; i >= 0; i--) {
	    listpos = paths[i].find(Colorset_Vertex_Pair(act_color &= del[colors[act_node]],
							 last_node));
	    act_node = last_node;
	    last_node = (listpos->second).first;
	    res_path[i] = act_node;
	}
	compl_res = Weight_Path_Pair(result.begin()->first, res_path);
    } else {
	compl_res = Weight_Path_Pair(INT_MAX, res_path);
    }

    return compl_res;
}

//-------------------------------------------------------------------------------
// Search Path Array
// -> searchs for the best path on the current coloring.
//    Data structures: Array, Stack
// 
// Inputparameter: path_length   -> length of the path to search for
//                 number_colors -> number of the different colors
//                 weight_border -> abortcriteria for path-search
// Returnparamter: Pair(weight of the best path, vertices of the best path)
//-------------------------------------------------------------------------------

Weight_Path_Pair Graph::search_path_array(int path_length, int number_colors,
					  float weight_border, int iteration)
{
    Weight_Pathvertex_Map result;
    Weight_Pathvertex_Map_Iter respos;

    Colorset_Vertex_Pair entry;

    stack<Colorset_Vertex_Pair> nodes_to_do[2];

    int i, j, nn;
    colorset pathcolor, new_pathcolor;
    vertex node, act_node;
    bool act_stack = false;
    weight weight_sum;

//   array_weights.assign((int)pow(2,number_colors),VEC_FLT(number_nodes,INT_MAX));
//   array_last_nodes.assign((int)pow(2,number_colors),VEC_INT(number_nodes,INT_MAX));

    for (i = 0; i < start_nodes.size(); i++) {
	pathcolor = ins[colors[start_nodes[i]]];
	act_node = start_nodes[i];
	array_weights[pathcolor][act_node] = 0;
	nodes_to_do[0].push(Colorset_Vertex_Pair(pathcolor, act_node));
    }

    for (i = 0; i < path_length - 1; i++) {
	while (!nodes_to_do[act_stack].empty()) {
	    pathcolor = (nodes_to_do[act_stack].top()).first;
	    node = (nodes_to_do[act_stack].top()).second;
	    nodes_to_do[act_stack].pop();
	    nn = number_neighbours[node];
	    for (j = 0; j < nn; j++) {
		act_node = neighbours_list[node][j];
		if ((ins[colors[act_node]] & pathcolor) != 0)
		    continue;
		new_pathcolor = ins[colors[act_node]] | pathcolor;
		entry = Colorset_Vertex_Pair(new_pathcolor, act_node);
		weight_sum = n_weights_list[node][j] + array_weights[pathcolor][node];
		if (weight_sum > weight_border)
		    continue;
		if (array_iterations[new_pathcolor][act_node] == iteration) {
		    if (array_weights[new_pathcolor][act_node] >= weight_sum) {
			array_weights[new_pathcolor][act_node] = weight_sum;
			array_last_nodes[new_pathcolor][act_node] = node;
			nodes_to_do[!act_stack].push(entry);
		    }
		} else {
		    array_weights[new_pathcolor][act_node] = weight_sum;
		    array_last_nodes[new_pathcolor][act_node] = node;
		    nodes_to_do[!act_stack].push(entry);
		    array_iterations[new_pathcolor][act_node] = iteration;
		}

	    }
	}
	act_stack = !act_stack;
    }

    while (!nodes_to_do[act_stack].empty()) {
	pathcolor = (nodes_to_do[act_stack].top()).first;
	node = (nodes_to_do[act_stack].top()).second;

	result.insert(Weight_Pathvertex_Pair(array_weights[pathcolor][node],
					     nodes_to_do[act_stack].top()));
	nodes_to_do[act_stack].pop();
    }

    Vertices_Vec res_path(path_length, 0);
    Weight_Path_Pair compl_res;

    if (result.begin() != result.end()) {
	colorset act_color = (result.begin()->second).first;

	act_node = (result.begin()->second).second;
	vertex last_node = array_last_nodes[act_color][act_node];

	res_path[path_length - 1] = act_node;

	for (i = path_length - 2; i >= 0; i--) {
	    res_path[i] = last_node;
	    last_node = array_last_nodes[act_color &= del[colors[act_node]]][last_node];
	    act_node = res_path[i];
	}
	compl_res = Weight_Path_Pair(result.begin()->first, res_path);
    } else {
	compl_res = Weight_Path_Pair(INT_MAX, res_path);
    }

    return compl_res;
}

//-------------------------------------------------------------------------------
// Compute Results
// -> searchs for the best paths on different colorings.
//    the results are stored in the member variable results 
// 
// Inputparameter: number_colors     -> number of the different colors
//                 path_length       -> length of the path to search for
//                 number_iterations -> number of different colorings
//                 number_results    -> number of results, which are to be computed
// Returnparamter: none
//-------------------------------------------------------------------------------

void Graph::compute_results(int number_colors, int path_length,
			    int number_iterations, int number_results) {
    int i;

    Weight_Path_Pair res;

    weight weight_border = INT_MAX;

    results.clear();

    bool function_array = false;	//Set on true for function search_path_array

    //Set on false for function search_path

    if (function_array == true) {
	//------ This block is needed, when working with arrays -------
	debug << "Start initialising Arrays" << endl;
	array_weights.assign(1 << number_colors, Weights_Vec(number_nodes, INT_MAX));
	debug << "Array1 ready" << endl;
	array_last_nodes.assign(1 << number_colors, Vertices_Vec(number_nodes, INT_MAX));
	debug << "Array2 ready" << endl;
	array_iterations.assign(1 << number_colors, Vertices_Vec(number_nodes, INT_MAX));
	debug << "Array3 ready" << endl;
	//------ end of block -----------------------------------------
    }

    for (i = 0; i < number_iterations; i++) {
	color_nodes(number_colors);
	if (function_array == false)
	    // Maps
	    res = search_path(path_length, weight_border);
	else
	     //Arrays and stacks
	    res = search_path_array(path_length, number_colors, weight_border, i);
	results.insert(res);
    }
}

//-------------------------------------------------------------------------------
// Display Results
// -> display the results of a search, which are stored in the member variable 
//    results, on the screen 
// 
// Inputparameter: number_results    -> number of results, which are to be displayed
// Returnparamter: none
//-------------------------------------------------------------------------------

void Graph::display_results(int number_results) {
    Weight_Path_Map_Iter result_pos;
    int i, j;

    cout << "Best " << number_results << " of " << results.size()
	 << " found paths:" << endl;
    result_pos = results.begin();
    for (j = 0; ((result_pos != results.end()) && (j < number_results));
	 result_pos++, j++) {
	cout << endl << result_pos->first << "\t  ";
	for (i = 0; i < (result_pos->second).size(); i++) {
	    cout << node_list2.find((result_pos->second)[i])->second << " ";
	}
    }
    cout << flush;
}
