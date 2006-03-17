#include <math.h>
#include <time.h>

#ifdef __unix__
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "debug.h"
#include "graph.h"
#include "util.h"
#include "find_path.h"

std::size_t peak_mem_usage;

static void usage(FILE *stream) {
    fputs("colorcode: Find most probable path in a graph\n"
	  "  stdin      Input graph\n"
	  "  -i FILE    Read start vertices from FILE\n"
	  "  -v         Print progress to stderr\n"
	  "  -l K       Find paths of length K (default: 8)\n"
	  "  -c C       Use C colors (default: K)\n"
	  "  -n P       Find the best P paths (default: 100)\n"
	  "  -f F       Filter paths with more than F% in common (default: 70)\n"
	  "  -t T       T trials\n"
	  "  -p S       S% success probability (default: 99.9)\n"
	  "  -r [R]     Random seed R (or random if not given) (default: 1)\n"
	  "  -s         Print only statistics\n"
	  "  -h         Display this list of options\n"
	  , stream);
}

// returns ln(n!)
static double lfact(std::size_t n) {
    return lgamma(n + 1);
}

int main(int argc, char *argv[]) {
    const char *start_vertices_file = NULL;
    std::size_t path_length = 8;
    std::size_t num_colors = 0;
    std::size_t num_paths = 100;
    double filter = 70;
    std::size_t num_trials = 0;
    double success_prob = 99.9;
    bool stats_only = false;

    int c;
    while ((c = getopt(argc, argv, "i:vl:c:n:f:t:p:r::sh")) != -1) {
	switch (c) {
	case 'i': start_vertices_file = optarg; break;
	case 'v': info.turn_on(); break;
	case 'l': path_length = atoi(optarg); break;
	case 'c': num_colors = atoi(optarg); break;
	case 'n': num_paths = atoi(optarg); break;
	case 'f': filter = atof(optarg); break;
	case 't': num_trials = atoi(optarg); break;
	case 'p': success_prob = atof(optarg); break;
	case 'r':
	    if (optarg) {
		srand(atoi(optarg));
	    } else {
#ifdef __unix__
		struct timeval tv;
		gettimeofday(&tv, NULL);
		srand((unsigned(tv.tv_sec) * unsigned(getpid())) ^ unsigned(tv.tv_usec));
#else
		srand(time(NULL));
#endif
	    }
	    break;
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
    if (num_colors >= 31) {
	fprintf(stderr, "error: number of colors must be < 31\n");
	exit(1);
    }

    Graph g;
    g.read_graph(stdin);

    if (num_trials == 0) {
	std::size_t k = path_length;
	std::size_t x = num_colors - path_length;
	double colorful_prob = exp(lfact(k + x) - lfact(x) - k * log(double(k + x)));
	num_trials = std::size_t(ceil(log1p(-success_prob / 100) / log1p(-colorful_prob)));
	fprintf(stderr, "num_trials = %zd\n", num_trials);
    }

    if (start_vertices_file) {
	FILE* file = fopen(start_vertices_file, "r");
	g.read_start_nodes(file);
	fclose(file);
    } else {
	for (std::size_t i = 0; i < g.num_vertices(); ++i)
	    g.start_nodes.push_back(i);
    }

    std::size_t max_common = int(path_length * (filter / 100));

#if 1
    double start = timestamp();
    PathSet paths = lightest_path(g, g.startnodes(), path_length, num_colors, num_trials,
				  num_paths, max_common);
    double stop = timestamp();
    if (stats_only) {
	printf("%15.2f %6d %12.8f %12.8f\n", stop - start,
	       peak_mem_usage / 1024 / 1024, paths.best_weight(), paths.worst_weight());
    } else {
	for (PathSet::it i = paths.begin(); i != paths.end(); ++i) {
	    std::cout << i->path_weight();
	    for (std::size_t j = 0; j < i->path().size(); ++j)
		std::cout << ' ' << g.node_name(i->path()[j]);
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
