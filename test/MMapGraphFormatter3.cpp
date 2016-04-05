
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
    std::ofstream output("/data/hugo-data/hugo-others/datasets/.M-FLASH/twitter_plain_sorted");
    MappedStream stream ("/data/hugo-data/hugo-others/datasets/.M-FLASH/twitter/0_0.block.output");
    int32 from ,to;
    while(stream.has_remain()){
        from = stream.next<int32>();
        to = stream.next<int32>();
        output <<from <<" "<<to<<std::endl;
    }
    output.close();

	return 0;
}
