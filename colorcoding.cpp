#include "graph.h"
using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 6) {
	cout << "Number of paramters incorrect!" << endl;
	cout << "The following parameters are expected:" << endl
	     << "1. file, which contains graph data" << endl
	     << "2. file, which contains the startvertices" << endl
	     << "3. lenght of the paths, which should be searched" << endl
	     << "4. number of the colors, which are used for coloring" << endl
	     << "5. number of iterations" << endl;
	return 0;
    }
    int path_length = atoi(argv[3]);
    int number_colors = atoi(argv[4]);
    int number_iterations = atoi(argv[5]);

    cout << "Entered paramters:" << endl
	 << "Graph: " << argv[1] << endl
	 << "Startvertices: " << argv[2] << endl
	 << "Pathlength: " << argv[3] << endl
	 << "Number of colors: " << argv[4] << endl
	 << "Number of iterations: " << argv[5] << endl;

    long sek;

    time(&sek);
    srand(sek);

    graph protein_network;

    protein_network.read_graph(argv[1]);
    protein_network.read_start_nodes(argv[2]);
    long zeit_start, zeit_ende;

    time(&zeit_start);

    protein_network.compute_results(number_colors, path_length, number_iterations, 100);

    time(&zeit_ende);

    protein_network.display_results(100);

    cout << endl << "Benötigte Zeit (in Sekunden): " << (zeit_ende - zeit_start) << endl;
    return 0;
}
