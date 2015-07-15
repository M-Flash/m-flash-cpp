#include "../src/core/graphbinarization.hpp"
#include "../src/core/mapped_stream.hpp"
#include "../log/easylogging++.h"
#include "../src/core/splitterbuffer.hpp"


//INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){
	//std::string graph_file = "/hugo/datasets/tiny-graph/tiny-graph";


	//std::string graph_file = "/hugo/datasets/twitter/twitter";


	/*
	 * YAHOO
	 */
	std::string graph_file = "/run/media/hugo/hugo/datasets/lj/.M-FLASH/lj/0_0.block";
	int64 edges = 6636600779;
	int64 max_vertex_id = 1413511393;

	std::ofstream file;
	file.open(graph_file + ".txt");

	MappedStream stream (graph_file);

	file<<max_vertex_id << " "<<edges<<std::endl;
	int from, to, idx;
	while(stream.has_remain()){
		from = stream.next_int();
		to = stream.next_int();
		file<<from << " "<<to<<std::endl;
	}
	file.close();
	stream.close_stream();
	return 0;
}
