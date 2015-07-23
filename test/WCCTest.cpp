#include "../src/mflash_basic_includes.hpp"
#include "../src/algorithm/wcc.hpp"

using namespace std;
using namespace mflash;

int main(int argc, char ** argv){
	mflash_init(argc, argv);
	Matrix<EmptyField, int> matrix ("/run/media/hugo/data/datasets/twitter");
	PrimitiveVector<int, int> pvector("/run/media/hugo/data/datasets/wcc");
	WCC::run(matrix, pvector);
	return 0;
}
