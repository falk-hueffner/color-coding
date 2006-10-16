VERSION = 1.0
CXX	= /home/mit/theinf1/hueffner/bin/g++-4.2
# gcc-arch can be obtained from http://people.debian.org/~falk/gcc-arch
CXXFLAGS= -O3 $(shell CC=$(CC) /home/mit/theinf1/hueffner/bin/gcc-arch) -g -W -Wall -Wno-sign-compare -pipe
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
	qpath_trial.o	\
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

DIST_FILES = \
	README		\
	bounds.cpp	\
	bounds.h	\
	colorcoding.cpp	\
	colored_graph.h	\
	debug.cpp	\
	debug.h		\
	find_path.cpp	\
	find_path.h	\
	graph.cpp	\
	graph.h		\
	mempool.h	\
	pathset.cpp	\
	pathset.h	\
	problem.h	\
	ptree.cpp	\
	ptree.h		\
	trial.cpp	\
	trial.h		\
	types.h		\
	util.cpp	\
	util.h

dist:
	rm -rf colorcoding-$(VERSION)
	mkdir colorcoding-$(VERSION)
	cp $(DIST_FILES) colorcoding-$(VERSION)
	cp Makefile.dist colorcoding-$(VERSION)/Makefile
	cp netz.txt colorcoding-$(VERSION)/example.graph
	cp /usr/share/common-licenses/GPL colorcoding-$(VERSION)/LICENSE
	(cd colorcoding-$(VERSION) && touch .depend && make depend)
	GZIP=--best tar -cvvzf colorcoding-$(VERSION).tar.gz colorcoding-$(VERSION)

.depend: depend

depend:
	$(CXX) $(CXXFLAGS) -MM *.cpp > .depend

include .depend
