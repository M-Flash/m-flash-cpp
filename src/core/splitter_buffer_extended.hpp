// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CORE_SPLITTERBUFFEREXTENDED_HPP_
#define CORE_SPLITTERBUFFEREXTENDED_HPP_

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "../util/easylogging++.h"
#include "array.hpp"
#include "splitterbuffer.hpp"
#include "type.hpp"
#include "util.hpp"


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
