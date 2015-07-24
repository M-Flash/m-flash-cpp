#include "../src/mflash_basic_includes.hpp"
#include "../src/algorithm/lanczosSO.hpp"

using namespace std;
using namespace mflash;

int main(int argc, char ** argv){
	mflash_init(argc, argv);
	//Matrix<EmptyField, int> matrix2 ("/run/media/hugo/data/datasets/twitter");
	PrimitiveMatrix<double, EmptyField, int> matrix ("/run/media/hugo/data/datasets/twitter");
	PrimitiveVector<double, int> pvector("/run/media/hugo/data/datasets/lancsos");
	//matrix.multiply(pvector, pvector);
	//pvector.multiply(3.3);
	LanczosSO<double, EmptyField, int> lanczos (matrix, 20, 6);
	lanczos.run();
	return 0;
}
