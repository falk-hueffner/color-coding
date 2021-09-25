CXX	= c++
CXXFLAGS= -O3 -g

VERSION = 1.1.2

LIBS	= -lm

OBJS	= bounds.o colorcoding.o debug.o find_path.o graph.o pathset.o \
	  ptree.o qpath_trial.o random_path.o trial.o util.o

all: colorcoding

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

colorcoding: $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o $@

clean:
	rm -f *.o core gmon.out
	rm -f colorcoding

realclean: clean
	rm -f *~ *.bak

DIST_FILES = \
	LICENSE		\
	Makefile	\
	README		\
	bounds.cpp	\
	bounds.h	\
	colorcoding.cpp \
	colored_graph.h \
	debug.cpp	\
	debug.h		\
	example.graph	\
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
	qpath_trial.cpp \
	qpath_trial.h	\
	random_path.cpp \
	random_path.h	\
	trial.cpp	\
	trial.h		\
	types.h		\
	util.cpp	\
	util.h		\

dist:
	rm -rf colorcoding-$(VERSION)
	mkdir colorcoding-$(VERSION)
	cp $(DIST_FILES) colorcoding-$(VERSION)
	(cd colorcoding-$(VERSION) && touch .depend && make depend)
	GZIP=--best tar -cvvzf colorcoding-$(VERSION).tar.gz colorcoding-$(VERSION)

.depend: depend

depend:
	$(CXX) $(CXXFLAGS) -MM $(OBJS:.o=.cpp) > .depend

include .depend
