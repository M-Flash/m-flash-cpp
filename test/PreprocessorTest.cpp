

#include "../src/mflash_basic_includes.hpp"

using namespace mflash;
int main(int argc, char* argv[]){
	mflash_init(argc, argv);
	//std::cout<<mflash::get_config_option_string("filetype", "ed")<<std::endl;
	std::cout<<mflash::get_config_option_string("memorysize")<<std::endl;
	//TYNY GRAPH
/*
	std::string graph_file = "/hugo/datasets/tiny-graph/tiny-graph";
	int64 vertices_partition = 3;
	int64 buffer_size = 8 * 100;
*/


/*

	//YAHOO
	std::string graph_file = "/run/media/hugo/data/datasets/yahoo";
	int64 vertices_partition = 1073741824;
	int64 buffer_size = 1024L*1024*1024 * 8;
	bool edgelist = false;
	char separator = ' ';
*/


/*

	//Twitter
	std::string graph_file = "/run/media/hugo/data/datasets/twitter";
	int64 vertices_partition = 41652231;
	int64 buffer_size = 1024L*1024*1024 * 8;
	bool edgelist = true;
	char separator = ' ';

*/

	//LiveJournal
	std::string graph_file = "/data/datasets/test";
	std::string format = "adjlist2";


	mflash::convert<EmptyField, int>(graph_file, sizeof(float), format);
	return 0;
}
