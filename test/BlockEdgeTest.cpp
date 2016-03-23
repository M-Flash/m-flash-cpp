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
	graph_file = "/data/hugo-data/hugo-others/datasets/.M-FLASH/powerlawgraph/0.partition";
	MappedStream stream (graph_file);

	int32 from, to;
	float value;
	while(stream.has_remain()){
		from = stream.next_int();
		to = stream.next_int();
		value = *((float*)stream.next(4, 0));

		std::cout<<from << " "<< to<<" "<<value<<std::endl;
	}
	stream.close_stream();

	return 0;
}
