#include <cstring>
#include <string>
#include <vector>
#include <iostream>

#include "../src/mflash_basic_includes.hpp"
#include "../src/core/mapped_stream.hpp"

//INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){
	std::string graph_file;
	graph_file = "/data/hugo-data/hugo-git/projects/m-flash-cpp/test/.M-FLASH/sparse-graph/0_0.block";
	MappedStream stream (graph_file);

	int from, to;
	while(stream.has_remain()){
		from = stream.next_int();
		to = stream.next_int();
		std::cout<<from << " "<< to<<std::endl;
	}
	stream.close_stream();

	return 0;
}
