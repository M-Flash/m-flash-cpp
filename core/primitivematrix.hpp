/*
 * primitivematrix.hpp
 *
 *  Created on: Mar 3, 2015
 *      Author: hugo
 */

#ifndef CORE_PRIMITIVEMATRIX_HPP_
#define CORE_PRIMITIVEMATRIX_HPP_

#include "matrix.hpp"
#include "primitivevector.hpp"

namespace mflash{
	template <class V, class E>
	class SpMVMAlgorithmPrimitiveMatrix;

	template <class V, class E>
	class SpMVMAlgorithmWeightedPrimitiveMatrix;

	template <class V, class E = EmptyType>
	class PrimitiveMatrix : public Matrix<V,E>{

		public:
			PrimitiveMatrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode) : Matrix<V,E>(file, size, transpose, element_by_block, mode){}
			void multiply(PrimitiveVector<V> &inVector, PrimitiveVector<V> &outVector);
	};

	template<class V, class E>
	inline void PrimitiveMatrix<V,E>::multiply(PrimitiveVector<V> &inVector, PrimitiveVector<V> &outVector){

	  //check if the E type is weighted
	  E tmp;
	  EmptyType * emptyType = dynamic_cast<EmptyType*> (&tmp);
	  if(emptyType != 0){
	      SpMVMAlgorithmPrimitiveMatrix<V,E> spvm;
	      this->operate(spvm, inVector, outVector);
	  }else{
	      SpMVMAlgorithmWeightedPrimitiveMatrix<V,E> spvmweighted;
	      this->operate(spvmweighted, inVector, outVector);
	  }


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
			*(out_accumulator.value) = *(accumulator1.value) + *(accumulator2.value);
		}
		inline  void apply(MatrixWorker<V, E> &worker, Element<V> &out_element) {}
		inline  bool isInitialized(){
			return true;
		}
		inline  bool is_applied(){
			return false;
		}

	};


	template <class V, class E>
    class SpMVMAlgorithmWeightedPrimitiveMatrix : public MAlgorithm<V,E>{
    inline void initialize(MatrixWorker<V, E> &worker, Element<V> &out_element){
      *(out_element.value) = 0;
    }
    inline void gather(MatrixWorker<V, E> &worker, Element<V> &in_element, Element<V> &out_element, E &edge_data){
      *(out_element.value) += *(in_element.value) * edge_data;
    }
    inline  void sum(Element<V> &accumulator1, Element<V> &accumulator2, Element<V> &out_accumulator){
      *(out_accumulator.value) = *(accumulator1.value) + *(accumulator2.value);
    }
    inline  void apply(MatrixWorker<V, E> &worker, Element<V> &out_element) {}
    inline  bool isInitialized(){
      return true;
    }
    inline  bool is_applied(){
      return false;
    }

  };
}

#endif /* CORE_PRIMITIVEMATRIX_HPP_ */
