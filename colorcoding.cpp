#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include <math.h>
#include <stdio.h>
#include <time.h>

#ifdef __unix__
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "debug.h"
#include "find_path.h"
#include "graph.h"
#include "problem.h"
#include "util.h"
#include "trial.h"
#include "random_path.h"

std::size_t peak_mem_usage;

static void usage(std::ostream& out) {
    out << "colorcode: Find most probable path in a graph\n"
	   "  stdin      Input graph\n"
	   "  -i FILE    Read start vertices from FILE\n"
	   "  -e FILE    Read end vertices from FILE\n"
	   "  -q FILE    Read query path from FILE\n"
	   "  -m FILE    Read matching weights from FILE (only meaningful with -q)\n"
	   "             (default: 0 if vertex names match, 1 otherwise)\n"
	   "  -v         Print progress to stderr\n"
	   "  -M M       Maximal M insertions (default: 3)\n"
	   "  -N N       Maximal N deletions (default: 3)\n"
	   "  -C C       One insertion costs C (default: 0.0)\n"
	   "  -D D       One deletion costs D (default: 0.0)\n"
	   "  -l K       Find paths of length K (default: 8)\n"
	   "  -c C       Use C colors (default: auto)\n"
	   "  -n P       Find the best P paths (default: 100)\n"
	   "  -f F       Filter paths with more than F% in common (default: 70)\n"
	   "  -p S       S% success probability (default: 99.9)\n"
	   "  -t T       T trials (default: based on -p)\n"
	   "  -x X       X pre-heating trials (default: auto)\n"
	   "  -b X       Heuristic. 'n': none; 'e': min. edge weight;\n"
 	   "             x: dynamic programming, max. path length x (default: 2)\n"
	   "  -r [R]     Random seed R (or random if not given) (default: 1)\n"
	   "  -s         Print only statistics\n"
	   "  -R         Find random paths (ignoring weights)\n"
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

std::vector<std::string> read_vertex_name_file(const std::string& file) {
    std::ifstream in(file.c_str());
    if (!in) {
	std::cerr << "error: cannot open \"" << file << "\"\n";
	exit(1);
    }
    std::string line;
    std::size_t lineno = 0;
    std::vector<std::string> vertices;
    while (std::getline(in, line)) {
	++lineno;
	std::string::size_type p = line.find('#');
	if (p != std::string::npos)
	    line = line.substr(0, p);
	line = trim(line);
	std::vector<std::string> fields = split(line);
	for (std::size_t i = 0; i < fields.size(); ++i) {
	    if (std::find(vertices.begin(), vertices.end(), fields[i]) != vertices.end()) {
		std::cerr << file << ':' << lineno << ": error: duplicate vertex '"
			  << fields[i] << "'\n";
		exit(1);
	    } else {
		vertices.push_back(fields[i]);
	    }
	}
    }
    return vertices;
}

std::map<std::pair<std::string, std::string>, weight_t>
read_match_weights(const std::string& file) {
    std::ifstream in(file.c_str());
    if (!in) {
	std::cerr << "error: cannot open \"" << file << "\"\n";
	exit(1);
    }
    std::string line;
    std::size_t lineno = 0;
    std::map<std::pair<std::string, std::string>, weight_t> match_weights;
    while (std::getline(in, line)) {
	++lineno;
	std::string::size_type p = line.find('#');
	if (p != std::string::npos)
	    line = line.substr(0, p);
	std::vector<std::string> fields = split(line);
	if (fields.empty())
	    continue;
	if (fields.size() != 3) {
	    std::cerr << "line " << lineno << ": error: syntax error\n";
	    exit(1);
	}
	std::pair<std::string, std::string> match = std::make_pair(fields[0], fields[1]);
	double weight = atof(fields[2].c_str());

	if (match_weights.find(match) != match_weights.end()) {
	    if (0)
	    std::cerr << "line " << lineno << ": warning: duplicate entry for "
		      << fields[0] << " - " << fields[1] << std::endl;
	} else {
	    match_weights[match] = weight;
	}
    }
    return match_weights;
}

int main(int argc, char *argv[]) {
    std::string start_vertices_file, end_vertices_file, query_path_file, match_weights_file;
    std::size_t path_length = 8;
    std::size_t max_deletions = 3;
    std::size_t max_insertions = 3;
    weight_t insertion_cost = 0.0;
    weight_t deletion_cost = 0.0;
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
    bool random_paths = false;

    int c;
    while ((c = getopt(argc, argv, "yi:e:q:m:M:N:C:D:l:c:n:f:t:p:x:b:r::vsRh")) != -1) {
	switch (c) {
	case 'i': start_vertices_file = optarg; break;
	case 'e': end_vertices_file = optarg; break;
	case 'q': query_path_file = optarg; break;
	case 'm': match_weights_file = optarg; break;
	case 'M': max_insertions = atoi(optarg); break;
	case 'N': max_deletions = atoi(optarg); break;
	case 'C': insertion_cost = atof(optarg); break;
	case 'D': deletion_cost = atof(optarg); break;
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
	case 'R': random_paths = true; break;
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

    Graph g(std::cin);

    if (g.num_vertices() > MAX_VERTEX) {
	std::cerr << "error: graph has" << g.num_vertices() << " vertices, but only"
		  << MAX_VERTEX << " supported\n";
	exit(1);
    }

    if (random_paths) {
	for (std::size_t np = 0; np < num_paths; ++np) {
	    std::vector<vertex_t> p = random_path(g, path_length);
	    for (std::size_t i = 0; i < p.size(); ++i) {
		if (i)
		    std::cout << ' ';
		std::cout << g.vertex_name(p[i]);
	    }
	    std::cout << std::endl;
	}
	return 0;
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
 
    std::map<std::pair<std::string, std::string>, weight_t> match_weights;
    std::vector<std::vector<weight_t> > path_match_weights;
    if (match_weights_file != "")
	match_weights = read_match_weights(match_weights_file);
    if (query_path_file != "") {
	std::vector<std::string> query_vertices = read_vertex_name_file(query_path_file);
	path_match_weights.resize(query_vertices.size());
	for (std::size_t i = 0; i < query_vertices.size(); ++i) {
	    path_match_weights[i].resize(g.num_vertices());
	    weight_t best_match = WEIGHT_MAX;
	    for (std::size_t v = 0; v < g.num_vertices(); ++v) {
		std::map<std::pair<std::string, std::string>, weight_t>::const_iterator
		    it = match_weights.find(std::make_pair(query_vertices[i], g.vertex_name(v)));
		if (it != match_weights.end()) {
		    path_match_weights[i][v] = it->second;
		} else {
		    if (g.vertex_name(v) == query_vertices[i])
			path_match_weights[i][v] = 0;
		    else
			path_match_weights[i][v] = 100;
		    if (0)
		    std::cerr << "warning: no match weight for "
			      << query_vertices[i] << " - " << g.vertex_name(v)
			      << ", defaulting to " << path_match_weights[i][v] << std::endl;
		}
		if (path_match_weights[i][v] < best_match)
		    best_match = path_match_weights[i][v];
	    }
	    //std::cerr << "Best match for " << query_vertices[i] << ": " << best_match << std::endl;
	}
	path_length = query_vertices.size() + max_insertions;
    }

    std::size_t max_common = int(path_length * (filter / 100));

    problem.g = g;
    problem.is_start_vertex = is_start_vertex;
    problem.is_end_vertex = is_end_vertex;
    problem.match_weights = path_match_weights;
    problem.max_deletions = max_deletions;
    problem.insertion_cost = insertion_cost;
    problem.deletion_cost = deletion_cost;
    problem.max_insertions = max_insertions;
    problem.path_length = path_length;
    problem.num_preheat_trials = preheat_trials;
    problem.num_trials = num_trials;
    problem.success_prob = success_prob / 100;
    problem.num_colors = num_colors;

    double start = timestamp();
    PathSet paths = lightest_path(problem, num_paths, max_common, mode, max_lb_edges);
    double stop = timestamp();
    if (stats_only) {
	printf("%15.2f %6lu %12.8f %12.8f\n", stop - start,
	       (unsigned long) (peak_mem_usage / 1024 / 1024),
	       paths.best_weight(), paths.worst_weight());
    } else {
	for (PathSet::it i = paths.begin(); i != paths.end(); ++i) {
	    std::cout << i->path_weight();
	    for (std::size_t j = 0; j < i->path().size(); ++j) {
		small_vertex_t v = i->path()[j];
		if (v == DELETED_VERTEX)
		    std::cout << " -";
		else if (v < 0)
		    std::cout << " +" << g.vertex_name(-v);
		else
		    std::cout << ' ' << g.vertex_name(v);
	    }
	    std::cout << std::endl;
	    if (match_weights.size()) {
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
