SHELL = /bin/sh

CC = gcc
CXX = g++
LD = g++

CCFLAGS = -c -g -Wall
CCDEFS = -DDEBUG -D__USE_POSIX2 -D_THREAD_SAFE -D_REENTRANT
CXXFLAGS = -c -g -Wall
INCLUDES = -I. -I../../common -I/usr/include/g++-2/ -I/usr/lib/qt4/mkspecs/default -I/usr/lib/qt4/include

LDFLAGS = #-E -s -Bstatic
LIBDIRS = 
LIBS = $(LIBDIR) -lnetsnmp -L/usr/lib/qt4/lib -lQtCore

SOFLAGS = -fPIC

HEADERS = $(wildcard *.h)
CCSOURCES = $(wildcard *.c)
CXXSOURCES = $(wildcard *.cpp)
SOURCES = $(CCSOURCES) $(CXXSOURCES)
OBJ = $(CXXSOURCES:%.cpp=%.o) $(CCSOURCES:%.c=%.o)

TARG = 
SOTG = ntpMIB.so
LIBTG =
BIGOBJTG =

.c.o:
	$(CC) $(CCFLAGS) $(CCDEFS) $(SOFLAGS) $(INCLUDES) -o $@ $<

.cpp.o:
	$(CXX) $(CXXFLAGS) $(CCDEFS) $(SOFLAGS) $(INCLUDES) -o $@ $<

$(SOTG): $(OBJ)
	$(LD) $(LIBS) $(LDFLAGS) -shared -o $@ $^

clean:
	@for t in $(TARG) $(SOTG) $(LIBTG) $(BIGOBJTG) $(OBJ); do \
		if [ -f $$t ] ; then \
			printf "Removing $$t\n" ;\
			rm -f $$t ; \
		fi ; \
	done
