#include <cstring>
#include <string>
#include <vector>
#include <iostream>

#include "../log/easylogging++.h"
#include "../src/core/mapped_stream.hpp"

//INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){
	std::string graph_file;


	int64 vertices_by_partition;
	int64 vertices_graph;
	int64 partitions = 200;
	//YAHOO GRAPH
	graph_file = "/run/media/hugo/hugo/datasets/yahoo1/0.partition";
	vertices_graph = 1413511393;
	vertices_by_partition = 268435456;
	partitions = vertices_graph /vertices_by_partition+ (vertices_graph %vertices_by_partition != 0? 1:0);

	//TWITTER
/*

	graph_file = "/hugo/datasets/twitter/.M-FLASH/0.partition";
	vertices_graph = 41652231;

*/

	//LJ
/*
	graph_file = "/hugo/datasets/lj/.M-FLASH/0.partition";
	vertices_graph = 4847573;
*/


	//vertices_by_partition = vertices_graph /partitions + (vertices_graph %partitions != 0? 1:0);


	MappedStream stream (graph_file);

	std::vector<int64> counters (partitions * partitions);


	memset(counters.data(), 0, sizeof(int64) * partitions * partitions);
	int max_id = 0;
	int from, to, idx;
	while(stream.has_remain()){
		from = stream.next_int();
		to = stream.next_int();
		idx = ((int64)( from/vertices_by_partition)) * partitions+ (int64)(to/ vertices_by_partition);
		counters[idx]++;
		max_id = max(max(from,to), max_id);
	}

	std::cout<<"MAX ID = "<<max_id<<std::endl;
	for(int i = 0 ; i< partitions; i++){
		for(int j = 0 ; j< partitions; j++){
			std::cout<< counters[i*partitions + j]<<" ";
		}
		std::cout<<std::endl;
	}

	stream.close_stream();

	return 0;
}
