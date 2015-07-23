/*
 * conversions.hpp
 *
 *  Created on: Jul 15, 2015
 *      Author: hugo
 */

#ifndef SRC_CORE_CONVERSIONS_HPP_
#define SRC_CORE_CONVERSIONS_HPP_

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../../log/easylogging++.h"
#include "../util/cmdopts.hpp"
#include "edgeconversor.hpp"
#include "mapped_stream.hpp"
#include "splitter_buffer_block_counting.hpp"
#include "splitter_buffer_extended.hpp"
#include "splitterbuffer.hpp"
#include "type.hpp"
#include "util.hpp"

namespace mflash{

	struct MatrixProperties;

	template <class E = EmptyField, class IdType>
	MatrixProperties convert(const std::string graph_file, int vertex_size, std::string file_type_str = "null"){

		LOG(INFO) << "==== PREPROCESSING ==== ";
		if(file_type_str == "null"){
			file_type_str = get_config_option_string("filetype", "null");
		}

		int64 buffer_size = get_config_option_long("memorysize", DEFAULT_MEMORY_SIZE);
		int64 vertices_by_partition = getVeticesByPartition(vertex_size);
		int64 edge_data_size = mflash::getEdgeDataSize<E>();


		if(file_type_str == "null"){
			file_type_str = get_option_string_interactive("filetype", "edgelist, adjlist, binedgelist");
		}

		if (file_type_str != "adjlist" && file_type_str != "edgelist"  && file_type_str != "binedgelist") {
			LOG(ERROR) << "You need to specify filetype: 'edgelist',  'adjlist', or 'binedgelist'." << std::endl;
			assert(false);
		}

		LOG(INFO) << "==== DIVIDING IN PARTITIONS ==== ";
		clean_mflash_directory(graph_file);
		SplitterBufferWithBlockCounting<IdType> *splitter = new SplitterBufferWithBlockCounting<IdType> (graph_file, edge_data_size, buffer_size, vertices_by_partition, true , "tmp");

		if (file_type_str == "adjlist" || file_type_str == "edgelist"){
			EdgeConversor<IdType>::process(graph_file, ' ',  file_type_str == "edgelist" , *splitter );
		} else {
			LOG(ERROR) << "Support for "<< file_type_str  << " not implemented.";
			assert(false);
		}


		std::vector<int64> counters = splitter->getCounters();
		int64 partitions = splitter->getPartitions();
		int64 max_id = splitter->getMaxId();
		delete splitter;

		if(partitions == 1){
			LOG(INFO) << "==== DIVIDING IN BLOCKS OMITTED BECAUSE GRAPH HAS DIVIDED IN A SINGLE PARTITION ==== ";
			rename_file(get_partition_file(graph_file, 0, "tmp"), get_block_file(graph_file,0, 0));
		}else{
			LOG(INFO) << "==== DIVIDING IN BLOCKS ==== ";
			for (int64 i = 0; i < partitions; i++){
				LOG(INFO) << "==== DIVIDING PARTITION "<< i << "==== ";
				std::string tmp_partition = get_partition_file(graph_file, i, "tmp");
				if(!mflash::exist_file(tmp_partition))
					continue;
				MappedStream stream (tmp_partition);
				IdType from, to;

				std::vector<BlockType> block_types(partitions);
				//checking block density
				//int64 edge_size = 2 * sizeof(IdType) + edge_data_size;
				for (int64 j = 0; j < partitions; j++){
					//ratio = ((double)1)/partitions + ( ((double)2) * counters[i * partitions + j] * edge_size / vertices_by_partition);
					block_types[j] =  getBlockType<E, IdType>(partitions, vertices_by_partition, counters[i * partitions + j]); //ratio<1? BlockType::SPARSE: BlockType::DENSE;
				}
				SplitterBufferExtended<IdType> psplitter (graph_file, 0, buffer_size, vertices_by_partition, false, i, block_types);

				EmptyField edge_data_type;
				while(stream.has_remain()){
					from = stream.next<IdType>();
					to = stream.next<IdType>();
					psplitter.add(from, to, &edge_data_type);
				}
				psplitter.flush();
				stream.close_stream();
				//remove tmp file
				delete_file(tmp_partition);
			}
		}
		LOG(INFO) << "==== PREPROCESSING FINISHED ==== ";
		MatrixProperties properties(max_id+1, sizeof(IdType) ,partitions, vertices_by_partition, counters.data());
		update_matrix_properties(graph_file, properties);



		return properties;

	}


}

#endif /* SRC_CORE_CONVERSIONS_HPP_ */
