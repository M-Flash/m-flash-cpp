/*
 * matrixworker.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MATRIXWORKER_HPP_
#define MFLASH_CPP_CORE_MATRIXWORKER_HPP_

#include "array.hpp"
#include "malgorithm.hpp"
#include "matrix.hpp"
#include "type.hpp"

namespace mflash{

	template <class V, class E>
	class MAlgorithm;


	template <class V, class E>
	class Matrix;


	template <class V, class E>
	class MatrixWorker{
			Element<V> out_vertex_accumulator;
			Element<V> in_vertex;
			MAlgorithm<V,E> *algorithm;
			Array<V> *in_array;
			Array<V> *out_array;
			Matrix<V,E> * matrix;

			//used to control when the matrix worker set the data or it is loaded for the stream processor.
			bool load_vertex_data;

			friend class EdgeListWriter<V,E>;
			friend class EdgeListThread<V,E>;

	  protected:
			Array<V>* get_in_array(){return in_array;}
			Array<V>* get_out_array(){return out_array;}

		public:
			MatrixWorker(MAlgorithm<V,E> *algorithm, Matrix<V,E> *matrix, Array<V> *in_array, Array<V> *out_array, bool load_vertex_data);
			MatrixWorker(MAlgorithm<V,E> *algorithm, Matrix<V,E> *matrix, Array<V> *in_array, Array<V> *out_array) : MatrixWorker(algorithm, matrix, in_array, out_array, true){}

			bool next_edge (int64 in_vertex_id, int64 out_vertex_id, E &edge_data);
			Matrix<V,E> get_matrix(){return *matrix;}

	};

	template <class V, class E>
	inline MatrixWorker<V,E>::MatrixWorker(MAlgorithm<V,E> *algorithm, Matrix<V,E> *matrix, Array<V> *in_array, Array<V> *out_array, bool load_vertex_data){
		this->algorithm = algorithm;
		this->matrix = matrix;
		this->out_array = out_array;
		this->in_array = in_array;
		this->load_vertex_data = load_vertex_data;
	}

	template <class V, class E>
	inline bool MatrixWorker<V,E>::next_edge(int64 in_vertex_id, int64 out_vertex_id, E &edge_data){
		out_vertex_accumulator.id = out_vertex_id;
		in_vertex.id = in_vertex_id;

		if(load_vertex_data){
		    in_vertex.value = in_array->get_element(in_vertex_id);
    }
		out_vertex_accumulator.value = out_array->get_element(out_vertex_id);
		algorithm->gather(*(this), in_vertex, out_vertex_accumulator, edge_data);
		//algorithm->sum(this, *in_vertex, *out_vertex_accumulator, edge_data);

		return false;
	}
}

#endif /* MFLASH_CPP_CORE_MATRIXWORKER_HPP_ */
