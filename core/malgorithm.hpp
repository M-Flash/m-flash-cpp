/*
 * malgorithm.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MALGORITHM_HPP_
#define MFLASH_CPP_CORE_MALGORITHM_HPP_

#include "type.hpp"
#include "matrixworker.hpp"

namespace mflash{


  template <class VSource, class VDestination, class E>
	class MAlgorithm{
		public:

	    /**
	     * Called before start the processing of each column to initialize the values of the output or the accumulators.
	     */
			virtual void initialize(MatrixWorker<E> &worker, Element<VDestination> &destination) = 0;

			/**
       * Called for each edge to transfers data since source to destination. On the destination is accumulated temporal state for the destination vertex,
       * if it is used multi-thread each thread maintain its own accumulator.
       */
			virtual void gather(MatrixWorker<E> &worker, Element<VSource> &source, Element<VDestination> &destination, E &edge_data) = 0;

			/**
			 * Called to combine accumulators between threads.
			 */
			virtual void sum(Element<VDestination> &accumulator1, Element<VDestination> &accumulator2, Element<VDestination> &out_accumulator) = 0;

			/**
			 * Called after combine the accumulators for each vertex to update the vertex value.
			 */
			virtual void apply(MatrixWorker<E> &worker, Element<VDestination> &out_element) = 0;

			/**
       *  Check is the initialized method is executed. By default is true.
       */
			virtual bool is_initialized(){
			  return true;
			}

			/**
       *  Check is the apply method is executed. By default is true.
       */
			virtual bool is_applied(){
			  return true;
			}

			/**
			 * Called before an iteration starts.
			 */
			virtual void before_iteration(int iteration, MatrixWorker<E> &worker){

			}

			/**
			 * Called after an iteration has finished.
			 */
      virtual void after_iteration(int iteration, MatrixWorker<E> &worker){
      }

	};

}


#endif /* MFLASH_CPP_CORE_MALGORITHM_HPP_ */
