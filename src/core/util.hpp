// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef MFLASH_CPP_CORE_UTIL_HPP_
#define MFLASH_CPP_CORE_UTIL_HPP_

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <assert.h>
#include <map>
#include <boost/filesystem.hpp>

#include "../util/cmdopts.hpp"
#include "../core/type.hpp"
#include "../util/easylogging++.h"

using namespace std;

namespace mflash {


const int64 DEFAULT_MEMORY_SIZE = 4 * 1073741824L; //4GB
const int64 DEFAULT_ELEMENT_SIZE = 2 *sizeof(float); //4GB


const int MFLASH_MATRIX_THREADS = 1;
const int MFLASH_VECTOR_THREADS = 1;
const double MAPPING_PERCENTAGE = 0.1;

const string FILE_SEPARATOR = "/";
const string DIRECTORY = ".M-FLASH";
const string GRAPH = "graph";
const string STREAM_FILE = "edge_stream";

const int64 DEFAULT_BYTES_BLOCK = sizeof(int64) * 5; // 40 BYTES

int64 MEMORY_SIZE_BYTES = 1 * 1073741824L; //2GB



int64 get_mapping_limit(int64 block_size_bytes) {
	return 1048576; //MAPPING_PERCENTAGE * block_size_bytes;
}

string get_parent_directory(string graph) {
	int64 pos = graph.find_last_of(FILE_SEPARATOR);

	return graph.substr(0, pos) + FILE_SEPARATOR;
}

string get_filename(string graph) {
	int64 pos = graph.find_last_of(FILE_SEPARATOR);
	return graph.substr(pos+1);
}

string get_mflash_directory(string graph) {
	string path = get_parent_directory(graph) + DIRECTORY;
		boost::filesystem::path dir(path);

	 if( !boost::filesystem::exists(path) ){
	 boost::filesystem::create_directories(path);
	 }
	return path;
}

void clean_mflash_directory(string graph){
	boost::filesystem::remove_all(get_mflash_directory(graph));
}

string get_graph_directory(string graph) {
	string path = get_mflash_directory(graph) + FILE_SEPARATOR + get_filename(graph) + FILE_SEPARATOR;
	boost::filesystem::path dir(path);

	 if( !boost::filesystem::exists(path) ){
		 boost::filesystem::create_directories(path);
	 }
	return path;
}

string get_stream_file(string graph) {
	return get_graph_directory(graph) + STREAM_FILE;
}

string get_properties_file(string graph) {
	return get_graph_directory(graph) + GRAPH;
}


string get_block_file(string graph, int64 i, int64 j, string prefix = "") {
	std::stringstream file;
	file << get_graph_directory(graph) ;

	if(!prefix.empty())
			file<< prefix + "_" ;
	file << i << "_" << j << ".block";
	return file.str();
}


string get_partition_file(string graph, int64 partition_id, string prefix = "") {
	std::stringstream file;
	file << get_graph_directory(graph);
	if(!prefix.empty())
		file<< prefix + "_" ;
	file<< partition_id << ".partition";
	return file.str();
}
string get_in_degree_file(string graph) {
	std::stringstream file;
	file << get_graph_directory(graph);
	file<< get_filename(graph) << ".in_degree";
	return file.str();
}

string get_out_degree_file(string graph) {
	std::stringstream file;
	file << get_graph_directory(graph);
	file<< get_filename(graph) << ".out_degree";
	return file.str();
}



void delete_file(std::string file){
	boost::filesystem::remove(file);
}


void rename_file(std::string file, std::string newName){
	boost::filesystem::rename(file, newName);
}





bool exist_file(string file) {
	ifstream f(file.c_str());
	if (f.good()) {
		f.close();
		return true;
	} else {
		f.close();
		return false;
	}
}

int64 file_size(string file) {
	if (exist_file(file)) {
		ifstream in(file, std::ifstream::ate | std::ifstream::binary);
		int64 size = in.tellg();
		in.close();
		return size;
	}
	return 0;
}

ios::openmode get_file_properties(string file, bool write) {

	if (!write) {
		return ios::in | ios::binary | ios::ate;
	}

	ios::openmode properties = ios::out | ios::binary | ios::ate;
	if (exist_file(file)) {
		properties |= ios::in;
	}

	return properties;
}


/*
 * It updates the file that register all changes on BigMat directory, ie, it writes the BigMat structure.
 *			FILE STRUCTURE
 * # LINE	| DESCRIPTION
 *	1		| vertices
 *	2		| partitions or beta
 *	3		| id size: bytes required to store data in the computation
 *	4		| vertices_partitions
 *	5		| edges in block 0,0
 *	6		| edges in block 0,1
 *	.. 		|
 *	5+beta	| edges in block 0,beta-1
 *	.. 		|
 *	5 + beta*beta	| edges in block beta-1,beta-1
 */
void update_matrix_properties(std::string file_graph, MatrixProperties &properties) {
	string f_properties = get_properties_file(file_graph);
	std::ofstream file;
	file.open(f_properties.c_str());
	file << properties.vertices << std::endl;
	file << properties.idSize<< std::endl;
	file << properties.partitions<< std::endl;
	file << properties.vertices_partition<< std::endl;

	for(int i = 0 ; i<properties.partitions ; i++){
		for(int j = 0 ; j<properties.partitions ; j++){
			file<<properties.edges_by_block[i*properties.partitions  + j];
			file<<std::endl;
		}
	}
	file.close();
}

void update_matrix_properties(std::string file_graph, MatrixProperties *properties) {
	update_matrix_properties(file_graph, *properties);
}


MatrixProperties load_matrix_properties(std::string file_graph) {

	std::string filename = get_properties_file(file_graph);

	if(!exist_file(filename)){
		LOG(ERROR)<< filename << " not exist!!!";
		assert(false);
	}

	std::ifstream file;
	file.open(filename.c_str());

	MatrixProperties properties;//= new MatrixProperties();

	//std::getline(file, line);
	file >> properties.vertices;
	file >> properties.idSize;
	file >> properties.partitions;
	file >> properties.vertices_partition;
	properties.edges_by_block = new int64[properties.partitions * properties.partitions];
	for(int i = 0 ; i<properties.partitions ; i++){
		for(int j = 0 ; j<properties.partitions ; j++){
			file>>properties.edges_by_block[i*properties.partitions  + j];
		}
	}

	file.close();
	return properties;

}

template <class E, class IdType>
inline int64 getEdgeSize(){
	int64 size = 0;
	#if E != EmptyField
		size += sizeof(E)
	#endif
	return size +  (sizeof(IdType)<<1);

}

template <class E>
inline int64 getEdgeDataSize(){
	int64 size = 0;
	#if Edgetype != EmptyField
		size += sizeof(E)
	#endif
	return size;
}

template <class V>
inline int64 getVeticesByPartition() {
	return get_config_option_long("memorysize", DEFAULT_MEMORY_SIZE)/ (2 * sizeof(V));
}

inline int64 validateElementSize(int64 element_size) {
	if(element_size == 0){
		return get_config_option_long("elementsize", DEFAULT_ELEMENT_SIZE);
	}
	return element_size;
}

inline int64 getVeticesByPartition(int64 element_size) {
	element_size = validateElementSize(element_size);
	return get_config_option_long("memorysize", DEFAULT_MEMORY_SIZE)/ (element_size);
}

/**
 *
 * int64 edge_size = 2 * sizeof(IdType) + edge_data_size;
				for (int64 j = 0; j < partitions; j++){
					ratio = ((double)1)/partitions + ( ((double)2) * counters[i * partitions + j] * edge_size / vertices_by_partition);
					block_types[j] = ratio<1? BlockType::SPARSE: BlockType::DENSE;
				}
 *
 */

template<class E, class IdType>
inline double getBlockRatio(int64 partitions, int64 vertices_by_partition, int64 edges) {
	return ((double)1)/partitions + ( ((double)2) * edges * getEdgeSize<E, IdType>()/ vertices_by_partition);
}




template<class E, class IdType>
inline BlockType getBlockType(int64 partitions, int64 vertices_by_partition, int64 edges) {
	double ratio = getBlockRatio<E, IdType>(partitions, vertices_by_partition, edges);
	return ratio < 1.0 ? BlockType::SPARSE : BlockType::DENSE;
}


/*string get_mflash_directory(string graph) {
 string path = get_parent_directory(graph);

 boost::filesystem::path dir(path);

 if( !boost::filesystem::exists(path) ){
 boost::filesystem::create_directories(path);
 }
 return path;
 }

 string get_block_file(string graph, int64 i, int64 j){
 std::stringstream file;
 file << get_mflash_directory(graph) << FILE_SEPARATOR << i << "_" << j << ".block";
 return file.str();
 }

 bool exist_file(string file){
 return boost::filesystem::exists(boost::filesystem::path(file));
 }

 int64 file_size(string file){
 return boost::filesystem::file_size(boost::filesystem::path(file));
 }*/

void quicksort(double values[], int64 indexes[], int64 left, int64 right,
		bool asc) {
	double pivot = values[left + (right - left) / 2];
	int i = left;
	int j = right;

	int operator_ = asc ? 1 : -1;

	while (i <= j) {
		while (operator_ * values[i] < operator_ * pivot) {
			i++;
		}
		while (operator_ * values[j] > operator_ * pivot) {
			j--;
		}
		if (i <= j) {

			int64 idxTmp = indexes[i];
			double valueTmp = values[i];

			values[i] = values[j];
			values[j] = valueTmp;

			indexes[i] = indexes[j];
			indexes[j] = idxTmp;

			i++;
			j--;
		}
	}

	if (left < j)
		quicksort(values, indexes, left, j, asc);
	if (i < right)
		quicksort(values, indexes, i, right, asc);

}

int64* sort_and_get_indexes(int64 n, double values[], bool asc) {
	int64 *indexes = new int64[n];

	for (int64 i = 0; i < n; i++) {
		indexes[i] = i;
	}

	quicksort(values, indexes, 0, n - 1, asc);

	return indexes;
}

/*
int stick_this_thread_to_core(int core_id) {
	int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
	if (core_id < 0 || core_id >= num_cores)
		return EINVAL;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(core_id, &cpuset);

	pthread_t current_thread = pthread_self();
	return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}
*/
}

#endif /* MFLASH_CPP_CORE_UTIL_HPP_ */
