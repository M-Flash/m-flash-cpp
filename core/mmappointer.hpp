/*
 * stream.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MMAPPOINTER_HPP_
#define MFLASH_CPP_CORE_MMAPPOINTER_HPP_

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <sys/mman.h>

#include "type.hpp"
#include "util.hpp"


using namespace std;

namespace mflash{
	/**
	 * It creates a wrapper to map a region in memory with size independent of the architecture for random access
	 * The first implementation suppose that we can map whole file.
	 */
	template <class T>
	class RandomMMapPointer{
			const static int64 ELEMENT_SIZE = (int64)sizeof(T);

			string file;
			int64 offset;
			int64 size_;

			T *ptr;
			T *last_ptr;

			int file_id;

			bool closed;

	//		bool reverse;
		public:
			RandomMMapPointer(string file, int64 offset, int64 size);
			~RandomMMapPointer();
			T* address(){return ptr;}
			int64 size(){return size_;}
			T* get(int64 pos);

			void close_pointer();
	};

	template <class T>
	RandomMMapPointer<T>::RandomMMapPointer(string file, int64 offset, int64 size){
		int64 fsize = file_size(file);
		this->file = file;
		this->offset = offset;

		offset *= sizeof(T);
		size *= sizeof(T);

		closed = false;

		size = min(fsize - offset, size);
		this->size_ = size/ sizeof(T);


		file_id = open(this->file.c_str(), O_RDONLY);
		if (file_id == -1) {
			perror("Error opening file for reading");
			exit(EXIT_FAILURE);
		}
		ptr = (T*) mmap(0, size, PROT_READ, MAP_SHARED, file_id, offset);
		last_ptr = ptr + size;

		if (ptr == MAP_FAILED) {
			close(file_id);
			perror("Error mmapping the file");
			exit(EXIT_FAILURE);
		}

	}


	/**
	 *
	 */
	template <class T>
	inline T* RandomMMapPointer<T>::get(int64 pos){
		return ptr + pos;
	}

	template <class T>
	RandomMMapPointer<T>::~RandomMMapPointer(){
		close_pointer();
	}

	template <class T>
	void RandomMMapPointer<T>::close_pointer(){
		if(!closed){
			closed = true;
			if (munmap(ptr, size_*sizeof(T)) == -1) {
				perror("Error un-mmapping the file");
			}
			close(this->file_id);
		}
	}



}

#endif /* MFLASH_CPP_CORE_MMAPPOINTER_HPP_ */
