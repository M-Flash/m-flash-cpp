#include "../src/mflash_basic_includes.hpp"
#include "../src/algorithm/pagerank.hpp"

using namespace std;
using namespace mflash;

int main(int argc, char ** argv){
	mflash_init(argc, argv);

    /* Parameters */
    std::string filename    = get_option_string("file"); // Base filename
    int niters              = get_option_int("niters", 4);

    std::string pg_ranks 	= get_parent_directory(filename) + "pg";


	Matrix<EmptyField, int> matrix (filename);
	PrimitiveVector<float, int> pvector(pg_ranks);
	PageRank::run(matrix, pvector, niters);
	return 0;
}
