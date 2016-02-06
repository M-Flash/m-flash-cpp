# Makefile
# 
# Replace GRAPHCHI_DIR and BOOST_DIR with your proper library directories.

MFLASH_DIR = -I.
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

FILE_PATH = $(basename $(shell echo $@ | sed -E 's/^(debug|build)\/[^/]+\///g'))
FILENAME = $(notdir $(FILE_PATH))
SOURCE_FILE = $(addsuffix .cpp,$(FILE_PATH)) 


all: apps
apps: build/example_apps/lancsosso build/example_apps/pagerank build/example_apps/wcc

echo/%: 
	echo bin/$(@F)

clean:
	@rm -rf bin/*

debug/%: $(SOURCE_FILE) $(HEADERS) 
	@mkdir -p bin/$(dir $(FILE_PATH))
	$(CPP) $(CPPFLAGS-DEBUG) -I. $(FILE_PATH).cpp -o bin/$(FILE_PATH) $(LINKERFLAGS)

build/%: $(SOURCE_FILE) $(HEADERS) 
	@mkdir -p bin/$(dir $(FILE_PATH))
	$(CPP) $(CPPFLAGS) -I. $(FILE_PATH).cpp -o bin/$(FILE_PATH) $(LINKERFLAGS)
