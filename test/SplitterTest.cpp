#include "../src/core/splitterbuffer.hpp"
#include "../src/core/graphbinarization.hpp"
#include "../src/core/mapped_stream.hpp"
#include "../log/easylogging++.h"


INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){
	//std::string graph_file = "/hugo/datasets/tiny-graph/tiny-graph";
	std::string graph_file = "/hugo/datasets/yahoo.txt";
	//SplitterBuffer<int> splitter(graph_file, 0, 3, 24);
	SplitterBuffer<int> splitter(graph_file, 0, 134217728, 8589934592);
	convert_adjlist<EmptyField, int>(graph_file, splitter );
	return 0;
}
