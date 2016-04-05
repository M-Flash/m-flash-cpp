

#include "../src/mflash_basic_includes.hpp"

using namespace mflash;
int main(int argc, char* argv[]){

	std::string graph_file = "/data/hugo-data/hugo-others/datasets/.M-FLASH/powerlawgraph/powerlawgraph.bin";

	MappedStream stream (graph_file);

	int64 value = -1;
	int64 tmp = 0;
	int32 from, to;
	int64 edges = 0;
	while(stream.has_remain()){
	    from = stream.next<int32>();
	    to = stream.next<int32>();
	    value = (((int64)from) <<32) + to;
	    edges++;
	    if(tmp >= value){
	        LOG(ERROR)<<"Sorting error in edge "<< edges<< " : " << tmp << " > " << value << " (" <<from <<","<<to <<")";
	        return -1;
	    }
	    value = tmp;

	}

	return 0;
}
