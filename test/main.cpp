#include "../src/core/graphbinarization.hpp"
#include "../src/core/mapped_stream.hpp"
#include "../log/easylogging++.h"
#include "../src/core/splitterbuffer.hpp"


INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){

	//:SplitterBuffer(std::string graph, int64 edge_data_size, int64 ids_by_partitions, int64 buffer_size, int64 partitions, bool in_split){


	std::string graph_file = "/hugo/datasets/yahoo.txt";
	graph_file = "/hugo/datasets/yahoo.txt";
	SplitterBuffer<int> splitter(graph_file, 0, 268435456, 4294967296);
	convert_adjlist<EmptyField, int>(graph_file, splitter );





	/**
	 * SPEED TEST EXAMPLE

		LOG (INFO) << sizeof(char);
		MappedStream stream("/hugo/datasets/yahoo.txt");

		long sum = 0;

		while(stream.has_remain()){
			sum += stream.next_int();
		}
		LOG (INFO) << sum;
		stream.close_stream();
	 */



	return 0;
}
