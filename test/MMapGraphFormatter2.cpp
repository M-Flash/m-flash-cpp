
#include <stdio.h>
#include "../src/mflash_basic_includes.hpp"
#include "../src/core/genericedgesplittermanager.hpp"
#include "../src/util/kwaymergesort.h"

//INITIALIZE_EASYLOGGINGPP

using namespace mflash;




// a basic struct for a Edge entry.
struct Edge {
    int32 from;
    int32 to;

    bool operator < (const Edge &b) const
    {

        if( (((int64)from) <<32) + to < (((int64)b.from) <<32) + b.to){
            return true;
        }

        return false;
    }


};

// comparison functions for the ext. merge sort
bool comparator(const Edge &a, const Edge &b) { return a < b; }


int main(){

/*
	std::string graph_file = "/data/hugo-data/hugo-others/datasets/powerlawgraph";
	GenericEdgeSplitterManager<int32> emanager( 4294967296, true, 0);
    SplitterBuffer<int32, GenericEdgeSplitterManager<int32> , EmptyField> bsplitter (&emanager, graph_file, 0 ,mflash::DEFAULT_MEMORY_SIZE, "", false);
    EdgeConversor<int32, EmptyField>::process(graph_file, ' ', true, bsplitter );
*/


   string inFile       = "/data/hugo-data/hugo-others/datasets/.M-FLASH/twitter/0_0.block";//argv[1];
   string outFile      = "/data/hugo-data/hugo-others/datasets/.M-FLASH/twitter/0_0.block.output";//argv[1];
   int32  bufferSize     = 1073741824;      // allow the sorter to use 100Kb (base 10) of memory for sorting.
                                      // once full, it will dump to a temp file and grab another chunk.
   bool compressOutput = false;       // not yet supported
   string tempPath     = "/data/hugo-data/hugo-others/datasets/.M-FLASH/twitter/";        // allows you to write the intermediate files anywhere you want.

   // sort the lines of a file lexicographically in ascending order (akin to UNIX sort, "sort FILE")
   KwayMergeSort<Edge> *sorter = new KwayMergeSort<Edge> (inFile,
                                                              outFile,
                                                              comparator,
                                                              bufferSize,
                                                              compressOutput,
                                                              tempPath);
   sorter->Sort();
	return 0;
}
