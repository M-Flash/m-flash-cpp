;
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

#include "../log/easylogging++.h"
#include "type.hpp"

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

/**
 * Converts graph from an edge list format. Input may contain
 * value for the edges. Self-edges are ignored.
 */
template <typename EdgeDataType>
void convert_edgelist(std::string graph_file, ElementIdSize id_size) {

	FILE * inf = fopen(graph_file.c_str(), "r");
	size_t bytesread = 0;
	size_t linenum = 0;
	if (inf == NULL) {
		LOG (ERROR) << "Could not load :" << graph_file << " error: " << strerror(errno);
	}
	//assert(inf != NULL);

	bool simple_id = id_size == SIMPLE;

	LOG (INFO) << "Reading in edge list format!";
	char s[1024];
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
			//assert(false);
		}
		int64 from = atol(t);
		t = strtok(NULL, delims);
		if (t == NULL) {
			LOG(ERROR) << "Input file is not in right format. "
			<< "Expecting \"<from>\t<to>\". "
			<< "Current line: \"" << s << "\"\n";
			//assert(false);
		}
		int64 to = atoi(t);

		/* Check if has value */
		t = strtok(NULL, delims);


		EdgeDataType val;
		if (t != NULL) {
			parse(val, (const char*) t);
		}
		if (from != to) {
			if (t != NULL) {
				sharderobj.preprocessing_add_edge(from, to, val);
			} else {
				sharderobj.preprocessing_add_edge(from, to);
			}
		}
	}
	fclose(inf);
}

/**
 * Converts a graph from adjacency list format. Edge values are not supported,
 * and each edge gets the default value for the type. Self-edges are ignored.
 */
template <typename EdgeDataType>
void convert_adjlist(std::string inputfile,
		sharder<EdgeDataType, FinalEdgeDataType> &sharderobj) {
	FILE * inf = fopen(inputfile.c_str(), "r");
	if (inf == NULL) {
		logstream(LOG_FATAL) << "Could not load :" << inputfile << " error: "
				<< strerror(errno) << std::endl;
	}
	assert(inf != NULL);
	logstream(LOG_INFO) << "Reading in adjacency list format!" << std::endl;

	int maxlen = 100000000;
	char * s = (char*) malloc(maxlen);

	size_t bytesread = 0;

	char delims[] = " \t";
	size_t linenum = 0;
	size_t lastlog = 0;
	/*** PHASE 1 - count ***/
	while (fgets(s, maxlen, inf) != NULL) {
		linenum++;
		if (bytesread - lastlog >= 500000000) {
			logstream(LOG_DEBUG) << "Read " << linenum << " lines, "
					<< bytesread / 1024 / 1024. << " MB" << std::endl;
			lastlog = bytesread;
		}
		FIXLINE(s);
		bytesread += strlen(s);

		if (s[0] == '#')
			continue; // Comment
		if (s[0] == '%')
			continue; // Comment
		char * t = strtok(s, delims);
		vid_t from = atoi(t);
		t = strtok(NULL, delims);
		if (t != NULL) {
			vid_t num = atoi(t);
			vid_t i = 0;
			while ((t = strtok(NULL, delims)) != NULL) {
				vid_t to = atoi(t);
				if (from != to) {
					sharderobj.preprocessing_add_edge(from, to, EdgeDataType());
				}
				i++;
			}
			if (num != i) {
				logstream(LOG_ERROR) << "Mismatch when reading adjacency list: "
						<< num << " != " << i << " s: " << std::string(s)
						<< " on line: " << linenum << std::endl;
				continue;
			}
		}
	}
	free(s);
	fclose(inf);
}

/**
 * Extract a vector of node indices from a line in the file.
 *
 * @param[in]   line        line from input file containing node indices
 * @param[out]  adjacencies     node indices extracted from line
 */
static std::vector<vid_t> VARIABLE_IS_NOT_USED parseLine(std::string line);
static std::vector<vid_t> VARIABLE_IS_NOT_USED parseLine(std::string line) {

	std::stringstream stream(line);
	std::string token;
	char delim = ' ';
	std::vector<vid_t> adjacencies;

	// split string and push adjacent nodes
	while (std::getline(stream, token, delim)) {
		if (token.size() != 0) {
			vid_t v = atoi(token.c_str());
			adjacencies.push_back(v);
		}
	}

	return adjacencies;
}

}
 // end namespace

#endif

