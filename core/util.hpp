/*
 * util.hpp
 *
 *  Created on: Feb 28, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_UTIL_HPP_
#define MFLASH_CPP_CORE_UTIL_HPP_

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "type.hpp"

using namespace std;

namespace mflash{

	const int MFLASH_MATRIX_THREADS = 1;
	const int MFLASH_VECTOR_THREADS = 1;
	const double MAPPING_PERCENTAGE = 0.1;

	const string FILE_SEPARATOR = "/";
	const string DIRECTORY = ".G-FLASH";
	const string GRAPH = "graph";
  const string STREAM_FILE = "edge_stream";




	int64 get_mapping_limit(int64 block_size_bytes){
		return 1048576;//MAPPING_PERCENTAGE * block_size_bytes;
	}

	string get_parent_directory(string graph) {
		int64 pos = graph.find_last_of(FILE_SEPARATOR);

		return graph.substr(0, pos) + FILE_SEPARATOR;
	}

	string get_mflash_directory(string graph) {
		string path = get_parent_directory(graph) + DIRECTORY;

	/*	boost::filesystem::path dir(path);

		if( !boost::filesystem::exists(path) ){
				boost::filesystem::create_directories(path);
		}*/
		return path;
	}

	string get_stream_file(string graph) {
    return get_parent_directory(graph) + DIRECTORY + FILE_SEPARATOR + STREAM_FILE;
  }

	string get_block_file(string graph, int64 i, int64 j){
		std::stringstream file;
		file << get_mflash_directory(graph) << FILE_SEPARATOR << i << "_" << j << ".block";
		return file.str();
	}

	bool exist_file(string file){
		 ifstream f(file.c_str());
			if (f.good()) {
					f.close();
					return true;
			} else {
					f.close();
					return false;
			}
	}

	int64 file_size(string file){
		if (exist_file(file)){
				ifstream in(file, std::ifstream::ate | std::ifstream::binary);
				int64 size = in.tellg();
				in.close();
				return size;
		}
		return 0;
	}

	ios::openmode get_file_properties(string file, bool write){

    if(!write){
        return ios::in|ios::binary|ios::ate;
    }

    ios::openmode properties = ios::out|ios::binary|ios::ate;
    if(exist_file(file)){
        properties |= ios::in;
    }

    return properties;
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

	void quicksort(double values[], int64 indexes[], int64 left, int64 right, bool asc){
	  double pivot = values[left + (right - left) / 2];
    int i = left;
    int j = right;

    int operator_ = asc? 1: -1;

    while (i <= j) {
        while (operator_*values[i] < operator_*pivot) {
            i++;
        }
        while (operator_*values[j] > operator_*pivot) {
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

    if(left < j)
        quicksort(values, indexes,left,j, asc);
    if(i < right)
        quicksort(values, indexes, i,right, asc);

	}

	int64* sort_and_get_indexes(int64 n, double values[], bool asc){
		int64 *indexes = new int64[n];

	  for ( int64 i = 0; i < n; i++ ){
	    indexes[i] = i;
	  }

	  quicksort(values, indexes, 0, n-1, asc);

	  return indexes;
	}

}



#endif /* MFLASH_CPP_CORE_UTIL_HPP_ */
