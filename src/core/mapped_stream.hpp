// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef MFLASH_CPP_CORE_MAPPED_STREAM_HPP_
#define MFLASH_CPP_CORE_MAPPED_STREAM_HPP_

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <boost/interprocess/mapped_region.hpp>

#include "../core/type.hpp"
#include "../core/util.hpp"


using namespace std;

namespace mflash{

  /**
   * It creates a stream using memory mapping for reading files in sequential mode.
   * The first implementation suppose that we can map whole file.
   */
  class MappedStream{
	  const static int64 CHAR_SIZE = (int64)sizeof(char);
      const static int64 INT_SIZE = (int64)sizeof(int);
      const static int64 INT64_SIZE = (int64)sizeof(int64);
      const static int64 FLOAT_SIZE = (int64)sizeof(float);
      const static int64 DOUBLE_SIZE = (int64)sizeof(double);
      int64 PAGE_SIZE = boost::interprocess::mapped_region::get_page_size();
  public:
      string file;
      int64 offset;
      int64 size;

      char *ptr;
      char *current_ptr;
      char *last_ptr;

      int file_id;

      bool reverse;

      MappedStream(string file, int64 offset, int64 size);
      MappedStream(string file): MappedStream(file, 0,0){};

      int64 position();
      void set_position(int64 position);

      char next_char();
      int next_int();
      int next_int(int64 step);
      char* next(int64 bytes, int64 step = 0);

      template<class Type>
      Type next(int64 step = 0);

      bool has_remain();

      void close_stream();
  };


  inline MappedStream::MappedStream(string file, int64 offset, int64 size){
    this->file = file;
    this->offset = offset;
    //this->opposite_direction = opposite_direction;

    if(size <=0){
      size = file_size(file);
    }

    //PAGE ALIGNMENT
    int64 bytes_align = offset %PAGE_SIZE;
    offset -= bytes_align;
    size += bytes_align;



    this->size = size;
    reverse = false;

    file_id = open(this->file.c_str(), O_RDONLY);
    if (file_id == -1) {
      perror("Error opening file for reading");
      exit(EXIT_FAILURE);
    }
    //cout<< file << endl;
    //cout<< sizeof(size_t)<<endl;
    ptr = (char*) mmap(0, size , PROT_READ, MAP_SHARED, file_id, offset);

    //MOVING POINTER TO THE FIRST POSITION OF THE OFFSET
    //if(!opposite_direction){
      current_ptr = ptr + bytes_align;
      last_ptr = ptr + size;
    /*}else{
      last_ptr  = ptr + bytes_align;
      current_ptr = ptr + size;
    }*/

    if (ptr == MAP_FAILED) {
      close(file_id);
      perror("Error mmapping the file");
      exit(EXIT_FAILURE);
    }

  }

  inline bool MappedStream::has_remain(){
    return /*opposite_direction? current_ptr > last_ptr : */current_ptr < last_ptr;
  }


  inline char* MappedStream::next(int64 bytes, int64 step){
    char* ptr;
    //if(!opposite_direction){
      ptr = current_ptr;
      current_ptr += bytes + step;
    /*}else{
      ptr = current_ptr - bytes;
      current_ptr -= (bytes + step);
    }*/
    return ptr;
  }

  template<class Type>
  inline Type MappedStream::next(int64 step){
    Type ptr = *( (Type*)current_ptr);
	current_ptr += sizeof(Type) + step;
    return ptr;
  }


  /*inline char* MappedStream::next(int64 bytes, int64 step){
     char* ptr;
     ptr = current_ptr;
     current_ptr += bytes + step;
     return ptr;
   }*/

  inline int MappedStream::next_int(){
    return next_int(0);
  }

  inline char MappedStream::next_char(){
    return  next<char>();
  }


  inline int MappedStream::next_int(int64 step){
	  return  *( (int*) next(INT_SIZE, step) );
    //int v = *( (int*) next(INT_SIZE, step) );
    /*if(reverse)
      return ((v >> 24)  & 0xFF)
            +(((v >> 16) & 0xFF) << 8)
            +(((v >>  8) & 0xFF) << 16)
            + ((v        & 0xFF) << 24);*/
  //  return v;
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

  inline int64 MappedStream::position(){
    return current_ptr - ptr;
  }

  inline void MappedStream::set_position(int64 position){
    char* p = position + ptr;
    if(p> ptr && p< last_ptr){
        current_ptr = p;
    }
  }

  inline void MappedStream::close_stream(){
    if (munmap(ptr, size) == -1) {
      perror("Error un-mmapping the file");
    }
    close(file_id);
  }

}
#endif /* MFLASH_CPP_CORE_MAPPED_STREAM_HPP_ */
