CXX	= g++
# gcc-arch can be obtained from http://people.debian.org/~falk/gcc-arch
CXXFLAGS= -O3 $(shell CC=$(CC) gcc-arch) -g -W -Wall -Wno-sign-compare -pipe
# disable internal consistency checking for some speedup
#CXXFLAGS  += -DNDEBUG

INCLUDES=
LDPATH	=
LIBS	=

OBJS	= \
	bounds.o	\
	colorcoding.o	\
	debug.o		\
	find_path.o	\
	graph.o		\
	mst.o		\
	pathset.o	\
	ptree.o		\
	trial.o		\
	util.o

CXXOMPILE = $(CXX) $(CXXFLAGS) $(INCLUDES)
CXXLINK	= $(CXX) $(CXXFLAGS) $(LDPATH) $(LIBS)

all: depend colorcoding generate_graph analyse_graph optimal-x

colorcoding: $(OBJS)
	$(CXXLINK) $^ -o $@

generate_graph: generate_graph.o
	$(CXXLINK) $^ -o $@

analyse_graph: analyse_graph.o graph2.o debug.o
	$(CXXLINK) $^ -o $@

optimal-x: optimal-x.c
	$(CXXLINK) $^ -o $@

%.o: %.cpp
	$(CXXOMPILE) -c $<

clean:
	rm -f *.o core gmon.out
	rm -f colorcoding generate_graph

realclean: clean
	rm -f *~ *.bak

.depend: depend

depend:
	$(CXX) $(CXXFLAGS) -MM *.cpp > .depend

include .depend
