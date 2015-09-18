// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef SRC_EDGELISTCONVERSOR_HPP_
#define SRC_EDGELISTCONVERSOR_HPP_

#include <cassert>
#include <string>

#include "../util/easylogging++.h"
#include "mapped_stream.hpp"
#include "splitterbuffer.hpp"

namespace mflash{



/**
 * Conversor for processing file graphs with special edge list format. First line with number of nodes.
 * The first line in the file is the total number of nodes.
 * Following is one line of outlinks (target nodes) for each node ID, starting with node 0.
 * Any target node IDs will be space-separated on the line for that node.
 * If a node does not have any outlinks associated with it, the line will be blank.
 * See http://www.lemurproject.org/clueweb12/webgraph.php/
 *
 */
template <class IdType>
class EdgeListConversor{
	public:
		template <class Splitter>
		static void process (const std::string file_graph, const char separator, Splitter &splitter);
};

template <class IdType>
template <class Splitter>
void EdgeListConversor<IdType>::process(const std::string file_graph, const char separator, Splitter &splitter){
	MappedStream in(file_graph);

	IdType v1 = 		0;
	IdType v2 = 		0;

	const char comment = '#';
	const char comment2 = '%';
	const char end_line = '\n';
	const char end_line2 = '\r';
	//char separator = ' ';
	const char separator2 = '\t';
	const char separator3 = ' ';

	bool first_line = true;
	EmptyField value;

	while(in.has_remain()){
		char b = in.next_char();
		/*if( (in.current_ptr-in.ptr) % STEP_INFO ==0){
			LOG(INFO)<<"Processed: " << (in.current_ptr-in.ptr) / MEGABYTE << "MB";
		}*/
	 //removing comment line or first line with edges
	  if(b == comment || b == comment2 || first_line){
		if(first_line){
		  first_line = false;
		}
		while(b != end_line){
		  b = in.next_char();
		}
		b = in.next_char();
		//when the line content \n\r
		while( b==end_line2){
			b = in.next_char();
		}
		in.set_position(in.position()- sizeof(char));
		continue;
	  }
	  if (b == separator || b == separator2 || b == separator3){
		splitter.add(v1, v2, &value);
		v2 = 0;
		continue;
	  }
		if (b == end_line || b == end_line2){
			b = in.next_char();
			//when the line content \r\n
			while( b==end_line2){
				b = in.next_char();
			}
			in.set_position(in.position()- sizeof(char));

			if(v2 != 0){
				splitter.add(v1, v2, &value);
			}
			v1++;
			v2 = 0;
			continue;
		}
		if(b<48 || b > 57){
			LOG(ERROR) << "The character '" << b << "' was not recognized.";
			assert(false);
		}
		v2 = (v2<< 3) + (v2 << 1) + (0xF & b);

	}
	in.close_stream();
	splitter.flush();
	LOG(INFO)<<"Graph Binarization was succesfully";
}

}


#endif /* SRC_PREPROCESSOR_HPP_ */
