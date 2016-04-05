
#include <stdio.h>
#include "../src/mflash_basic_includes.hpp"
#include "../src/core/genericedgesplittermanager.hpp"

//INITIALIZE_EASYLOGGINGPP

using namespace mflash;

int main(){

	std::string graph_file = "/data/hugo-data/hugo-others/datasets/lj";
	GenericEdgeSplitterManager<int32> emanager( 4294967296, true, 0);
    SplitterBuffer<int32, GenericEdgeSplitterManager<int32> , EmptyField> bsplitter (&emanager, graph_file, 0 ,mflash::DEFAULT_MEMORY_SIZE, "", false);
    EdgeConversor<int32, EmptyField>::process(graph_file, ' ', true, bsplitter );
	return 0;
}
