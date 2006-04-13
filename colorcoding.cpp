#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include <math.h>
#include <time.h>

#ifdef __unix__
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "debug.h"
#include "find_path.h"
#include "graph.h"
#include "mst.h"
#include "problem.h"
#include "util.h"
#include "trial.h"

std::size_t peak_mem_usage;

static void usage(std::ostream& out) {
    out << "colorcode: Find most probable path in a graph\n"
	   "  stdin      Input graph\n"
	   "  -y	 find trees instead of paths\n"
	   "  -i FILE    Read start vertices from FILE\n"
	   "  -e FILE    Read end vertices from FILE\n"
	   "  -v         Print progress to stderr\n"
	   "  -l K       Find paths of length K (default: 8)\n"
	   "  -c C       Use C colors (default: K)\n"
	   "  -n P       Find the best P paths (default: 100)\n"
	   "  -f F       Filter paths with more than F% in common (default: 70)\n"
	   "  -t T       T trials\n"
	   "  -p S       S% success probability (default: 99.9)\n"
	   "  -x X       X pre-heating trials (default: 50)\n"
	   "  -b X       Heuristic. 'n': none; 'e': min. edge weight;\n"
 	   "             x: dynamic programming, max. path length x (default: 2)\n"
	   "  -r [R]     Random seed R (or random if not given) (default: 1)\n"
	   "  -s         Print only statistics\n"
	   "  -h         Display this list of options\n";
}

std::set<vertex_t> read_vertex_file(const std::string& file, const Graph& g) {
    std::ifstream in(file.c_str());
    if (!in) {
	std::cerr << "error: cannot open \"" << file << "\"\n";
	exit(1);
    }
    std::string line;
    std::size_t lineno = 0;
    std::set<vertex_t> vertices;
    while (std::getline(in, line)) {
	++lineno;
	std::string::size_type p = line.find('#');
	if (p != std::string::npos)
	    line = line.substr(0, p);
	line = trim(line);
	if (line.empty())
	    continue;
	if (line.find_first_of(WHITESPACE) != std::string::npos) {
	    std::cerr << file << ':' << lineno << ": error: syntax error\n";
	    exit(1);
	}
	const vertex_t* pv = g.lookup_vertex(line);
	if (!pv) {
	    std::cerr << file << ':' << lineno << ": warning: unknown vertex '"
		      << line << "'\n";	    
	} else if (vertices.find(*pv) != vertices.end()) {
	    std::cerr << file << ':' << lineno << ": warning: ignoring duplicate vertex '"
		      << line << "'\n";
	} else {
	    vertices.insert(*pv);
	}
    }
    return vertices;
}

