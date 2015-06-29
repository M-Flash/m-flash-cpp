#include "../src/core/splitterbuffer.hpp"
#include "../src/core/graphbinarization.hpp"
#include "../src/core/mapped_stream.hpp"
#include "../log/easylogging++.h"


INITIALIZE_EASYLOGGINGPP

using namespace mflash;
int main(){

	std::string graph_file = "/hugo/datasets/.M-FLASH/0.partition";
	SplitterBuffer<int> splitter(graph_file, 0, 134217728, 8589934592, 0, true);

	MappedStream stream (graph_file);

	int from, to;
	EmptyField field;
	stream.set_position(0 );
	while(stream.has_remain()){
		from = stream.next_int();
		to = stream.next_int();
		splitter.add(from, to, &field);
	}

	splitter.flush();
}
