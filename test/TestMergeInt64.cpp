#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
using namespace std;

// local includes
#include "../src/mflash_basic_includes.hpp"
#include "../src/util/kwaymergesort.h"



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


int main(int argc, char* argv[]) {

    string inFile       = "/data/hugo-data/hugo-others/datasets/.M-FLASH/twitter/0.partition";//argv[1];
    string outFile      = "/data/hugo-data/hugo-others/datasets/.M-FLASH/twitter/0.partition.output";//argv[1];
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
}
