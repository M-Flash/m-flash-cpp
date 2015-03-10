/*
 * edgelistthread.hpp
 *
 *  Created on: Mar 2, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_EDGELISTREWRITER_HPP_
#define MFLASH_CPP_CORE_EDGELISTREWRITER_HPP_

#include <string>

#include "array.hpp"
#include "direct_stream.hpp"
#include "edgelistthread.hpp"
#include "matrix.hpp"
#include "matrixworker.hpp"
#include "streamprocessor.hpp"
#include "type.hpp"

namespace mflash{

	template <class V, class E>
	class EdgeListWriter: public StreamProcessor<V,E>{

	  /**
	   * Position used to shift in the initial position on the file and store all blocks using only one file. We must add support for block with edge values!!!!!
	   */
	  DirectStreamWriter* writer;


		public:
	    EdgeListWriter(DirectStreamWriter *writer, string block_file, BlockProperties properties, MatrixWorker<V,E> &worker) : StreamProcessor<V,E>(block_file, properties, worker, 0){
	      this->writer = writer;
	    }
			void call();
	};

	template <class V, class E>
	inline void EdgeListWriter<V,E>::call(){
			//E edge_data;
			const ElementIdSize id_size = this->worker->get_matrix().get_element_id_size();
			const bool transpose = this->worker->get_matrix().is_transpose();
			int bytes = id_size == ElementIdSize::SIMPLE? sizeof(int): sizeof(int64);
		//	int step = 0;
			Array<V> *in_array = this->worker->get_in_array();
			int64 vertex_size_bytes = sizeof(V);

			this->stream->set_position (bytes * this->id);

			int64 in_vertex_id;
			int64 out_vertex_id;

			if(ElementIdSize::SIMPLE == id_size){
					if(transpose){
						while(this->stream->has_remain()){
							in_vertex_id = this->stream->next_int();
							out_vertex_id = this->stream->next_int();
							//writing edge
							writer->write(&in_vertex_id, bytes);
							writer->write(&out_vertex_id, bytes);
							//writing vertex values
							writer->write((void*)in_array->get_element(out_vertex_id), vertex_size_bytes);
							//this->worker->next_edge(out_vertex_id, in_vertex_id, edge_data);
						}
					}else{
							while(this->stream->has_remain()){
								out_vertex_id = this->stream->next_int();
								in_vertex_id = this->stream->next_int();
								//writing edge
                writer->write(&out_vertex_id, bytes);
                writer->write(&in_vertex_id, bytes);
                //writing vertex values
                writer->write(in_array->get_element(out_vertex_id), vertex_size_bytes);
							}
					}
			}
			//writer->close_stream();
			this->stream->close_stream();
		}
}

#endif /* MFLASH_CPP_CORE_EDGELISTREWRITER_HPP_ */
