/*
 * preprocessor.hpp
 *
 *  Created on: Jun 29, 2015
 *      Author: hugo
 */

#ifndef SRC_PREPROCESSOR_HPP_
#define SRC_PREPROCESSOR_HPP_

#include <cassert>
#include <string>

#include "../../log/easylogging++.h"
#include "mapped_stream.hpp"
#include "splitterbuffer.hpp"

namespace mflash{


template <class IdType>
class EdgeConversor{
	public:
		template <class Splitter>
		static void process (const std::string file_graph, const char separator, const bool edgelist, Splitter &splitter);
};

template <class IdType>
template <class Splitter>
void EdgeConversor<IdType>::process(const std::string file_graph, const char separator, const bool edgelist, Splitter &splitter){
	MappedStream in(file_graph);

	IdType v1 = 		0;
	IdType v2 = 		0;
	bool isQuantity = !edgelist;
	bool isInVertice = 	true;

	const char comment = '#';
	const char comment2 = '%';
	const char end_line = '\n';
	const char end_line2 = '\r';
	//char separator = ' ';
	const char separator2 = '\t';
	const char separator3 = ' ';

	EmptyField value;

/*

	const int64 MEGABYTE = 1024 * 1024;
	const int64 STEP_INFO = 500 * MEGABYTE;
*/

	while(in.has_remain()){
		char b = in.next();
		/*if( (in.current_ptr-in.ptr) % STEP_INFO ==0){
			LOG(INFO)<<"Processed: " << (in.current_ptr-in.ptr) / MEGABYTE << "MB";
		}*/
		//removing comment line
	  if(b == comment || b == comment2){
		while(b != end_line){
		  b = in.next();
		}
		b = in.next();
		//when the line content \n\r
		while( b == end_line || b==end_line2){
			b = in.next();
		}
		in.set_position(in.position()- sizeof(char));
		continue;
	  }
	  if (b == separator || b == separator2 || b == separator3){
		if(!isInVertice && !isQuantity){ //
			splitter.add(v1, v2, &value);
		}else{
			if(isInVertice) isInVertice = false;
			else if(isQuantity) isQuantity = false;
		}
		v2 = 0;
		continue;
	  }
		if (b == end_line || b == end_line2){
			b = in.next();
			//when the line content \r\n
			while( b == end_line || b==end_line2){
				b = in.next();
			}
			in.set_position(in.position()- sizeof(char));

			if(!isQuantity || edgelist){
				splitter.add(v1, v2, &value);
			}
			v1 = 0;
			v2 = 0;
			isInVertice = true;
			isQuantity = !edgelist;
			continue;
		}
		if(b<48 || b > 57){
			LOG(ERROR) << "The character '" << b << "' was not recognized.";
			assert(false);
		}

		if(isInVertice){
			v1 = (v1<< 3) + (v1 << 1) + (0xF & b);
		}else{
			v2 = (v2<< 3) + (v2 << 1) + (0xF & b);
		}
	}
	in.close_stream();
	splitter.flush();
	LOG(INFO)<<"Graph Binarization was succesfully";
}


}


#endif /* SRC_PREPROCESSOR_HPP_ */
