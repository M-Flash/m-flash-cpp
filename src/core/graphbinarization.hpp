#include <stddef.h>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/**
 * @file
 * @author  Hugo Gualdron<gualdron@usp.br>
 * @version 1.0
 *
 * @section LICENSE
 *
 * Copyright [2014] [Hugo Gualdron, Jose Fernando Rodrigues Junior, Duen Horng Chau / Sao Paulo University , Georgia Tech University]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

 * @section DESCRIPTION
 *
 * Graph conversion and parsing routines.
 *
 * NOTICE:
 * This file was adapted from GraphChi project https://github.com/GraphChi/graphchi-cpp/blob/master/src/preprocessing/conversions.hpp
 *
 */

#ifndef MFLASH_GRAPH_BINARIZATION_DEF
#define MFLASH_GRAPH_BINARIZATION_DEF

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <string>

#include "../../log/easylogging++.h"
#include "type.hpp"
#include "splitterbuffer.hpp"

namespace mflash {

/* Simple string to number parsers */
static void parse(int &x, const char * s);
static void parse(unsigned int &x, const char * s);
static void parse(float &x, const char * s);
static void parse(long &x, const char * s);
static void parse(char &x, const char * s);
static void parse(bool &x, const char * s);
static void parse(double &x, const char * s);
static void parse(short &x, const char * s);
static void FIXLINE(char * s);

static void parse(int &x, const char * s) {
	x = atoi(s);
}

static void parse(unsigned int &x, const char * s) {
	x = (unsigned int) strtoul(s, NULL, 10);
}

static void parse(float &x, const char * s) {
	x = (float) atof(s);
}

static void parse(long &x, const char * s) {
	x = atol(s);
}

static void parse(int64 &x, const char * s) {
	x = atol(s);
}


static void parse(char &x, const char * s) {
	x = s[0];
}

static void parse(bool &x, const char * s) {
	x = atoi(s) == 1;
}

static void parse(double &x, const char * s) {
	x = atof(s);
}

static void parse(short &x, const char * s) {
	x = (short) atoi(s);
}

// Catch all
template <typename T>
void parse(T &x, const char * s) {
	LOG(ERROR) << "You need to define parse<your-type>(your-type &x, const char *s) function"
	<< " to support parsing the edge value.";
	assert(false);
}

// Removes \n from the end of line
inline void FIXLINE(char * s) {
   int len = (int) strlen(s)-1;
   if(s[len] == '\n') s[len] = 0;
}
/**
 * Converts graph from an edge list format. Input may contain
 * value for the edges. Self-edges are ignored.
 */
template <typename EdgeDataType, class IdType>
void convert_edgelist(std::string graph_file, SplitterBuffer<IdType> & splitter ) {



	FILE * inf = fopen(graph_file.c_str(), "r");
	size_t bytesread = 0;
	size_t linenum = 0;
	if (inf == NULL) {
		LOG (ERROR) << "Could not load :" << graph_file << " error: " << strerror(errno);
	}

	LOG (INFO) << "Reading in edge list format!";
	char s[1024];

	IdType idType;
	EdgeDataType edge_type;

	//std::string graph, int64 edge_data_size, int64 ids_by_partitions, int64 buffer_size, int64 partitions, bool in_split){
	IdType from, to;
	EdgeDataType val;
	while(fgets(s, 1024, inf) != NULL) {

		linenum++;
		if (linenum % 10000000 == 0) {
			LOG(DEBUG) << "Read " << linenum << " lines, " << bytesread / 1024 / 1024. << " MB";
		}
		FIXLINE(s);
		bytesread += strlen(s);
		if (s[0] == '#') continue; // Comment
		if (s[0] == '%') continue;// Comment

		char delims[] = "\t, ";
		char * t;
		t = strtok(s, delims);
		if (t == NULL) {
			LOG ( ERROR ) << "Input file is not in right format. "
			<< "Expecting \"<from>\t<to>\". "
			<< "Current line: \"" << s << "\"\n";
			assert(false);
		}

		parse(from, t);
		t = strtok(NULL, delims);
		if (t == NULL) {
			LOG(ERROR) << "Input file is not in right format. "
			<< "Expecting \"<from>\t<to>\". "
			<< "Current line: \"" << s << "\"\n";
			assert(false);
		}
		parse(to, t);

		/* Check if has value */
		t = strtok(NULL, delims);



		if (t != NULL) {
			parse(val, (const char*) t);
		}
		if (from != to) {
			splitter.add(from, to, &val);
		}
	}
	fclose(inf);
}

/**
 * Converts a graph from adjacency list format. Edge values are not supported,
 * and each edge gets the default value for the type. Self-edges are ignored.
 */
template <typename EdgeDataType, class IdType>
void convert_adjlist(std::string graph_file, SplitterBuffer<IdType> & splitter ) {

	FILE * inf = fopen(graph_file.c_str(), "r");
	size_t bytesread = 0;
	size_t linenum = 0;
	if (inf == NULL) {
		LOG (ERROR) << "Could not load :" << graph_file << " error: " << strerror(errno);
	}

	assert(inf != NULL);
	LOG(INFO) << "Reading in adjacency list format!" << std::endl;

	int maxlen = 100000000;
	char * s = (char*) malloc(maxlen);


	char delims[] = " \t";
	size_t lastlog = 0;
	/*** PHASE 1 - count ***/
	IdType from, to;
	EdgeDataType val;
	while (fgets(s, maxlen, inf) != NULL) {

		linenum++;
		if (bytesread - lastlog >= 500000000) {
			LOG(INFO) << "Read " << linenum << " lines, "<< bytesread / 1024 / 1024. << " MB" << std::endl;
			lastlog = bytesread;
		}
		FIXLINE(s);
		bytesread += strlen(s);

		if (s[0] == '#')
			continue; // Comment
		if (s[0] == '%')
			continue; // Comment
		char * t = strtok(s, delims);

		//parse(from, t);
		from = atol(t);
		t = strtok(NULL, delims);
		if (t != NULL) {
			IdType num = atol(t);
			IdType i = 0;
			while ((t = strtok(NULL, delims)) != NULL) {
				to = atol(t);
				//parse(to, t);
				if (from != to) {
					splitter.add(from, to, (char*)&val);
				}
				i++;
			}
			if (num != i) {
				LOG(ERROR) << "Mismatch when reading adjacency list: "
						<< num << " != " << i << " s: " << std::string(s)
						<< " on line: " << linenum << std::endl;
				continue;
			}
		}
	}
	splitter.flush();
	free(s);
	fclose(inf);
}


}
 // end namespace

#endif

