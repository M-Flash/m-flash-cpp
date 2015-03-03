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
			EdgeListThread(string file, MatrixWorker<V,E> &worker, int id) : StreamProcessor<V,E>(file, worker, id){}
			void call();
	};

	template <class V, class E>
	inline void EdgeListThread<V,E>::call(){
			E edge_data;
			const ElementIdSize id_size = this->worker->get_matrix().get_element_id_size();
			const bool transpose = this->worker->get_matrix().is_transpose();
			int bytes = id_size == ElementIdSize::SIMPLE? sizeof(int): sizeof(int64);
			int step = MFLASH_MATRIX_THREADS * bytes - bytes;

			this->stream->set_position (bytes * this->id);

			int64 in_vertex_id;
			int64 out_vertex_id;

			if(ElementIdSize::SIMPLE == id_size){
					if(transpose){
						while(this->stream->has_remain()){
							in_vertex_id = this->stream->next_int();
							out_vertex_id = this->stream->next_int(step);
							this->worker->next_edge(out_vertex_id, in_vertex_id, edge_data);
						}
					}else{
							while(this->stream->has_remain()){
								out_vertex_id = this->stream->next_int();
								in_vertex_id = this->stream->next_int(step);
								this->worker->next_edge(out_vertex_id, in_vertex_id, edge_data);
							}
					}
			}
			this->stream->close_stream();
		}
}

#endif /* MFLASH_CPP_CORE_EDGELISTTHREAD_HPP_ */
