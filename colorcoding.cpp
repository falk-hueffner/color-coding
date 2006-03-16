#include <math.h>

#include "debug.h"
#include "graph.h"
#include "util.h"
#include "find_path.h"

using namespace std;

static void usage(FILE *stream) {
    fputs("colorcode: Find most probable path in a graph\n"
	  "  stdin  	Input graph\n"
	  "  -i FILE	Read start vertices from FILE\n"
	  "  -v	Print progress to stderr\n"
	  "  -l K	Find paths of length K (default: 8)\n"
	  "  -c C	Use C colors (default: K)\n"
	  "  -n P	Find the best P paths (default: 10)\n"
	  "  -t T	T trials\n"
	  "  -p S	S\% success probability (default: 99.9)\n"
	  "  -r [R]	Random seed R (or random if not given) (default: 0)\n"
	  "  -s	Print only statistics\n"
	  "  -h	Display this list of options\n"
	  , stream);
}

static double lfact(int n) {
    return lgamma(n + 1.0);
}

int main(int argc, char *argv[]) {
    const char *start_vertices_file = NULL;
    std::size_t path_length = 8;
    std::size_t num_colors = 0;
    std::size_t num_paths = 10;
    std::size_t num_trials = 0;
    double success_prob = 99.9;
    bool stats_only = false;
    
    int c;
    while ((c = getopt(argc, argv, "i:vl:c:n:t:p:r::sh")) != -1) {
	switch (c) {
	case 'i': start_vertices_file = optarg; break;
	case 'v': info.turn_on(); break;
	case 'l': path_length = atoi(optarg); break;
	case 'c': num_colors = atoi(optarg); break;
	case 'n': num_paths = atoi(optarg); break;
	case 't': num_trials = atoi(optarg); break;
	case 'p': success_prob = atof(optarg); break;
	case 'r':
	    if (optarg)
		srand(atoi(optarg));
	    else
		srand(time(NULL));
	    num_paths = atoi(optarg); break;
	case 's': stats_only = true; break;
	case 'h': usage(stdout); exit(0); break;
	default:  usage(stderr); exit(1); break;
	}
    }
    if (optind < argc) {
	usage(stderr);
	exit(1);
    }

    if (path_length >= 31) {
	fprintf(stderr, "error: path length must be < 31\n");
	exit(1);
    }
    if (num_colors == 0)
	num_colors = path_length;
    if (num_colors < path_length) {
	fprintf(stderr, "error: need at least as many colors as the path length\n");
	exit(1);
    }

    Graph g;
    g.read_graph(stdin);

    if (num_trials == 0) {
	double epsilon = 1 - success_prob / 100;
	int n = g.num_vertices();
	int k = path_length;
	int x = num_colors - path_length;
	double colorful_prob;
	colorful_prob = exp(lfact(k + x) - lfact(x) - k * log(double(k + x)));
	num_trials = int(log(epsilon) / log1p(-colorful_prob) + 1);
    }

    if (start_vertices_file) {
	FILE* file = fopen(start_vertices_file, "r");
	g.read_start_nodes(file);
	fclose(file);
    } else {
	for (std::size_t i = 0; i < g.num_vertices(); ++i)
	    g.start_nodes.push_back(i);
    }

#if 1
    double start = timestamp();
    PathSet paths = lightest_path(g, g.startnodes(), path_length, num_colors, num_trials,
				  num_paths);
    double stop = timestamp();
    if (stats_only) {
	printf("%15.2f %12.8f %12.8f\n", stop - start, paths.best_weight(), paths.worst_weight());
    } else {
	for (PathSet::it i = paths.begin(); i != paths.end(); ++i) {
	    std::cout << i->w;
	    for (std::size_t j = 0; j < i->p.size(); ++j)
		std::cout << ' ' << g.node_name(i->p[j]);
	    std::cout << std::endl;
	}
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
