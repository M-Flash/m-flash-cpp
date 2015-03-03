/*
 * malgorithm.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MALGORITHM_HPP_
#define MFLASH_CPP_CORE_MALGORITHM_HPP_

#include "matrixworker.hpp"
#include "type.hpp"

namespace mflash{
	template <class V, class E>
	class MAlgorithm{
		public:
			virtual void initialize(MatrixWorker<V, E> &worker, Element<V> &out_element) ;
			virtual void gather(MatrixWorker<V, E> &worker, Element<V> &in_element, Element<V> &out_element, E &edge_data);
			virtual void sum(Element<V> &accumulator1, Element<V> &accumulator2, Element<V> &out_accumulator);
			virtual void apply(MatrixWorker<V, E> &worker, Element<V> &out_element) ;
			virtual bool isInitialized();
			virtual bool isApplied();
	};
}


#endif /* MFLASH_CPP_CORE_MALGORITHM_HPP_ */
