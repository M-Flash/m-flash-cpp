
#include "../src/core/graphbinarization.hpp"
#include "../src/core/mapped_stream.hpp"
#include "../log/easylogging++.h"
#include "../src/core/edgeconversor.hpp"
#include "../src/core/splitter_buffer_block_counting.hpp"
#include "../src/core/splitter_buffer_extended.hpp"

#include "../src/mflash_basic_includes.hpp"

using namespace mflash;
int main(int argc, char* argv[]){
	mflash_init(argc, argv);
	std::cout<<mflash::get_config_option_string("filetype")<<std::endl;
	std::cout<<mflash::get_config_option_string("memorysize")<<std::endl;
	//TYNY GRAPH
/*
	std::string graph_file = "/hugo/datasets/tiny-graph/tiny-graph";
	int64 vertices_partition = 3;
	int64 buffer_size = 8 * 100;
*/


/*

	//YAHOO
	std::string graph_file = "/run/media/hugo/data/datasets/yahoo";
	int64 vertices_partition = 1073741824;
	int64 buffer_size = 1024L*1024*1024 * 8;
	bool edgelist = false;
	char separator = ' ';
*/



/*

	//Twitter
	std::string graph_file = "/run/media/hugo/data/datasets/twitter";
	int64 vertices_partition = 41652231;
	int64 buffer_size = 1024L*1024*1024 * 8;
	bool edgelist = true;
	char separator = ' ';

*/


	//LiveJournal
	std::string graph_file = "/run/media/hugo/hugo/datasets/lj/lj";
	int64 vertices_partition = 4847573;
	int64 buffer_size = 1024L*1024*1024 * 8;
	bool edgelist = true;
	char separator = '\t';

	//MatrixProperties pr= load_matrix_properties(graph_file);


	LOG(INFO) << "==== PREPROCESSING ==== ";
	LOG(INFO) << "==== DIVIDING IN PARTITIONS ==== ";
	int64 edge_data_size = 0;
//std::string graph, int64 edge_data_size, int64 buffer_size, int64 ids_by_partitions, bool in_split = true,  std::string file_prefix = "", int64 partitions = 0);
	SplitterBufferWithBlockCounting<int> *splitter = new SplitterBufferWithBlockCounting<int> (graph_file, edge_data_size, buffer_size, vertices_partition, true, "tmp");
	EdgeConversor<int>::process(graph_file, separator, edgelist, *splitter );
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
	return 0;
}
