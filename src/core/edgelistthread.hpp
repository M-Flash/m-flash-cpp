// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef MFLASH_CPP_CORE_EDGELISTTHREAD_HPP_
#define MFLASH_CPP_CORE_EDGELISTTHREAD_HPP_

#include "array.hpp"
#include "blockiterator.hpp"
#include "mapped_stream.hpp"
#include "matrixworker.hpp"
#include "type.hpp"


namespace mflash{



	/*
	 * WITHOUT MULTITHREAD SUPPORT AND AUXILIAR FIELDS FOR SOURCE AND DESTINATION
	 */
	template <class E, class IdType, class VSource, class VDestination>
	class EdgeListThread {
		private:

			MatrixWorker<E, IdType> *worker;
			int thread_id;
			Block *block;
			MappedStream *stream;

			template <class MALGORITHM>
			void dense_transpose(MALGORITHM &algorithm, int64 step);

			template <class MALGORITHM>
			void dense_normal(MALGORITHM &algorithm, int64 step);

			template <class MALGORITHM>
			void sparse_transpose(MALGORITHM &algorithm, int64 step);

			template <class MALGORITHM>
			void sparse_normal(MALGORITHM &algorithm, int64 step);

		public:
			EdgeListThread(MatrixWorker<E, IdType> &worker, int thread_id, Block &block);

			template <class MALGORITHM>
			void call(MALGORITHM &algorithm);

	};

	template <class E, class IdType, class VSource, class VDestination>
	EdgeListThread<E, IdType, VSource, VDestination>::EdgeListThread(MatrixWorker<E, IdType> &worker, int thread_id, Block &block){
		this->worker = &worker;
		this->thread_id = thread_id;
		this->block = &block;
		this->stream = 0;
	}

