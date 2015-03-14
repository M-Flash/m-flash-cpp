/*
 * edgelistthread.hpp
 *
 *  Created on: Mar 2, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_EDGELISTTHREAD_HPP_
#define MFLASH_CPP_CORE_EDGELISTTHREAD_HPP_

#include "streamprocessor.hpp"

namespace mflash{

	class EdgeListThread : public StreamProcessor{
		public:
			EdgeListThread(string file, BlockProperties &properties, int64 edge_size, int id, bool transpose, ElementIdSize &element_id_size, AbstractProcessor *processor)
		      : StreamProcessor(file, properties, edge_size, id, transpose, element_id_size, processor){}

			inline void call();
	};

	inline void EdgeListThread::call(){
			int64 vertex_value_size = this->edge_data_size;
			stick_this_thread_to_core(id);
			int bytes = 2 *(*element_id_size== ElementIdSize::SIMPLE? sizeof(int): sizeof(int64))  + vertex_value_size;
			int step = (MFLASH_MATRIX_THREADS -1)* bytes;

			this->stream->set_position (this->properties->offset + bytes * this->id);

			int64 in_vertex_id;
			int64 out_vertex_id;

			if(ElementIdSize::SIMPLE == *element_id_size){
			    if(BlockType::M_FLASH == this->properties->type){
            if(transpose){
              while(this->stream->has_remain()){
                in_vertex_id = stream->next_int();
                out_vertex_id = stream->next_int(step);
                edge_processor->next_edge(out_vertex_id, in_vertex_id, stream->current_pointer());
              }
            }else{
              while(this->stream->has_remain()){
                out_vertex_id = this->stream->next_int();
                in_vertex_id = this->stream->next_int(step);
                edge_processor->next_edge(out_vertex_id, in_vertex_id, stream->current_pointer());
              }
            }

			    }/*else if(BlockType::X_STREAM == this->properties->type){
            if(transpose){
              while(this->stream->has_remain()){
                in_vertex_id = this->stream->next_int();
                out_vertex_id = this->stream->next_int();
                this->worker->in_vertex.value = (V*) this->stream->next(vertex_value_size,step);
                this->worker->next_edge(out_vertex_id, in_vertex_id, edge_data);
              }
            }else{
              while(this->stream->has_remain()){
                out_vertex_id = this->stream->next_int();
                in_vertex_id = this->stream->next_int();
                this->worker->in_vertex.value = (V*) this->stream->next(vertex_value_size,step);
                this->worker->next_edge(out_vertex_id, in_vertex_id, edge_data);
              }
            }
			    }*/


			}
			this->stream->close_stream();

		}

}

#endif /* MFLASH_CPP_CORE_EDGELISTTHREAD_HPP_ */
