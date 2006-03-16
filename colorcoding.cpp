#include "debug.h"
#include "graph.h"
#include "util.h"
#include "find_path.h"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 6) {
	cerr << "Number of paramters incorrect!" << endl
	     << "The following parameters are expected:" << endl
	     << "1. file, which contains graph data" << endl
	     << "2. file, which contains the startvertices" << endl
	     << "3. lenght of the paths, which should be searched" << endl
	     << "4. number of the colors, which are used for coloring" << endl
	     << "5. number of iterations" << endl;
	return 1;
    }
    int path_length = atoi(argv[3]);
    int number_colors = atoi(argv[4]);
    int number_iterations = atoi(argv[5]);

    debug << "Entered paramters:" << endl
	  << "Graph: " << argv[1] << endl
	  << "Startvertices: " << argv[2] << endl
	  << "Pathlength: " << argv[3] << endl
	  << "Number of colors: " << argv[4] << endl
	  << "Number of iterations: " << argv[5] << endl;

#if 0
    long sek;
    time(&sek);
    srand(sek);
#endif

    Graph protein_network;

    protein_network.read_graph(fopen(argv[1], "r"));
    protein_network.read_start_nodes(fopen(argv[2], "r"));

    double start = timestamp(), stop;

#if 1
    PathSet paths = lightest_path(protein_network, protein_network.startnodes(),
				  path_length, number_colors, number_iterations);
    for (PathSet::it i = paths.begin(); i != paths.end(); ++i) {
	std::cout << i->w;
	for (std::size_t j = 0; j < i->p.size(); ++j)
	    std::cout << ' ' << protein_network.node_name(i->p[j]);
	std::cout << std::endl;
    }
#else
    protein_network.compute_results(number_colors, path_length, number_iterations, 100);

    stop = timestamp();

    protein_network.display_results(10);

    debug.turn_on();
    
    debug << endl << "Benötigte Zeit (in Sekunden): " << (stop - start) << endl;
#endif
    return 0;
}
