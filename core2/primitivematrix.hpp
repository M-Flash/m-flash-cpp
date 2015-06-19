/*
 * primitivematrix.hpp
 *
 *  Created on: Mar 3, 2015
 *      Author: hugo
 */

#ifndef CORE2_PRIMITIVEMATRIX_HPP_
#define CORE2_PRIMITIVEMATRIX_HPP_

#include "../core2/matrix.hpp"
#include "../core2/primitivevector.hpp"


namespace mflash{

  template <class VSource, class VDestination, class E>
  class SpMVMAlgorithmPrimitiveMatrix;


	template <class E>
	class PrimitiveMatrix : public Matrix<E>{

		public:
			PrimitiveMatrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode) : Matrix<E>(file, size, transpose, element_by_block, mode){}

			template<class V>
			void multiply(PrimitiveVector<V> &inVector, PrimitiveVector<V> &outVector);
	};

	template<class E>
	template<class V>
	inline void PrimitiveMatrix<E>::multiply(PrimitiveVector<V> &inVector, PrimitiveVector<V> &outVector){
    SpMVMAlgorithmPrimitiveMatrix<V,V,E> spvm;
    Matrix<E>::operate(spvm, inVector, outVector);
	}



	template <class VSource, class VDestination, class E>
	class SpMVMAlgorithmPrimitiveMatrix : public MAlgorithm<VSource, VDestination,E>{
		inline void initialize(MatrixWorker<E> &worker, Element<VDestination> &out_element){
			*(out_element.value) = 0;
		}
		inline void gather(MatrixWorker<E> &worker, Element<VSource> &in_element, Element<VDestination> &out_element, E &edge_data){
      #if (E == EmptyType)
          *(out_element.value) += *(in_element.value);
      #else
          *(out_element.value) += *(in_element.value) * edge_data;
      #endif
		}
		inline  void sum(Element<VDestination> &accumulator1, Element<VDestination> &accumulator2, Element<VDestination> &out_accumulator){
			*(out_accumulator.value) = *(accumulator1.value) + *(accumulator2.value);
		}
		inline  void apply(MatrixWorker<E> &worker, Element<VDestination> &out_element) {}
		inline  bool is_initialized(){
			return true;
		}
		inline  bool is_applied(){
			return false;
		}

	};
}


#endif /* CORE2_PRIMITIVEMATRIX_HPP_ */
