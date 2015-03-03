/*
 * primitivematrix.hpp
 *
 *  Created on: Mar 3, 2015
 *      Author: hugo
 */

#ifndef CORE_PRIMITIVEMATRIX_HPP_
#define CORE_PRIMITIVEMATRIX_HPP_

#include "matrix.hpp"

namespace mflash{
	template <class V, class E>
	class SpMVMAlgorithmPrimitiveMatrix;

	template <class V, class E>
	class PrimitiveMatrix : public Matrix<V,E>{
		public:
			void multiply(PrimitiveVector<V> &inVector, PrimitiveVector<V> &outVector);
	};

	template<class V, class E>
	void PrimitiveMatrix<V,E>::multiply(PrimitiveVector<V> &inVector, PrimitiveVector<V> &outVector){
		SpMVMAlgorithmPrimitiveMatrix<V,E> spvm;
		operate(spvm, inVector, outVector);
	}



	template <class V, class E>
		class SpMVMAlgorithmPrimitiveMatrix : public MAlgorithm<V,E>{
		inline void initialize(MatrixWorker<V, E> &worker, Element<V> &out_element){
			*(out_element.value) = 0;
		}
		inline void gather(MatrixWorker<V, E> &worker, Element<V> &in_element, Element<V> &out_element, E &edge_data){
			*(out_element.value) += *(in_element.value);
		}
		inline  void sum(Element<V> &accumulator1, Element<V> &accumulator2, Element<V> &out_accumulator){
			*(out_accumulator.value) = *(accumulat1.value) + *(accumulat2.value);
		}
		inline  void apply(MatrixWorker<V, E> &worker, Element<V> &out_element) {}
		inline  bool isInitialized(){
			return true;
		}
		inline  bool isApplied(){
			return false;
		}

	};

}

#endif /* CORE_PRIMITIVEMATRIX_HPP_ */
