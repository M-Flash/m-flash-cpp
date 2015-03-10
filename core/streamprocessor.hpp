/*
 * streamprocessor.hpp
 *
 *  Created on: Mar 2, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_STREAMPROCESSOR_HPP_
#define MFLASH_CPP_CORE_STREAMPROCESSOR_HPP_

#include <string>

#include "mapped_stream.hpp"
#include "matrixworker.hpp"
#include "type.hpp"


namespace mflash{

	template <class V, class E>
	class MatrixWorker;

	template <class V, class E>
	class StreamProcessor{
		protected:
			MappedStream *stream;
			MatrixWorker<V,E> *worker;
			int id;
			BlockProperties *properties;


		public:
			StreamProcessor(string file, BlockProperties &properties, MatrixWorker<V,E> &worker, int id){
				stream = new MappedStream(file, properties.offset, properties.size);
				this->worker = &worker;
				this->id = id;
				this->properties = &properties;
			}
			virtual void call() = 0;

			~StreamProcessor(){
					delete stream;
			}
	};

}

#endif /* MFLASH_CPP_CORE_STREAMPROCESSOR_HPP_ */
