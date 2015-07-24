

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "../../log/easylogging++.h"
#include "array.hpp"
#include "splitterbuffer.hpp"
#include "type.hpp"
#include "util.hpp"


#ifndef CORE_SPLITTERBUFFEREXTENDED_HPP_
#define CORE_SPLITTERBUFFEREXTENDED_HPP_


namespace mflash{

template <class IdType>
class SplitterBufferExtended: public SplitterBuffer<IdType>{


private:
	IdType getPartitionId(IdType in_id, IdType out_id);
	std::string getPartitionFile(IdType id);

	bool *sparse_block;
	int64 partition_id;

public:
	SplitterBufferExtended(std::string graph, int64 edge_data_size, int64 buffer_size, int64 ids_by_partition, bool in_split, int64 partition_id, std::vector<BlockType> block_types);

	~SplitterBufferExtended();
};

template <class IdType> inline
SplitterBufferExtended<IdType>::SplitterBufferExtended(std::string graph, int64 edge_data_size, int64 buffer_size, int64 ids_by_partition, bool in_split, int64 partition_id, std::vector<BlockType> block_types)
	: SplitterBuffer<IdType>(graph, edge_data_size, buffer_size, ids_by_partition, in_split, "", block_types.size()+1){
	this->partition_id = partition_id;

	sparse_block = new bool[this->partitions];

	for(int i = 0; i < this->partitions -1; i++){
		sparse_block[i] = block_types[i] == BlockType::SPARSE;
	}

	sparse_block[this->partitions-1] = true;
}
template <class IdType> inline
SplitterBufferExtended<IdType>::~SplitterBufferExtended(){
	delete this->sparse_block;
}

template <class IdType> inline
IdType SplitterBufferExtended<IdType>::getPartitionId(IdType in_id, IdType out_id){
	in_id = (this->in_split? in_id: out_id)/this->ids_by_partition;
	return sparse_block[in_id]? this->partitions-1: in_id;
}


template <class IdType> inline
std::string SplitterBufferExtended<IdType>::getPartitionFile(IdType id){
	if(id == this->partitions -1)
		return get_partition_file(this->graph, partition_id);
	return get_block_file(this->graph, partition_id, id);
}


}
#endif
