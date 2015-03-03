/*
 * streamprocessor.hpp
 *
 *  Created on: Mar 2, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_STREAMPROCESSOR_HPP_
#define MFLASH_CPP_CORE_STREAMPROCESSOR_HPP_

#include <string>
#include "stream.hpp"
#include "matrixworker.hpp"


namespace mflash{

	template <class V, class E>
	class MatrixWorker;

	template <class V, class E>
	class StreamProcessor{
		protected:
			Stream *stream;
			MatrixWorker<V,E> *worker;
			int id;


		public:
			StreamProcessor(string file, MatrixWorker<V,E> &worker, int id){
				stream = new Stream(file);
				this->worker = &worker;
				this->id = id;
			}
			virtual void call() = 0;

			~StreamProcessor(){
					delete stream;
					delete worker;
			}
	};

}

#endif /* MFLASH_CPP_CORE_STREAMPROCESSOR_HPP_ */
