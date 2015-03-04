/*
 * stream.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MMAPPOINTER_HPP_
#define MFLASH_CPP_CORE_MMAPPOINTER_HPP_

#include <bits/mman-linux.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdlib>
#include <string>

#include "type.hpp"

using namespace std;

namespace mflash{
	/**
	 * It creates a stream using memory mapping for reading files in sequential mode.
	 * The first implementation suppose that we can map whole file.
	 */
	template <class T>
	class MMapPointer{
			const static int64 ELEMENT_SIZE = (int64)sizeof(T);

			string file;
			int64 offset;
			int64 size_;

			T *ptr;
			T *last_ptr;

			int file_id;

	//		bool reverse;
		public:
			MMapPointer(string file, int64 offset, int64 size);
			~MMapPointer();
			T* address(){return ptr;}
			int64 size(){return size_;}
			T* get(int64 pos);

			void close_pointer();
	};

	template <class T>
	MMapPointer<T>::MMapPointer(string file, int64 offset, int64 size){
		int64 fsize = file_size(file);
		this->file = file;
		this->offset = offset;

		offset *= sizeof(T);
		size *= sizeof(T);

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
	inline T* MMapPointer<T>::get(int64 pos){
		return ptr + pos;
	}

	template <class T>
	MMapPointer<T>::~MMapPointer(){
		if (munmap(ptr, size_*sizeof(T)) == -1) {
			perror("Error un-mmapping the file");
		}
		close(this->file_id);
	}
}

#endif /* MFLASH_CPP_CORE_MMAPPOINTER_HPP_ */
