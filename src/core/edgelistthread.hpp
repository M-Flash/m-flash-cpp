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
			bool check_ids(MALGORITHM &algorithm, int64 step);


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

		int step = 0;

		if (get_option_int("check_ids", 0) == 1){
		  check_ids(algorithm, step);
		}else{

		  if(block->isDense()){
		    //this->stream->set_position (this->properties->offset + bytes * this->id);
		    if(this->worker->matrix->is_transpose()){
			    LOG (INFO) << "--- PROCESSING DENSE BLOCK IN TRANSPOSE MODE";
			    dense_transpose(algorithm, step);
		    }else{
			    LOG (INFO) << "--- PROCESSING DENSE BLOCK IN NORMAL MODE";
			    dense_normal(algorithm, step);
		    }
		  }else{
		    if(this->worker->matrix->is_transpose()){
			    LOG (INFO) << "--- PROCESSING SPARSE BLOCK IN TRANSPOSE MODE";
			    sparse_transpose(algorithm, step);
		    }else{
			    LOG (INFO) << "--- PROCESSING SPARSE BLOCK IN NORMAL MODE";
			    sparse_normal(algorithm, step);
		    }
		  }
		}
		this->stream->close_stream();
		delete stream;
	}

template <class E, class IdType, class VSource, class VDestination>
template <class MALGORITHM>
bool EdgeListThread<E, IdType, VSource, VDestination>::check_ids(MALGORITHM &algorithm, int64 step) {
      const int64 elements = worker->matrix->size();
      const int64 vertices_by_partition = worker->matrix->get_elements_by_block();

      int64 source_offset;
      int64 destination_offset;
      int64 source_limit;
      int64 destination_limit;
      int64 next;


      if(block->isDense()){
	next = getEdgeSize<E, IdType>();
      }else{
	next = getEdgeSize<E, IdType>() + worker->source_size();
      }

      source_offset = block->get_col() * vertices_by_partition;
      destination_offset = block->get_row()*vertices_by_partition;
      source_limit =source_offset + min(source_offset + vertices_by_partition, elements - source_offset)- 1;
      destination_limit = destination_offset + min(destination_offset + vertices_by_partition, elements-destination_offset) -1;

      IdType in_vertex_id;
      IdType out_vertex_id;
      char * ptr;
      char * last_ptr;
      LOG (INFO) << "VERIFING IDS FOR ALL EDGES WITHIN BLOCKS";
      ptr = stream->current_ptr;
      last_ptr = stream->last_ptr;
      bool transpose =  this->worker->matrix->is_transpose();
      while (ptr < last_ptr) {
	    if(!transpose){
	      in_vertex_id = *((IdType*) ptr);
	      out_vertex_id = *((IdType*) (ptr + sizeof(IdType)));
	    }else{
	      out_vertex_id = *((IdType*) ptr);
	      in_vertex_id = *((IdType*) (ptr + sizeof(IdType)));
	    }

	    if(in_vertex_id < source_offset || in_vertex_id >  source_limit ||
		out_vertex_id < destination_offset || out_vertex_id >  destination_limit ){
		LOG (ERROR) << "Edge ("<< in_vertex_id << "," << out_vertex_id <<") out of bounds within the block" << block->get_file() <<". "
			    << "Input interval("<<source_offset<<","<< source_limit<<")."<< "Output interval("<<destination_offset<<","<< destination_limit<<").";
		assert(false);
		return false;
	    }
	    ptr += next;
      }
      return true;
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
	//LOG (INFO) << "Testing code";
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
	//LOG (INFO) << "Testing code";

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
