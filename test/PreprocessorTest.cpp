#include "../src/core/splitterbuffer.hpp"
#include "../src/core/graphbinarization.hpp"
#include "../src/core/mapped_stream.hpp"
#include "../src/core/preprocessor.hpp"
#include "../log/easylogging++.h"


INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){
	std::string graph_file = "/hugo/datasets/yahoo.txt";
	SplitterBuffer<int> splitter(graph_file, 0, 134217728, 8589934592);
	Preprocessor<int>::process(graph_file, ' ', false , splitter );
	return 0;
}
