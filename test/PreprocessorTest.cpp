#include "../src/core/splitterbufferextended.hpp"
#include "../src/core/graphbinarization.hpp"
#include "../src/core/mapped_stream.hpp"
#include "../src/core/preprocessor.hpp"
#include "../log/easylogging++.h"


INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){
	//TYNY GRAPH
	/*std::string graph_file = "/hugo/datasets/tiny-graph/tiny-graph";
	int64 vertices_partition = 3;
	int64 buffer_size = 8 * 100;*/

	//YAHOO
	std::string graph_file = "/hugo/datasets/yahoo.txt";
	int64 vertices_partition = 268435456;
	int64 buffer_size = 1024L*1024*1024 * 4;

	SplitterBufferExtended<int> splitter(graph_file, 0, vertices_partition, buffer_size, true);
	Preprocessor<int>::process(graph_file, ' ', false , splitter );
	splitter.printCounters();
	return 0;
}
