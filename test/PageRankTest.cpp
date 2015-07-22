#include "../src/mflash_basic_includes.hpp"
#include "../src/algorithm/pagerank.hpp"

using namespace std;
using namespace mflash;

int main(int argc, char ** argv){
	mflash_init(argc, argv);
	Matrix<EmptyField, int> matrix ("/run/media/hugo/data/datasets/lj");
	PrimitiveVector<float, int> pvector("/run/media/hugo/data/datasets/lj");
	PageRank::run(matrix, pvector, 1);
	return 0;
}