	template <class E, class IdType, class VSource, class VDestination>
	template <class MALGORITHM>
	void EdgeListThread<E, IdType, VSource, VDestination>::call(MALGORITHM &algorithm){

		stream = new MappedStream(block->get_file());

/*
		const ElementIdType id_size = this->worker->get_matrix().get_element_id_size();
		int64 vertex_value_size = (BlockType::SPARSE== this->properties->type? sizeof(V): 0);
		const int bytes = 2 *(id_size == ElementIdType::SIMPLE? sizeof(int): sizeof(int64))  + vertex_value_size;
		const int step = (MFLASH_MATRIX_THREADS -1)* bytes;
*/
		int step = 0;
		if(block->isDense()){
		  //this->stream->set_position (this->properties->offset + bytes * this->id);
		  if(this->worker->matrix->is_transpose()){
			  dense_transpose(algorithm, step);
		  }else{
			  dense_normal(algorithm, step);
		  }
		}else{
		  if(this->worker->matrix->is_transpose()){
			  sparse_transpose(algorithm, step);
		  }else{
			  sparse_normal(algorithm, step);
		  }
		}

		this->stream->close_stream();
		delete stream;
	}

template <class E, class IdType, class VSource, class VDestination>
template <class MALGORITHM>
void EdgeListThread<E, IdType, VSource, VDestination>::dense_transpose(MALGORITHM &algorithm, int64 step) {
	E edge_data;

	IdType in_vertex_id;
	IdType out_vertex_id;

	Array<VSource, IdType> *in = (Array<VSource, IdType>*)this->worker->source_pointer;
	Array<VDestination, IdType>  *out =  (Array<VDestination, IdType> *)this->worker->destination_pointer;

	Element<VSource, IdType> in_vertex;
	Element<VDestination, IdType> out_vertex_accumulator;

	int64 next = getEdgeSize<E, IdType>();

	char * ptr;
	char * last_ptr;

	ptr = stream->current_ptr;
	last_ptr = stream->last_ptr;
	while (ptr < last_ptr) {
		out_vertex_accumulator.id = out_vertex_id = *((IdType*) ptr);
		in_vertex.id = in_vertex_id = *((IdType*) (ptr + sizeof(IdType)));
		/*if(this->worker->load_vertex_data)*/
		in_vertex.value = in->get_element(in_vertex_id);
		/*if(this->worker->load_dest_data) */
		out_vertex_accumulator.value = out->get_element(out_vertex_id);
		algorithm.gather(*worker, in_vertex, out_vertex_accumulator,
				edge_data);
		ptr += next;
	}
}
template <class E, class IdType, class VSource, class VDestination>
template <class MALGORITHM>
void EdgeListThread<E, IdType, VSource, VDestination>::dense_normal(MALGORITHM &algorithm, int64 step){
	E edge_data;

	MappedStream *stream = this->stream;

	IdType in_vertex_id;
	IdType out_vertex_id;

	Array<VSource, IdType> *in = (Array<VSource, IdType>*)this->worker->source_pointer;
	Array<VDestination, IdType>  *out =  (Array<VDestination, IdType> *)this->worker->destination_pointer;

	Element<VSource, IdType> in_vertex;
	Element<VDestination, IdType> out_vertex_accumulator;

	int64 next = getEdgeSize<E, IdType>();

	char * ptr;
	char * last_ptr;

	ptr = stream->current_ptr;
	last_ptr = stream->last_ptr;
	while (ptr < last_ptr) {
		in_vertex.id = in_vertex_id = *((IdType*) ptr);
		out_vertex_accumulator.id = out_vertex_id = *((IdType*) (ptr + sizeof(IdType)));
		/*if(this->worker->load_vertex_data)*/
		in_vertex.value = in->get_element(in_vertex_id);
		/*if(this->worker->load_dest_data) */
		out_vertex_accumulator.value = out->get_element(out_vertex_id);
		algorithm.gather(*worker, in_vertex, out_vertex_accumulator,edge_data);
		ptr += next;
	}
}

template <class E, class IdType, class VSource, class VDestination>
template <class MALGORITHM>
void EdgeListThread<E, IdType, VSource, VDestination>::sparse_transpose(MALGORITHM &algorithm, int64 step){
	E edge_data;

	IdType in_vertex_id;
	IdType out_vertex_id;

	//Array<VSource, IdType> *in = (Array<VSource, IdType>*)this->worker->source_pointer;
	Array<VDestination, IdType>  *out =  (Array<VDestination, IdType> *)this->worker->destination_pointer;

	Element<VSource, IdType> in_vertex;
	Element<VDestination, IdType> out_vertex_accumulator;

	int64 next = getEdgeSize<E, IdType>() + worker->source_size();

	char * ptr;
	char * last_ptr;

	ptr = stream->current_ptr;
	last_ptr = stream->last_ptr;
	while (ptr < last_ptr) {
		out_vertex_accumulator.id = out_vertex_id = *((IdType*) ptr);
		in_vertex.id = in_vertex_id = *((IdType*) (ptr + sizeof(IdType)));
		in_vertex.value = (VSource*) (ptr + (sizeof(IdType) <<1));
		out_vertex_accumulator.value =  out->get_element(out_vertex_id);
		algorithm.gather(*worker, in_vertex, out_vertex_accumulator, edge_data);
		ptr += next;
	}
}

template <class E, class IdType, class VSource, class VDestination>
template <class MALGORITHM>
void EdgeListThread<E, IdType, VSource, VDestination>::sparse_normal(MALGORITHM &algorithm, int64 step){
	E edge_data;

	IdType in_vertex_id;
	IdType out_vertex_id;

	//Array<VSource, IdType> *in = (Array<VSource, IdType>*)this->worker->source_pointer;
	Array<VDestination, IdType>  *out =  (Array<VDestination, IdType> *)this->worker->destination_pointer;

	Element<VSource, IdType> in_vertex;
	Element<VDestination, IdType> out_vertex_accumulator;

	int64 next = getEdgeSize<E, IdType>() + worker->source_size();

	char * ptr;
	char * last_ptr;

	ptr = stream->current_ptr;
	last_ptr = stream->last_ptr;
	while (ptr < last_ptr) {
		in_vertex.id = in_vertex_id = *((IdType*) ptr);
		out_vertex_accumulator.id = out_vertex_id = *((IdType*)(ptr + sizeof(IdType)));
		in_vertex.value = (VSource*) (ptr + (sizeof(IdType) <<1));
		out_vertex_accumulator.value = out->get_element(out_vertex_id);
		algorithm.gather(*worker, in_vertex, out_vertex_accumulator, edge_data);
		ptr += next;
	}

}

}

#endif /* MFLASH_CPP_CORE_EDGELISTTHREAD_HPP_ */
