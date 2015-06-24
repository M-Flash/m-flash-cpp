/*
 * graphformatter.h
 *
 *  Created on: May 6, 2015
 *      Author: hugo
 */

#ifndef CORE_GRAPHFORMATTER_HPP_
#define CORE_GRAPHFORMATTER_HPP_

#include <string>
#include <fstream>

#include "../core/mapped_stream.hpp"
#include "../core/type.hpp"

namespace mflash{

class GraphFormatter {
public:
	GraphFormatter();
	virtual ~GraphFormatter();

	/**
	 *
	 */
	static void execute(std::string graph_file, char separator, GraphFormatterMode mode, int64 initial_id);
};



void static GraphFormatter::execute(std::string graph_file, char separator, GraphFormatterMode mode, int64 initial_id){

	MappedStream in_stream(graph_file);
	int64 fsize = file_size(graph_file);
	ofstream out_stream(graph_file, get_file_properties(graph_file, true));

	if (!out_stream.good()) {
		out_stream.close();
		return;
	}

	out_stream.close();



}


}
#endif /* CORE_GRAPHFORMATTER_HPP_ */
