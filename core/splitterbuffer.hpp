

#include <stdio.h>
#include <cstring>
#include <string>
#include <vector>

#include "core/array.hpp"
#include "core/type.hpp"



namespace mflash{

template <class ID_TYPE>
class SplitterBuffer{
	/**
	 * Buffer size in elements
	 */
	int64 buffer_size;

	/**
	 * Edge size in bytes without include the size of the in-id and out-id.
	 */
	int64 edge_data_size;

	/**
	 * Edge size in bytes.
	 */
	int64 edge_size;

	/**
	 *  Number of element in the buffer.
	 */
	int64 elements_buffer;

	int64 id_size;


	/**
	 *  Number of ids for partition. The edges are split using the ids.
	 *  Example:
	 *  Partition 0: 0 ... ids_by_partition -1
	 *  Partition 1: ids_by_partition+1 ... 2*ids_by_partition -1
	 *  ...
	 *  Partition partitions-1:  ids_by_partition*(partitions-1) ... (partitions)*ids_by_partition -1
	 *
	 */
	int64 ids_by_partition;

	bool in_split;

	std::vector<int64> *file_offsets;

	std::vector<int64> *partition_counters;

	int64 partitions;

	std::string graph;

	GenericArray *splitter_buffer;
	GenericArray *main_buffer;

	int64 current_position;

	void split();

	ID_TYPE getPartitionId(ID_TYPE in_id, ID_TYPE out_id);

	public:

	SplitterBuffer(std::string graph, int64 edge_data_size, int64 buffer_size, int64 ids_by_partitions, int64 partitions = 0, bool in_split = false);

	void add(ID_TYPE in_id, ID_TYPE out_id, char* edge_data);



	void flush();

};

template <class ID_TYPE> inline
SplitterBuffer<ID_TYPE>::SplitterBuffer(std::string graph, int64 edge_data_size, int64 ids_by_partitions, int64 buffer_size, int64 partitions, bool in_split){
	this->graph = graph;
	this->edge_data_size = edge_data_size;
	this->edge_size = 2 * sizeof(ID_TYPE) + edge_data_size;
	this->buffer_size = buffer_size;
	this->ids_by_partition = ids_by_partitions;
	this->partitions = partitions;
	this->in_split = in_split;
	this-> elements_buffer = buffer_size/ (2 * edge_size);
	this->id_size = sizeof(ID_TYPE);

	main_buffer = new GenericArray (edge_size, elements_buffer);
	splitter_buffer = new GenericArray (edge_size, elements_buffer);

	this->file_offsets = new std::vector<int64>(partitions);
	this->partition_counters = new std::vector<int64>(partitions);

	//memset(this->file_offsets, 0, sizeof(int64) * partitions);

	current_position = 0;

}
template <class ID_TYPE> inline
ID_TYPE SplitterBuffer<ID_TYPE>::getPartitionId(ID_TYPE in_id, ID_TYPE out_id){
	return (in_split? in_id: out_id)/ids_by_partition;
}

template <class ID_TYPE> inline
void SplitterBuffer<ID_TYPE>::add(ID_TYPE in_id, ID_TYPE out_id, char* edge_data){
	if(current_position >= elements_buffer ){
		split();
	}

	ID_TYPE partition_id = getPartitionId(in_id, out_id);
	char * element_ptr = splitter_buffer->get_element(current_position++);
	memcpy(element_ptr, *in_id, id_size);
	memcpy(element_ptr + id_size, *out_id, id_size);
	memcpy(element_ptr + (id_size<<2), edge_data, edge_data_size);


	//checking partition by source
	if ( partition_id < partition_counters->size()){
		partition_counters->resize(partition_id  + 1);

	}
	partition_counters[partition_id] ++;
}



template <class ID_TYPE>
void SplitterBuffer<ID_TYPE>::split(){

	//splitting value
	char *ptr = splitter_buffer->address();
	char *last_ptr = ptr + current_position * current_position;

	char tmp_edge[edge_size];

	int partitions = partition_counters->size();
	int64 split_positions[partitions];
	split_positions[0] = 0;

	for(int i = 1 ; i < partitions; i++){
		split_positions[i] = split_positions[i-1] + partition_counters[i];
	}

	ID_TYPE partition_id;

	int64 current_partition = 0;
	while(ptr < last_ptr){
		partition_id = getPartitionId(*((ID_TYPE*)ptr), *((ID_TYPE*)(ptr + id_size)));
		if(partition_id != current_partition){
			memcpy(tmp_edge, ptr, edge_size);
			memcpy(ptr, split_positions[partition_id] ,edge_size);
			memcpy(split_positions[partition_id], tmp_edge ,edge_size);
		}else{
			ptr += edge_size;
		}
	}

	file_offsets->resize(partitions);

	int64 offset = 0;
	//writing partitions to disk
	for(int i = 0 ; i < partitions; i++){
		if(partition_counters[i] == 0){
			continue;
		}

		FILE * pFile;
		pFile = fopen ("myfile.bin", file_offsets[i] == 0? "wb": "ab");
		fwrite (main_buffer + (offset * edge_size), sizeof(char), sizeof(char) * current_position * edge_size, pFile);
		fclose (pFile);

		offset += partition_counters[i];
		file_offsets[i] += partition_counters[i];
		partition_counters[i] = 0;
	}
	current_position = 0;

}


}
