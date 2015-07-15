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

	template <class V, class E>
	class EdgeListThread : public StreamProcessor<V,E>{
		public:
			EdgeListThread(string file, BlockProperties &properties, MatrixWorker<V,E> &worker, int id, bool opposite_direction) : StreamProcessor<V,E>(file, properties,worker, id, opposite_direction){}

			template< class MALGORITHM>
			void call(MALGORITHM &algorithm);

			template< class MALGORITHM>
			void dense_transpose(MALGORITHM &algorithm, int64 step);

			template< class MALGORITHM>
			void dense_normal(MALGORITHM &algorithm, int64 step);

			template< class MALGORITHM>
			void sparse_transpose(MALGORITHM &algorithm, int64 step, int64 vertex_value_size);

			template< class MALGORITHM>
			void sparse_normal(MALGORITHM &algorithm, int64 step, int64 vertex_value_size);
	};

	template <class V, class E>
	template <class MALGORITHM>
	void EdgeListThread<V,E>::call(MALGORITHM &algorithm){
			const ElementIdSize id_size = this->worker->get_matrix().get_element_id_size();
			int64 vertex_value_size = (BlockType::SPARSE== this->properties->type? sizeof(V): 0);
			const int bytes = 2 *(id_size == ElementIdSize::SIMPLE? sizeof(int): sizeof(int64))  + vertex_value_size;
			const int step = (MFLASH_MATRIX_THREADS -1)* bytes;
			//if(!this->opposite_direction){
        if(ElementIdSize::SIMPLE == id_size){
            if(BlockType::DENSE == this->properties->type){
              //check the offset for multithread in sparse blocks
              this->stream->set_position (this->properties->offset + bytes * this->id);
              if(this->worker->matrix->is_transpose()){
                  dense_transpose(algorithm, step);
              }else{
                  dense_normal(algorithm, step);
              }
            }else if(BlockType::SPARSE == this->properties->type){
              if(this->worker->matrix->is_transpose()){
                  sparse_transpose(algorithm, step, vertex_value_size);
              }else{
                  sparse_normal(algorithm, step, vertex_value_size);
              }
            }
        }
			//}
			this->stream->close_stream();
		}


	template <class V, class E>
  template <class MALGORITHM>
  void EdgeListThread<V,E>::dense_transpose(MALGORITHM &algorithm, int64 step){
	  E edge_data;
	  MappedStream *stream = this->stream;
	  MatrixWorker<V,E> *worker= this->worker;
	  int64 in_vertex_id;
    int64 out_vertex_id;
    Array<V> *in = this->worker->get_in_array();
    Array<V> *out = this->worker->get_out_array();
    Element<V> in_vertex;
    Element<V> out_vertex_accumulator;
    int64 next;
    char * ptr;
    char * last_ptr;

    if(!this->opposite_direction){
        next = 8;
        ptr =  stream->current_ptr;
        last_ptr = stream->last_ptr;
        while(ptr < last_ptr){
         out_vertex_accumulator.id = out_vertex_id = *( (int*)ptr);
         in_vertex.id = in_vertex_id = *( (int*)(ptr+4));
         /*if(this->worker->load_vertex_data)*/ in_vertex.value = in->get_element(in_vertex_id);
         /*if(this->worker->load_dest_data) */out_vertex_accumulator.value = out->get_element(out_vertex_id);
         algorithm.gather(*worker, in_vertex, out_vertex_accumulator, edge_data);
         ptr+=next;
       }
    }else{
        /*next = -8;
        ptr =  stream->last_ptr + next;
        last_ptr = stream->current_ptr;
        while(ptr > last_ptr){
         out_vertex_accumulator.id = out_vertex_id = *( (int*)ptr);
         in_vertex.id = in_vertex_id = *( (int*)ptr+4);
         if(this->worker->load_vertex_data) in_vertex.value = in->get_element(in_vertex_id);
         if(this->worker->load_dest_data) out_vertex_accumulator.value = out->get_element(out_vertex_id);
         algorithm.gather(*worker, in_vertex, out_vertex_accumulator, edge_data);
         ptr+=next;
       }*/
        next = 8;
        int64 shunk_size = 1073741824 ;
        int64 shunks = ((int64)stream->size/shunk_size)+ (stream->size%shunk_size ==0? 0: 1);
        for(int i = shunks-1; i>=0; i-- /*int i = shunks-1; i>=0; i--*/){
            ptr =  stream->current_ptr + (shunk_size * i);
            last_ptr = min(ptr+shunk_size, stream->last_ptr);
            while(ptr < last_ptr){
                out_vertex_accumulator.id = out_vertex_id = *( (int*)ptr);
                in_vertex.id = in_vertex_id = *( (int*)(ptr+4));

               /*if(this->worker->load_vertex_data)*/ in_vertex.value = in->get_element(in_vertex_id);
               /*if(this->worker->load_dest_data) */out_vertex_accumulator.value = out->get_element(out_vertex_id);
               algorithm.gather(*worker, in_vertex, out_vertex_accumulator, edge_data);
               ptr+=next;
            }
        }
    }


	}

	template <class V, class E>
  template <class MALGORITHM>
  void EdgeListThread<V,E>::dense_normal(MALGORITHM &algorithm, int64 step){
	  E edge_data;
	  MatrixWorker<V,E> *worker= this->worker;
    MappedStream *stream = this->stream;
    int64 in_vertex_id;
    int64 out_vertex_id;
    Array<V> *in = this->worker->get_in_array();
    Array<V> *out = this->worker->get_out_array();
    Element<V> in_vertex;
    Element<V> out_vertex_accumulator;

    int64 next;
    char * ptr;
    char * last_ptr;

    if(!this->opposite_direction){
        next = 8;
        ptr =  stream->current_ptr;
        last_ptr = stream->last_ptr;
        while(ptr < last_ptr){
          in_vertex.id = in_vertex_id = *( (int*)ptr);
          out_vertex_accumulator.id = out_vertex_id = *( (int*)(ptr+4));
          /*if(this->worker->load_vertex_data)*/ in_vertex.value = in->get_element(in_vertex_id);
          /*if(this->worker->load_dest_data) */out_vertex_accumulator.value = out->get_element(out_vertex_id);
          algorithm.gather(*worker, in_vertex, out_vertex_accumulator, edge_data);
          ptr+=next;
        }
    }else{
        next = 8;
        int64 shunk_size = 1073741824 ;
        int64 shunks = ((int64)stream->size/shunk_size)+ (stream->size%shunk_size ==0? 0: 1);
        for(int i = shunks-1; i>=0; i-- /*int i = shunks-1; i>=0; i--*/){
            ptr =  stream->current_ptr + (shunk_size * i);
            last_ptr = min(ptr+shunk_size, stream->last_ptr);
            while(ptr < last_ptr){
               in_vertex.id = in_vertex_id = *( (int*)ptr);
               out_vertex_accumulator.id = out_vertex_id = *( (int*)(ptr+4));
               /*if(this->worker->load_vertex_data)*/ in_vertex.value = in->get_element(in_vertex_id);
               /*if(this->worker->load_dest_data) */out_vertex_accumulator.value = out->get_element(out_vertex_id);
               algorithm.gather(*worker, in_vertex, out_vertex_accumulator, edge_data);
               ptr+=next;
            }
        }
    }
	}

	template <class V, class E>
  template <class MALGORITHM>
  void EdgeListThread<V,E>::sparse_transpose(MALGORITHM &algorithm, int64 step, int64 vertex_value_size){
	  E edge_data;
    MappedStream *stream = this->stream;
    MatrixWorker<V,E> *worker= this->worker;
    int64 in_vertex_id;
    int64 out_vertex_id;
    Array<V> *out = this->worker->get_out_array();
    Element<V> in_vertex;
    Element<V> out_vertex_accumulator;


    char * ptr;
    char * last_ptr;
    int64 next = 2 * sizeof(int) + vertex_value_size;

    ptr =  stream->current_ptr;
    last_ptr = stream->last_ptr;
    while(ptr < last_ptr){
        out_vertex_accumulator.id = out_vertex_id = *( (int*)ptr);
        in_vertex.id = in_vertex_id = *( (int*)(ptr+4));
        in_vertex.value = (V*)(ptr+8);
        out_vertex_accumulator.value = out->get_element(out_vertex_id);
        algorithm.gather(*worker, in_vertex, out_vertex_accumulator, edge_data);
        ptr+=next;
    }
    /*while(stream->has_remain()){
      in_vertex_id = stream->next_int();
      out_vertex_id = stream->next_int();
      this->worker->in_vertex.value = (V*) this->stream->next(vertex_value_size,step);
      this->worker->next_edge(out_vertex_id, in_vertex_id, edge_data);
    }*/
  }

	template <class V, class E>
  template <class MALGORITHM>
  void EdgeListThread<V,E>::sparse_normal(MALGORITHM &algorithm, int64 step, int64 vertex_value_size){
	  E edge_data;
    MappedStream *stream = this->stream;
    MatrixWorker<V,E> *worker= this->worker;
    int64 in_vertex_id;
    int64 out_vertex_id;
    Array<V> *out = this->worker->get_out_array();
    Element<V> in_vertex;
    Element<V> out_vertex_accumulator;
    char * ptr;
    char * last_ptr;
    int64 next = 2 * sizeof(int) + vertex_value_size;

    ptr =  stream->current_ptr;
    last_ptr = stream->last_ptr;
    while(ptr < last_ptr){
        in_vertex.id = in_vertex_id = *( (int*)(ptr));
        out_vertex_accumulator.id = out_vertex_id = *( (int*)(ptr+4));
        in_vertex.value = (V*)(ptr+8);
        out_vertex_accumulator.value = out->get_element(out_vertex_id);
        algorithm.gather(*worker, in_vertex, out_vertex_accumulator, edge_data);
        ptr+=next;
    }

    /*while(this->stream->has_remain()){
      out_vertex_id = stream->next_int();
      in_vertex_id = stream->next_int();
      this->worker->in_vertex.value = (V*) stream->next(vertex_value_size,step);
      this->worker->next_edge(out_vertex_id, in_vertex_id, edge_data);
    }*/
}


}

#endif /* MFLASH_CPP_CORE_EDGELISTTHREAD_HPP_ */
