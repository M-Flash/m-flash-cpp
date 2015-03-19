/*
 * matrix.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MATRIX_HPP_
#define MFLASH_CPP_CORE_MATRIX_HPP_

#include <algorithm>
#include <iostream>
#include <string>

#include "../log/easylogging++.h"
#include "array.hpp"
#include "blockiterator.hpp"
#include "direct_stream.hpp"
#include "mmappointer.hpp"
#include "operator.hpp"
#include "type.hpp"
#include "util.hpp"
#include "vector.hpp"

namespace mflash{

  template <class E>
  class MatrixWorker;

  template <class VSource, class VDestination, class E>
  class MAlgorithm;

  template <class E>
	class Matrix{

		string file;
		int64 n;
		int64 m;
		bool transpose_;

		int64 elements_by_block_;

		Mode mode;
		ElementIdSize element_id_size;
		MatrixWorker<E> *worker;

		//mode depends of the behaviour with one thread

	//	template<class V>
	//	BlockProperties** block_preprocessing(int block_count, Vector<V> &inVector, Vector<V> &outVector);

		BlockType get_block_type(int block_count, int64 vertex_size_bytes, int64 edge_size_bytes);

    template<class VSource, class VDestination>
    inline void operate(MAlgorithm<VSource,VDestination, E> &algorithm, Vector<VSource> *inVector, Vector<VDestination> *outVector);

		public:
			Matrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode);
			//Matrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode);
			int64 size(){return max(n,m);}
			Matrix<E> transpose();
			string get_file(){return file;}
			bool is_transpose(){return transpose_;}
			ElementIdSize get_element_id_size(){return element_id_size;}
			int64 get_elements_by_block(){return elements_by_block_;}

			int64 size_edge();
			int64 size_edge_data();

			bool add_field(FieldType type, string fieldname, AbstractVector &vector){return worker->add_field(type, fieldname, vector);}
			//bool add_field(FieldType type, string fieldname, AbstractVector *vector){return worker->add_field(type, fieldname, *vector);}

			bool remove_field(FieldType type, string fieldname){return worker->remove_field(type, fieldname);};


	    template<class VSource, class VDestination>
			void operate(MAlgorithm<VSource, VDestination,E> &algorithm, Vector<VSource> &inVector, Vector<VDestination> &outVector);

	    template<class VDestination>
	    void operate(MAlgorithm<EmptyField,VDestination, E> &algorithm, Vector<VDestination> &outVector);

	    void operate(MAlgorithm<EmptyField,EmptyField, E> &algorithm);


	};

	template<class E>
	Matrix<E>::Matrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode){
		this->file = file;
		this->n = size;
		this->m = size;
		this->transpose_ = !transpose;
		this->mode = mode;
		this->elements_by_block_ = element_by_block;
		this->element_id_size = ElementIdSize::SIMPLE;
		this->worker = new MatrixWorker<E>(*this);
	}

	template<class E>
	Matrix<E> Matrix<E>::transpose(){
		Matrix<E> t(this);
		t.transpose_ = !t.transpose;
		return (t);
	}

  template<class E>
  int64 Matrix<E>::size_edge_data(){
    int64 size = 0;
     #if E != EmptyType
      size = sizeof(E);
     #endif
    return size;
  }

  template<class E>
  int64 Matrix<E>::size_edge(){
    int64 size = 0;

    if(ElementIdSize::SIMPLE){
        size += 2*sizeof(int);
    }else{
        size += 2*sizeof(int64);
    }

    //We are considering always the format (source_id, destination_id, edge_data)

    return size;
  }
	//first implementation without replicates :)


  template<class E>
    template<class VSource, class VDestination>
    inline void Matrix<E>::operate(MAlgorithm<VSource,VDestination, E> &algorithm, Vector<VSource> *inVector, Vector<VDestination> *outVector){

      worker->set_default_destination_field(0);
      worker->set_default_source_field(0);

      worker->set_default_source_field(inVector);
      worker->set_default_destination_field(outVector);

      worker->operate<VSource, VDestination>(algorithm);

      worker->set_default_destination_field(0);
      worker->set_default_source_field(0);
    }

	template<class E>
	template<class VSource, class VDestination>
	inline void Matrix<E>::operate(MAlgorithm<VSource,VDestination, E> &algorithm, Vector<VSource> &inVector, Vector<VDestination> &outVector){
	  operate<VSource, VDestination>(algorithm, &inVector, &outVector);
	}

	template<class E>
  template<class VDestination>
  inline void Matrix<E>::operate(MAlgorithm<EmptyField,VDestination, E> &algorithm, Vector<VDestination> &outVector){
	  operate<EmptyField, VDestination>(algorithm, 0, &outVector);
  }

	template<class E>
  inline void Matrix<E>::operate(MAlgorithm<EmptyField,EmptyField, E> &algorithm){
    operate<EmptyField, EmptyField>(algorithm, 0, 0);
  }

}
#endif /* MFLASH_CPP_CORE_MATRIX_HPP_ */
