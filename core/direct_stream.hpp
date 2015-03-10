/*
 * stream.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_STREAM_HPP_
#define MFLASH_CPP_CORE_STREAM_HPP_

#include <cstring>
#include <fstream>
#include <string>

#include "type.hpp"
#include "util.hpp"

using namespace std;


namespace mflash{
  /**
   * It creates a stream using memory mapping for reading files in sequential mode.
   * The first implementation suppose that we can map whole file.
   */
  class DirectStreamWriter{

      ofstream *stream;
      string file;
      int64 offset;

      char *buffer;
      char *current_position_buffer;
      char *last_position_buffer;
      //int64 buffer_position;
      int64 buffer_size;

    public:
      DirectStreamWriter(string file, int64 position = 0, int64 buffer_size = 1024);

      void write(void * address, int64 size);
      void flush();
      void close_stream();
  };


  inline DirectStreamWriter::DirectStreamWriter(string file, int64 position, int64 buffer_size){
    this->file = file;
    this->offset = position;
    this->buffer_size = buffer_size;

    this->buffer = new char[buffer_size];
    this->current_position_buffer = this->buffer;
    this->last_position_buffer = this->buffer + buffer_size;

    stream = new ofstream(this->file.c_str(), get_file_properties(file, true));

  }

  inline void DirectStreamWriter::flush(){
    if(current_position_buffer != buffer){
        stream->seekp (offset, ios::beg);
        stream->write(buffer, current_position_buffer - buffer);
        offset += current_position_buffer - buffer;
        current_position_buffer = buffer;
    }
  }


  inline void DirectStreamWriter::write(void* address, int64 size){
    if(current_position_buffer + size > last_position_buffer){
        flush();
    }
    memcpy(current_position_buffer, (char*)address, size);
    current_position_buffer += size;
  }

  inline void DirectStreamWriter::close_stream(){
    if(stream != 0){
        flush();
        stream->close();
        delete [] buffer;
    }
  }

}
#endif /* MFLASH_CPP_CORE_STREAM_HPP_ */
