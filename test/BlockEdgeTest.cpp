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
	graph_file = "/data/hugo-data/hugo-others/datasets/.M-FLASH/lj/0.partition.output";
	MappedStream stream (graph_file);

	int32 from, to;
	//float value;
	int32 mfrom = 100000, mto = 100000;

	while(stream.has_remain()){
		from = stream.next_int();
		to = stream.next_int();
		//value = *((float*)stream.next(4, 0));
		if(from <= mfrom){
		    mfrom = from;
		    if(to < mto){
		        mto = to;
		    }
		}
	}
	stream.close_stream();
	std::cout<<mfrom << " "<< mto<<" "<</*value<<*/std::endl;

	return 0;
}
