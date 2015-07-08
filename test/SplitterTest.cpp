#include "../src/core/splitterbuffer.hpp"
#include "../src/core/graphbinarization.hpp"
#include "../src/core/mapped_stream.hpp"
#include "../log/easylogging++.h"


INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){
	//std::string graph_file = "/hugo/datasets/tiny-graph/tiny-graph";
	//std::string graph_file = "/hugo/datasets/twitter/twitter";
	std::string graph_file = "/hugo/datasets/lj/lj";
	//SplitterBuffer<int> splitter(graph_file, 0, 11, 24);
	SplitterBuffer<int> splitter(graph_file, 0, 4847573, 8589934592);
	convert_edgelist<EmptyField, int>(graph_file, splitter );
	return 0;
}
