/*
 * stream.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_STREAM_HPP_
#define MFLASH_CPP_CORE_STREAM_HPP_

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

/**
 * It creates a stream using memory mapping for reading files in sequential mode.
 * The first implementation suppose that we can map whole file.
 */
class Stream{
		const static int64 INT_SIZE = (int64)sizeof(int);
		const static int64 INT64_SIZE = (int64)sizeof(int64);
		const static int64 FLOAT_SIZE = (int64)sizeof(float);
		const static int64 DOUBLE_SIZE = (int64)sizeof(double);

		string file;
		int64 offset;
		int64 size;

		char *ptr;
		char *current_ptr;
		char *last_ptr;

		int file_id;

		bool reverse;
	public:
		Stream(string file);

		int64 position();
		void set_position(int64 position);

		char next();
		int next_int();
		int next_int(int64 step);
		char* next(int64 bytes, int64 step = 0);

//		int64 next_int64();
/*
		float next_float();
		double next_double();*/



		bool has_remain();

		void close_stream();
};


inline Stream::Stream(string file){
	this->file = file;
	this->offset = 0;

	//open file to check the size
	ifstream stream (this->file.c_str(), ios::in|ios::binary|ios::ate);
	this->size = stream.tellg();
	if(size == -1){
			throw 12;
	}
	stream.close();

	reverse = false;

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

inline bool Stream::has_remain(){
	return current_ptr < last_ptr;
}

inline char* Stream::next(int64 bytes, int64 step){
	char* ptr = current_ptr;
	current_ptr += bytes + step;
	return ptr;
}
inline int Stream::next_int(){
	return next_int(0);
}

inline int Stream::next_int(int64 step){
	int v = *( (int*) next(INT_SIZE, step) );
	if(reverse)
		return ((v >> 24)  & 0xFF)
					+(((v >> 16) & 0xFF) << 8)
					+(((v >>  8) & 0xFF) << 16)
					+ ((v        & 0xFF) << 24);
	return v;
}
/*
inline int64 Stream::next_int64(){
	return (int)*( next(INT64_SIZE) );
}*/

/*inline float Stream::next_float(){
	return (float)*( next(FLOAT_SIZE) );
}

inline double Stream::next_double(){
	return (double)*( next(DOUBLE_SIZE) );
}*/

inline int64 Stream::position(){
	return current_ptr - ptr;
}

inline void Stream::set_position(int64 position){
	char* p = position + ptr;
	if(p> ptr && p< last_ptr){
			this->current_ptr = p;
	}
}

inline void Stream::close_stream(){
  if (munmap(ptr, size) == -1) {
  	perror("Error un-mmapping the file");
  }
  close(this->file_id);
}


#endif /* MFLASH_CPP_CORE_STREAM_HPP_ */
