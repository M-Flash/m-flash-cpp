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
#include <fstream>
#include <iostream>
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
			int64 size;

			char *ptr;
			char *current_ptr;
			char *last_ptr;

			int file_id;

	//		bool reverse;
		public:
			MMapPointer(string file);

			int64 position();
			void set_position(int64 position);

			T* next();
			T* next(int64 step);

			bool has_remain();
			void close_pointer();
	};

	template <class T>
	MMapPointer<T>::MMapPointer(string file){
		this->file = file;
		this->offset = 0;

		//open file to check the size
		ifstream stream (this->file.c_str(), ios::in|ios::binary|ios::ate);
		this->size = stream.tellg();
		if(size == -1){
				throw 12;
		}
		stream.close();

		//reverse = false;

		file_id = open(this->file.c_str(), O_RDONLY);
		if (file_id == -1) {
			perror("Error opening file for reading");
			exit(EXIT_FAILURE);
		}
		ptr = (char*) mmap(0, size, PROT_READ, MAP_SHARED, file_id, 0);
		current_ptr = ptr;
		last_ptr = ptr + size;

		if (ptr == MAP_FAILED) {
			close(file_id);
			perror("Error mmapping the file");
			exit(EXIT_FAILURE);
		}

	}

	template <class T>
	inline bool MMapPointer<T>::has_remain(){
		return current_ptr < last_ptr;
	}

	/**
	 * Read the next element and increase the position in sizeof(T) + step
	 */
	template <class T>
	inline T* MMapPointer<T>::next(int64 step){
		char* ptr = current_ptr;
		current_ptr += step + step;
		return ptr;
	}

	/**
	 * Read the next element and increase the position in sizeof(T)
	 */
	template <class T>
	inline T* MMapPointer<T>::next(){
		return next(ELEMENT_SIZE);
	}


	template <class T>
	inline int64 MMapPointer<T>::position(){
		return current_ptr - ptr;
	}

	template <class T>
	inline void MMapPointer<T>::set_position(int64 position){
		char* p = position + ptr;
		if(p> ptr && p< last_ptr){
				this->current_ptr = p;
		}
	}

	template <class T>
	inline void MMapPointer<T>::close_pointer(){
		if (munmap(ptr, size) == -1) {
			perror("Error un-mmapping the file");
		}
		close(this->file_id);
	}
}

#endif /* MFLASH_CPP_CORE_MMAPPOINTER_HPP_ */
