# Makefile
# 
# Replace GRAPHCHI_DIR and BOOST_DIR with your proper library directories.

MFLASH_DIR = -I/hugo/doutorado/code/mflash-cpp/
BOOST_DIR = -I/usr/lib64/ -I/usr/lib/x86_64-linux-gnu/
BOOST_LIBRARIES = -lboost_filesystem -lboost_system 
SO_LIBRARIES = 

INCLUDES = $(MFLASH_DIR) $(BOOST_DIR)

CPP = g++
CPPFLAGS = -O3 $(INCLUDES) -Wall -Wno-strict-aliasing -std=c++11 
CPPFLAGS-DEBUG =  -g $(INCLUDES) -Wall -Wno-strict-aliasing -std=c++11
LINKERFLAGS =  $(BOOST_LIBRARIES) $(SO_LIBRARIES)
DEBUGFLAGS = -g -ggdb $(INCFLAGS)
HEADERS=$(shell find . -name '*.hpp')

EXTENSION = $(subst ., ,$(@F))
SELECTED_FILE = $(word 1, $(EXTENSION))
PROJECT = /mflash-cpp
TMPV = $@
echo/%: 
	echo bin/$(@F)

src/%: src/%.cpp $(HEADERS)
	@mkdir -p bin/
	$(CPP) $(CPPFLAGS) -I. $@.cpp -o bin/$(@F) $(LINKERFLAGS)

test/%: test/%.cpp $(HEADERS)
	@mkdir -p bin/
	$(CPP) $(CPPFLAGS) -I. $@.cpp -o bin/$(prefixfile) $(LINKERFLAGS)

selected_test/%: 
	@mkdir -p bin/
	$(CPP) $(CPPFLAGS) -I. test/${SELECTED_FILE}.cpp -o bin/${SELECTED_FILE} $(LINKERFLAGS)
	
selected_test_debug/%: 
	@mkdir -p bin/
	$(CPP) $(CPPFLAGS-DEBUG) -I. test/${SELECTED_FILE}.cpp -o bin/${SELECTED_FILE} $(LINKERFLAGS)

all: test/$(DEFAULT_APP).cpp $(HEADERS)
	@mkdir -p bin/
	$(CPP) $(CPPFLAGS) -I. test/$(DEFAULT_APP).cpp -o bin/$(DEFAULT_APP) $(LINKERFLAGS)

	