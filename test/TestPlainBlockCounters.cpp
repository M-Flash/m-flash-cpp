#include <cstring>
#include <string>
#include <vector>
#include <iostream>

#include "../log/easylogging++.h"
#include "../src/core/mapped_stream.hpp"


INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){
	std::string graph_file = "/hugo/datasets/.M-FLASH/0.partition";
	MappedStream stream (graph_file);

	int vertices_by_partition = 134217728;
	int vertices_graph = 1413511392;
	int partitions = vertices_graph/vertices_by_partition + 1;

	std::vector<int64> counters (partitions * partitions);


	memset(counters.data(), 0, sizeof(int64) * partitions * partitions);

	int from, to, idx;
	while(stream.has_remain()){
		from = stream.next_int();
		to = stream.next_int();
		idx = ( (int64)from/vertices_by_partition) * partitions+ (to/ vertices_by_partition);
		counters[idx]++;
	}

	for(int i = 0 ; i< partitions; i++){
		for(int j = 0 ; j< partitions; j++){
			std::cout<< counters[i*partitions + j]<<" ";
		}
		std::cout<<std::endl;
	}

	stream.close_stream();

	return 0;
}
