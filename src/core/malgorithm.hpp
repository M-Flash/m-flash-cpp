/*
 * malgorithm.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MALGORITHM_HPP_
#define MFLASH_CPP_CORE_MALGORITHM_HPP_

#include "../core/matrixworker.hpp"
#include "../core/type.hpp"

namespace mflash {

template<class VSource, class VDestination, class E, class IdType>
class MAlgorithm {
public:

	/**
	 * Called before start the processing of each edge to initialize the values to transfer using edges.
	 *
	 */
	virtual void initialize_source(MatrixWorker<E, IdType> &worker,
			Element<VSource, IdType> &destination) {
	}

	/**
	 * Called before start the processing of each each to initialize the values of the output or the accumulators.
	 */
	virtual void initialize(MatrixWorker<E, IdType>  &worker,
			Element<VDestination, IdType> &destination) = 0;

	/**
	 * Called for each edge to transfers data since source to destination. On the destination is accumulated temporal state for the destination vertex,
	 * if it is used multi-thread each thread maintain its own accumulator.
	 */
	virtual void gather(MatrixWorker<E, IdType>  &worker, Element<VSource> &source,
			Element<VDestination, IdType> &destination, E &edge_data) = 0;

	/**
	 * Called to combine accumulators between threads.
	 */
	virtual void process(Element<VDestination> &accumulator1,
			Element<VDestination, IdType> &accumulator2,
			Element<VDestination, IdType> &out_accumulator) = 0;

	/**
	 * Called after combine the accumulators for each vertex to update the vertex value.
	 */
	virtual void apply(MatrixWorker<E, IdType>  &worker,
			Element<VDestination, IdType> &out_element) = 0;

	/**
	 *  Check is the initialized method is executed. By default is false and MFlash uses the current value stored.
	 *  When it is true, it is executed before iteration starts and after before_iteration method.
	 *
	 */
	virtual bool is_source_initialized() {
		return false;
	}

	/**
	 *  Check is the initialized method is executed. By default is true.
	 */
	virtual bool is_initialized() {
		return true;
	}

	/**
	 *  Check is the apply method is executed. By default is true.
	 */
	virtual bool is_applied() {
		return true;
	}

	/**
	 * Called before an iteration starts.
	 */
	virtual void before_iteration(int iteration, MatrixWorker<E, IdType>  &worker) {

	}

	/**
	 * Called after an iteration has finished.
	 */
	virtual void after_iteration(int iteration, MatrixWorker<E, IdType>  &worker) {
	}

	virtual ~MAlgorithm() {
	}
};

}

#endif /* MFLASH_CPP_CORE_MALGORITHM_HPP_ */
