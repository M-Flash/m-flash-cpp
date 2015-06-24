

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "../../log/easylogging++.h"
#include "array.hpp"
#include "type.hpp"
#include "util.hpp"


#ifndef CORE_SPLITTERBUFFER_HPP_
#define CORE_SPLITTERBUFFER_HPP_


namespace mflash{

template <class IdType>
class SplitterBuffer{
	/**
	 * Buffer size in elements
	 */
	int64 buffer_size;

	/**
	 * Edge size in bytes without include the size of the in-id and out-id.
	 */
	int64 edge_data_size;

	bool is_edge_data;

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

	std::vector<int64> file_offsets;

	std::vector<int64> partition_counters;

	int64 partitions;

	std::string graph;

	GenericArray *splitter_buffer;
	GenericArray *main_buffer;

	int64 current_position;

	void split();

	IdType getPartitionId(IdType in_id, IdType out_id);

	public:

	SplitterBuffer(std::string graph, int64 edge_data_size, int64 buffer_size, int64 ids_by_partitions, int64 partitions = 0, bool in_split = false);

	void add(IdType in_id, IdType out_id, char* edge_data);


	void flush();

};

template <class IdType> inline
SplitterBuffer<IdType>::SplitterBuffer(std::string graph, int64 edge_data_size, int64 ids_by_partitions, int64 buffer_size, int64 partitions, bool in_split){
	this->graph = graph;
	this->edge_data_size = edge_data_size;
	this->edge_size =  2 * sizeof(IdType) + edge_data_size;
	this->is_edge_data = edge_data_size != 0;
	this->buffer_size = buffer_size;
	this->ids_by_partition = ids_by_partitions;
	this->partitions = partitions;
	this->in_split = in_split;
	this-> elements_buffer = buffer_size/ edge_size;
	this->id_size = sizeof(IdType);

	main_buffer = new GenericArray (edge_size, elements_buffer);
	splitter_buffer = new GenericArray (edge_size, elements_buffer);

	this->file_offsets.resize(partitions);// = new std::vector<int64>(partitions);
	this->partition_counters.resize(partitions);// = new std::vector<int64>(partitions);

	//memset(this->file_offsets, 0, sizeof(int64) * partitions);

	current_position = 0;

}
template <class IdType> inline
IdType SplitterBuffer<IdType>::getPartitionId(IdType in_id, IdType out_id){
	return (in_split? in_id: out_id)/ids_by_partition;
}

template <class IdType> inline
void SplitterBuffer<IdType>::add(IdType in_id, IdType out_id, char* edge_data){
	if(current_position >= elements_buffer ){
		split();
	}

	IdType partition_id = getPartitionId(in_id, out_id);
	char * element_ptr = splitter_buffer->get_element(current_position++);
	memcpy(element_ptr, &in_id, id_size);
	memcpy(element_ptr + id_size, &out_id, id_size);
	if(is_edge_data ){
		memcpy(element_ptr + (id_size<<2), edge_data, edge_data_size);
	}
	//checking partition by source
	if ( partition_id >= partition_counters.size()){
		partition_counters.resize(partition_id  + 1);
	}

	partition_counters[partition_id] ++;
}



template <class IdType>
void SplitterBuffer<IdType>::split(){

	//splitting value
	char *base_ptr = splitter_buffer->address();
	char *ptr = splitter_buffer->address();
	char *last_ptr = ptr + current_position * edge_size;

	char tmp_edge[edge_size];

	int partitions = partition_counters.size();
	char* split_positions[partitions];
	split_positions[0] = base_ptr;

	for(int i = 1 ; i < partitions; i++){
		split_positions[i] = split_positions[i-1] + edge_size * partition_counters[i-1];
	}

	IdType partition_id;

	char * partition_ptr;
	while(ptr < last_ptr){
		partition_id = getPartitionId(*((IdType*)ptr), *((IdType*)(ptr + id_size)));
		partition_ptr = split_positions[partition_id];
		if(ptr != partition_ptr){
			memcpy(tmp_edge, ptr, edge_size);
			memcpy(ptr, partition_ptr ,edge_size);
			memcpy(partition_ptr, tmp_edge ,edge_size);

		}else{
			ptr += edge_size;
		}
		split_positions[partition_id]+= edge_size;
	}

	file_offsets.resize(partitions);

	char* offset = base_ptr;
	//writing partitions to disk
	for(int i = 0 ; i < partitions; i++){
		if(partition_counters[i] == 0){
			continue;
		}
		int64 partition_size = sizeof(char) * partition_counters[i] * edge_size;

		FILE * pFile;
		std::string file = get_partition_file(graph, i);
		pFile = fopen (file.c_str(), file_offsets[i] == 0? "wb": "ab");
		LOG(DEBUG) << "Storing edges in partition "<< i << " : "<< file;
		fseek(pFile, file_offsets[i], SEEK_SET);
		fwrite (offset,sizeof(char), partition_size, pFile);
		fclose (pFile);

		offset += partition_size;

		file_offsets[i] += partition_size;
		//resetting counter for the next split
		partition_counters[i] = 0;
	}
	current_position = 0;

}


template <class IdType>
void SplitterBuffer<IdType>::flush(){
	split();
}



}
#endif
