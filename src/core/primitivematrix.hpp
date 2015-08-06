/*
 * primitivematrix.hpp
 *
 *  Created on: Mar 3, 2015
 *      Author: hugo
 */

#ifndef CORE_PRIMITIVEMATRIX_HPP_
#define CORE_PRIMITIVEMATRIX_HPP_

#include <string>

#include "malgorithm.hpp"
#include "matrix.hpp"
#include "matrixworker.hpp"
#include "primitivevector.hpp"
#include "type.hpp"


namespace mflash {

template<class V, class E, class IdType>
class SpMVMAlgorithmPrimitiveMatrix;

template<class E, class IdType>
class MatrixWorker;


template<class V, class E, class IdType>
class PrimitiveMatrix: public Matrix<E, IdType> {

public:
	PrimitiveMatrix(string file, bool transpose = false, Mode mode = Mode::VECTOR_REPLICATION) :
			Matrix<E, IdType>(file, transpose, mode) {
	}
	void multiply(PrimitiveVector<V, IdType> &inVector,PrimitiveVector<V, IdType> &outVector);
};

template <class V, class E, class IdType>
inline void PrimitiveMatrix<V, E, IdType>::multiply(
		PrimitiveVector<V, IdType> &inVector, PrimitiveVector<V, IdType> &outVector) {
	SpMVMAlgorithmPrimitiveMatrix < V, E, IdType > spvm;
	Matrix<E, IdType>::operate(spvm, inVector, outVector);
}

template<class V, class E, class IdType>
class SpMVMAlgorithmPrimitiveMatrix { //: public MAlgorithm<V, V, E, IdType> {
public:
	inline void initialize(MatrixWorker<E, IdType> &worker, Element<V, IdType> &out_element) {
		*(out_element.value) = 0;
	}
	inline void gather(MatrixWorker<E, IdType> &worker, Element<V, IdType> &in_element, Element<V, IdType> &out_element, E &edge_data) {
#if (E == EmptyType)
		*(out_element.value) += *(in_element.value);
#else
		*(out_element.value) += *(in_element.value) * edge_data;
#endif
	}
	inline void process(Element<V, IdType> &accumulator1, Element<V, IdType> &accumulator2, Element<V, IdType> &out_accumulator) {
		*(out_accumulator.value) = *(accumulator1.value) + *(accumulator2.value);
	}
	inline void apply(MatrixWorker<E, IdType> &worker, Element<V, IdType> &out_element) {}
	inline bool is_initialized() {return true;}
	inline bool is_applied() {return false;}

	inline bool is_destination_loaded(){return true;}
	inline bool is_source_loaded(){return true;}
	inline bool is_destination_stored(){return true;}

	inline void before_iteration(int iteration, MatrixWorker<E, IdType>  &worker) {}
	inline void after_iteration(int iteration, MatrixWorker<E, IdType>  &worker) {}

};
}

#endif /* CORE_PRIMITIVEMATRIX_HPP_ */
