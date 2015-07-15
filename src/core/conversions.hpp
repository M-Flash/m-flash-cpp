/*
 * conversions.hpp
 *
 *  Created on: Jul 15, 2015
 *      Author: hugo
 */

#ifndef SRC_CORE_CONVERSIONS_HPP_
#define SRC_CORE_CONVERSIONS_HPP_

#include "../mflash_basic_includes.hpp"

namespace mflash{

	template <class IdType, class V, class E>
	static void convert<IdType>::(const std::string file_graph){
		std::string file_type_str = get_config_option_string("filetype", "null");
		int64 buffer_size = get_config_option_long("memorysize", DEFAULT_MEMORY_SIZE);
		int64 vertices_by_partition = get_config_option_long("memorysize", DEFAULT_MEMORY_SIZE);
		int64 edge_data_size = mflash::getEdgeSize<E>();


		if(file_type_str == "null"){
			file_type_str = get_option_string_interactive("filetype", "edgelist, adjlist, binedgelist");
		}

		if (file_type_str != "adjlist" && file_type_str != "edgelist"  && file_type_str != "binedgelist") {
			LOG(ERROR) << "You need to specify filetype: 'edgelist',  'adjlist', or 'binedgelist'." << std::endl;
			assert(false);
		}

		SplitterBufferWithBlockCounting<IdType> *splitter = new SplitterBufferWithBlockCounting<int> (graph_file, edge_data_size, buffer_size, vertices_by_partition,  , "tmp");
		if (file_type_str == "adjlist"){
			convert_adjlist<EdgeDataType, FinalEdgeDataType>(basefilename, sharderobj);
		} else if (file_type_str == "edgelist") {
			convert_edgelist<EdgeDataType, FinalEdgeDataType>(basefilename, sharderobj);
		} else if (file_type_str == "binedgelist") {
			convert_binedgelistval<EdgeDataType, FinalEdgeDataType>(basefilename, sharderobj);
		} else if (file_type_str == "metis") {
			convert_metis<EdgeDataType, FinalEdgeDataType>(basefilename, sharderobj);
		} else {
			assert(false);
		}





		/*
		 * Conversion since file-type format to binary partitions
		 */

		SplitterBufferWithBlockCounting<IdType> *splitter =
		EdgeConversor<IdType>::process(graph_file, separator, edgelist, *splitter );
		std::vector<int64> counters = splitter->getCounters();
		int64 partitions = splitter->getPartitions();
		int64 max_id = splitter->getMaxId();
		delete splitter;
		/*int64 partitions = 6;
			int64 counters []= {4673832906,62346369,49286233,48480947,49027957,13984090,404477469,28374192,4502729,3895090,4420293,1325284,
										 363359757,4482796,28754309,4151632,4624383,1044833,
										 331969198,3990545,4273130,27070108,4046738,966799,
										 367594504,4243111,4583306,4095013,28554079,1036922,
										 92985015,1288797,1145679,1109580,1137007,6139979};
		*/

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
				int from, to;

				std::vector<BlockType> block_types(partitions);
				//checking block density
				double ratio;
				int64 edge_size = 2 * sizeof(int) + edge_data_size;
				for (int64 j = 0; j < partitions; j++){
					ratio = ((double)1)/partitions + ( ((double)2) * counters[i * partitions + j] * edge_size / vertices_partition);
					block_types[j] = ratio<1? BlockType::SPARSE: BlockType::DENSE;
				}
				SplitterBufferExtended<int> psplitter (graph_file, 0, buffer_size, vertices_partition, false, i, block_types);

				EmptyField edge_data_type;
				while(stream.has_remain()){
					from = stream.next_int();
					to = stream.next_int();
					psplitter.add(from, to, &edge_data_type);
				}
				psplitter.flush();
				stream.close_stream();
				//remove tmp file
				//delete_file(tmp_partition);
			}
		}
		LOG(INFO) << "==== PREPROCESSING FINISHED ==== ";
		MatrixProperties properties(max_id+1, partitions, vertices_partition, counters.data());
		update_matrix_properties(graph_file, properties);




		if (file_type_str == "adjlist") {
			convert_adjlist<EdgeDataType, FinalEdgeDataType>(basefilename, sharderobj);
		} else if (file_type_str == "edgelist") {
			convert_edgelist<EdgeDataType, FinalEdgeDataType>(basefilename, sharderobj);
		} else if (file_type_str == "binedgelist") {
			convert_binedgelistval<EdgeDataType, FinalEdgeDataType>(basefilename, sharderobj);
		} else if (file_type_str == "metis") {
			convert_metis<EdgeDataType, FinalEdgeDataType>(basefilename, sharderobj);
		} else {
			assert(false);
		}

	}


}

#endif /* SRC_CORE_CONVERSIONS_HPP_ */
