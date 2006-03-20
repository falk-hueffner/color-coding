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

void Graph::read_graph(FILE* stream) {
    int i, j;
    vertex n1i, n2i;
    char n1[10], n2[10];
    name n1s, n2s;
    float w;

    number_nodes = 0;
    number_neighbours.clear();
    colors.clear();
    node_list1.clear();
    node_list2.clear();
    neighbours_list.clear();
    n_weights_list.clear();

    debug << endl << endl << "Read graph-file:" << endl;
    i = 0;
    while (feof(stream) == 0) {
	fscanf(stream, "%s %s %f", n1, n2, &w);
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

void Graph::read_start_nodes(FILE* stream) {
    char n1[10];
    vertex n1i;
    name n1s;
    Names_Vec start_vec;

    start_nodes.clear();

    //cout << endl << "Startvertices:  ";
    while (feof(stream) == 0) {
	fscanf(stream, "%s", n1);

	n1s = string(n1);
	if (node_list1.count(n1s) == 1) {
	    //cout << n1s << "  ";
	    n1i = node_list1.find(n1s)->second;
	    start_nodes.push_back(n1i);
	}
    }
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

    bool function_array = true;	//Set on true for function search_path_array

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

//-------------------------------------------------------------------------------
// Analyse Graph
// -> analyses the graph 
//    output of number of vertices, number of edges, distribution of vertex-degrees,
//    distribution of edge-probabilities
// 
// Inputparameter: none    
// Returnparamter: none
//-------------------------------------------------------------------------------

void Graph::analyse_graph() {
    int number_edges=0, max_degree=0, max_comp_size=0, size, num_comp = 0;
    int i, i2, i3, j, deg_list_length = 101;
    Numbers_Vec degree_list(deg_list_length, 0);
    Numbers_Vec probability_list(20, 0);
    Numbers_Vec comp_size_list(31, 0);
    Weights_Vec degree_prob_list(deg_list_length, 0);
    vertex vertex2,vertex3;
    int pottriads=0,triadscount=0;
    double clustercoeff;
    float prob, prob_sum;
    set<vertex> processed_vertices;
    
    for (i = 0; i < number_nodes; i++) {
        number_edges += number_neighbours[i];
	if (max_degree < number_neighbours[i]) max_degree = number_neighbours[i];
	if (number_neighbours[i] < deg_list_length) degree_list[number_neighbours[i]]++;

	if (processed_vertices.find(i) == processed_vertices.end()) {
	    num_comp++;
	    size=get_comp_size(i, processed_vertices);
	    if (size < 31) comp_size_list[size]++;
	    if (max_comp_size < size) max_comp_size = size;
	}
	
	pottriads += number_neighbours[i] * (number_neighbours[i] - 1) / 2;
	prob_sum = 0;
	for (j = 0; j < number_neighbours[i]; j++) {
	    prob = exp(-n_weights_list[i][j]);
	    prob_sum += prob;
	    if (i < neighbours_list[i][j]) {
	        probability_list[(int)(prob * 20)]++;
	    }
	    vertex2 = neighbours_list[i][j];
	    for (i2 = 0; i2 < number_neighbours[vertex2]; i2++) {
	        vertex3 = neighbours_list[vertex2][i2];
		for (i3 = 0; i3 < number_neighbours[vertex3]; i3++) {
		    if (neighbours_list[vertex3][i3]==i) triadscount++;
		}
	    }
	    
	}
	if (number_neighbours[i] < deg_list_length) 
	    degree_prob_list[number_neighbours[i]] += prob_sum / number_neighbours[i];
    }
    clustercoeff = ((double) triadscount / (2 * ((double) pottriads)));
    number_edges /= 2;

    cout << "Number of vertices: " << node_list1.size() << endl;
    cout << "Number of edges: " << number_edges << endl;
    cout << endl << "Cluster coefficient: " << clustercoeff << endl;

    cout << endl << "Degree   Number of vertices   Average edge probability" << endl;
    for (i = 1; i < deg_list_length; i++) {
        cout << i << "\t\t" << degree_list[i] << "\t\t" 
	     << degree_prob_list[i] / degree_list[i]<< endl;
    }

    cout << endl << "Average vertex-degree: " << (float) (number_edges * 2) / number_nodes << endl;
    cout << "Maximal vertex-degree: " << max_degree << endl;

    cout << endl << "Size   Number of componente" << endl;
    for (i = 1; i < 31; i++) {
        cout << i << "\t\t" << comp_size_list[i] << endl;
    }

    cout << endl << "Number of components: " << num_comp << endl;
    cout << "Average component size: " << (float) number_nodes / num_comp << endl;
    cout << "Maximal component size: " << max_comp_size << endl;
   
    cout << endl << "Distribution of the edge-probabilities" << endl;
    cout << "Probability  Number of edges" << endl;
    for (i = 0; i < 20; i++) {
        cout << i * 0.05 << " .. " << (i + 1) * 0.05 << "\t" <<  probability_list[i] << endl;
    }
}

//-------------------------------------------------------------------------------
// Get Comp Size
// -> Recursive function to compute component size 
// 
// Inputparameter: vertex
//		   v_set    ->  set of the vertices, which are allready processed
// Returnparamter: size of the componentpart
//-------------------------------------------------------------------------------

int Graph::get_comp_size(vertex v, set<vertex> &v_set) {
    if (v_set.find(v) != v_set.end()) return 0;
    int i,size = 1;
    v_set.insert(v);

    for (i = 0; i < number_neighbours[v]; i++) {
	size += get_comp_size(neighbours_list[v][i], v_set);
    }

    return size;
}