int main(int argc, char *argv[]) {
    bool find_trees = false;
    std::string start_vertices_file, end_vertices_file;
    std::size_t path_length = 8;
    std::size_t num_colors = 0;
    std::size_t num_paths = 100;
    double filter = 70;
    std::size_t num_trials = 0;
    double success_prob = 99.9;
    std::size_t preheat_trials = 50;
    bool stats_only = false;
    Problem problem;
    Bounds::Mode mode = Bounds::DYNPROG;
    std::size_t max_lb_edges = 2;

    int c;
    while ((c = getopt(argc, argv, "yi:e:l:c:n:f:t:p:x:b:r::vsh")) != -1) {
	switch (c) {
	case 'y': find_trees = true; break;
	case 'i': start_vertices_file = optarg; break;
	case 'e': end_vertices_file = optarg; break;
	case 'l': path_length = atoi(optarg); break;
	case 'c': num_colors = atoi(optarg); problem.auto_colors = false; break;
	case 'n': num_paths = atoi(optarg); break;
	case 'f': filter = atof(optarg); break;
	case 't': num_trials = atoi(optarg); problem.auto_trials = false; break;
	case 'p': success_prob = atof(optarg); break;
	case 'x': preheat_trials = atoi(optarg); problem.auto_preheat_trials = false; break;
	case 'b':
	    if (optarg == std::string("n")) {
		mode = Bounds::NONE;
	    } else if (optarg == std::string("e")) {
		mode = Bounds::EDGE_WEIGHT;
	    } else {
		mode = Bounds::DYNPROG;
		max_lb_edges = atoi(optarg);
	    }
	    break;
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
	case 'v': info.turn_on(); break;
	case 's': stats_only = true; break;
	case 'h': usage(std::cout); exit(0); break;
	default:  usage(std::cerr); exit(1); break;
	}
    }
    if (optind < argc) {
	usage(std::cerr);
	exit(1);
    }

    if (path_length > MAX_COLORS) {
	std::cerr << "error: path length must be <= " << MAX_COLORS << std::endl;
	exit(1);
    }
    if (num_colors == 0)
	num_colors = path_length;

    if (num_colors < path_length) {
	std::cerr << "error: need at least as many colors as the path length\n";
	exit(1);
    }
    if (num_colors > MAX_COLORS) {
	std::cerr << "error: number of colors must be <= " << MAX_COLORS << std::endl;
	exit(1);
    }
    if (find_trees && end_vertices_file != "") {
	std::cerr << "error: end vertices not supported when looking for trees\n";
	exit(1);
    }

    Graph g(std::cin);

    if (g.num_vertices() > MAX_VERTEX) {
	std::cerr << "error: graph has" << g.num_vertices() << " vertices, but only"
		  << MAX_VERTEX << " supported\n";
	exit(1);
    }

    if (num_trials == 0)
	num_trials = trials_for_prob(path_length, num_colors, success_prob / 100);

    std::vector<bool> is_start_vertex(g.num_vertices());
    if (start_vertices_file != "") {
	std::set<vertex_t> start_vertices = read_vertex_file(start_vertices_file, g);
	for (std::set<vertex_t>::const_iterator it = start_vertices.begin();
	     it != start_vertices.end(); ++it)
	    is_start_vertex[*it] = true;
    } else {
	for (std::size_t i = 0; i < g.num_vertices(); ++i)
	    is_start_vertex[i] = true;
    }

    std::vector<bool> is_end_vertex(g.num_vertices());
    if (end_vertices_file != "") {
	std::set<vertex_t> end_vertices = read_vertex_file(end_vertices_file, g);
	for (std::set<vertex_t>::const_iterator it = end_vertices.begin();
	     it != end_vertices.end(); ++it)
	    is_end_vertex[*it] = true;
    } else {
	for (std::size_t i = 0; i < g.num_vertices(); ++i)
	    is_end_vertex[i] = true;
    }

    std::size_t max_common = int(path_length * (filter / 100));

    problem.g = g;
    problem.is_start_vertex = is_start_vertex;
    problem.is_end_vertex = is_end_vertex;
    problem.find_trees = find_trees;
    problem.path_length = path_length;
    problem.num_preheat_trials = preheat_trials;
    problem.num_trials = num_trials;
    problem.success_prob = success_prob / 100;
    problem.num_colors = num_colors;

    double start = timestamp();
    PathSet paths = lightest_path(problem, num_paths, max_common, mode, max_lb_edges);
    double stop = timestamp();
    if (stats_only) {
	printf("%15.2f %6d %12.8f %12.8f\n", stop - start,
	       peak_mem_usage / 1024 / 1024, paths.best_weight(), paths.worst_weight());
    } else {
	for (PathSet::it i = paths.begin(); i != paths.end(); ++i) {
	    std::cout << i->path_weight();
	    for (std::size_t j = 0; j < i->path().size(); ++j)
		std::cout << ' ' << g.vertex_name(i->path()[j]);
	    std::cout << std::endl;
	    if (problem.find_trees) {
		Graph h = g.induced_subgraph(i->path());
		Graph t = mst(h);
		if (fabs(t.weight() - i->path_weight()) > 1e-6) {
		    std::cerr << "internal error: MST weight is " << t.weight() << std::endl
			      << t << std::endl;
		    exit(1);
		}
	    } else {
		weight_t weight = 0;
		for (std::size_t j = 0; j < i->path().size() - 1; ++j)
		    weight += g.edge_weight(i->path()[j], i->path()[j + 1]);
		if (fabs(weight - i->path_weight()) > 1e-6) {
		    std::cerr << "internal error: path weight is " << weight << std::endl;
		    exit(1);
		}
	    }
	}
    }

    return 0;
}
