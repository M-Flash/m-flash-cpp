#include <ctime>
#include "../src/mflash_basic_includes.hpp"
#include "../src/algorithm/pagerank.hpp"

using namespace std;
using namespace mflash;

int main(int argc, char ** argv){
	mflash_init(argc, argv);
	time_t timer1;
	  time_t timer2;
	  time_t final;
	  time(&timer1);



    /* Parameters */
    std::string filename    = get_option_string("file"); // Base filename
    int niters              = get_option_int("niters", 4);

    std::string pg_ranks 	= get_parent_directory(filename) + "pg";


	Matrix<EmptyField, int> matrix (filename);
	PrimitiveVector<float, int> pvector(pg_ranks);
	PageRank::run(matrix, pvector, niters);

	time(&timer2);
  final = timer2 - timer1;
  cout<<"Time: "<< final << " seconds"<<endl;
  exit(0);
	return 0;
}
