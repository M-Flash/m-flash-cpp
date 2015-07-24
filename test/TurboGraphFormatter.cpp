#include "../src/core/graphbinarization.hpp"
#include "../src/core/mapped_stream.hpp"
#include "../log/easylogging++.h"
#include "../src/core/splitterbuffer.hpp"
#include <stdio.h>

//INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){
	//std::string graph_file = "/hugo/datasets/tiny-graph/tiny-graph";

	//std::string graph_file = "/hugo/datasets/twitter/twitter";


	//LJ
	/*std::string graph_file = "/run/media/hugo/hugo/datasets/lj/.M-FLASH/lj/0_0.block";
	int64 edges = 68993773;
	int64 max_vertex_id = 4847573;
	*/

	//YAHOO
/*
	std::string graph_file = "/run/media/hugo/hugo/datasets/yahoo1/0.partition";
	int64 edges = 6636600779;
	int64 max_vertex_id = 1413511393;
*/


	//TWITTER

	std::string graph_file = "/run/media/hugo/hugo/datasets/twitter/.M-FLASH/0_0.block";
	int64 edges = 1468365182;
	int64 max_vertex_id = 41652231;


	FILE* fout = fopen("/run/media/hugo/data/frameworks/turbograph/Turbograph/KDDResults/data/twitter.txt", "w");

	MappedStream stream (graph_file);

	fprintf(fout, "%lu %lu\n", max_vertex_id, edges);
	int from, to, idx;
	while(stream.has_remain()){
		from = stream.next_int();
		to = stream.next_int();
		fprintf(fout, "%d %d\n", from, to);
	}
	fclose(fout);
	stream.close_stream();
	return 0;
}
