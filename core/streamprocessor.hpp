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
#include "type.hpp"
#include "edgeprocessor.hpp"

namespace mflash{

  class StreamProcessor{
		protected:
			MappedStream *stream;
			int id;
			BlockProperties *properties;
			int64 edge_data_size;
			bool transpose;
			ElementIdSize *element_id_size;
			AbstractProcessor *edge_processor;

		public:
			StreamProcessor(string file, BlockProperties &properties, int64 edge_data_size, int id, bool transpose, ElementIdSize &element_id_size, AbstractProcessor *edge_processor){
				stream = new MappedStream(file, properties.offset, properties.size);
				this->edge_data_size = edge_data_size;
				this->id = id;
				this->properties = &properties;
				this->transpose = transpose;
				this->element_id_size = &element_id_size;
				this->edge_processor = edge_processor;
			}
			virtual void call() = 0;

			~StreamProcessor(){
					delete stream;
			}
	};

}

#endif /* MFLASH_CPP_CORE_STREAMPROCESSOR_HPP_ */
