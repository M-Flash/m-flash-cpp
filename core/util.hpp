/*
 * util.hpp
 *
 *  Created on: Feb 28, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_UTIL_HPP_
#define MFLASH_CPP_CORE_UTIL_HPP_

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <sstream>
#include <string>

#include "type.hpp"

using namespace std;

namespace mflash{

	const int MFLASH_MATRIX_THREADS = 1;
	const int MFLASH_VECTOR_THREADS = 1;

	const string FILE_SEPARATOR = "/";
	const string DIRECTORY = ".G-FLASH";
	const string GRAPH = "graph";

	string get_parent_directory(string graph) {
		int64 pos = graph.find_last_of(FILE_SEPARATOR);

		return graph.substr(0, pos) + FILE_SEPARATOR + DIRECTORY;
	}

	string get_mflash_directory(string graph) {
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
	}

	int64* sort_and_get_indexes(int64 n, double values[], bool asc){
		int64 indexes[n];

	  for ( int64 i = 0; i < n; i++ ){
	    indexes[i] = i;
	  }

	  quicksort(values, indexes, 0, n-1, asc);

	  return indexes;
	}


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

}



#endif /* MFLASH_CPP_CORE_UTIL_HPP_ */
