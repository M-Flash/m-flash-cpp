// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CORE_GenericEdgeEdgeSplitterManagerExtended_HPP_
#define CORE_GenericEdgeEdgeSplitterManagerExtended_HPP_

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#include "../util/easylogging++.h"
#include "type.hpp"
#include "util.hpp"

namespace mflash{

template <class IdType>
class EdgeSplitterManagerExtended: public EdgeSplitterManager<IdType>{

    public:
        IdType getPartitionId(IdType in_id, IdType out_id);
        IdType countEdge(IdType in_id, IdType out_id);
        std::string getPartitionFile(IdType id);
        std::vector<int64>& getPartitionCounters();
        IdType getPartitions();
        IdType getIdsByPartition();
        bool isInSplit();

        EdgeSplitterManagerExtended( int64 ids_by_partitions, bool in_split, int64 partition_id, std::vector<BlockType> block_types);
        ~EdgeSplitterManagerExtended(){}

    private:
        bool in_split;
        IdType ids_by_partition;
        IdType partitions;
        IdType partitionshift = 0;
        std::vector<int64> partition_counters;

};


template <class IdType> inline
IdType EdgeSplitterManagerExtended<IdType>::EdgeSplitterManagerExtended(int64 ids_by_partition, bool in_split, int64 partition_id, std::vector<BlockType> block_types){


	if(!is2nNumber(ids_by_partition)){
	  LOG (ERROR)<< "ids by partititon must be multiple of 2^n";
	  assert(false);
	}
	partitionshift = log2(ids_by_partition);

	this->ids_by_partition = ids_by_partition;
	this->partitions = block_types.size()+1;
	this->in_split = in_split;
	this->partition_counters.resize(partitions);// = new std::vector<int64>(partitions);

	this->partition_id = partition_id;

    sparse_block = new bool[this->partitions];

    for(int32 i = 0; i < this->partitions -1; i++){
        sparse_block[i] = block_types[i] == BlockType::SPARSE;
    }

    sparse_block[this->partitions-1] = true;

}

template <class IdType> inline
IdType EdgeSplitterManagerExtended<IdType>::getPartitionId(IdType in_id, IdType out_id){
  if (sparse_block[in_id])
      return this->partitions-1;
  if (InSplit)
      return in_id>>partitionshift;
  return out_id>>partitionshift;
}


template <class IdType> inline
IdType EdgeSplitterManagerExtended<IdType>::countEdge(IdType in_id, IdType out_id){
	IdType partition_id = getPartitionId(in_id, out_id);
	//checking partition by source
	if ( partition_id >= partitions){
		//LOG(INFO) << "Increasing partitions to "<< partition_id+1;
		partitions = partition_id  + 1;
		partition_counters.resize(partition_id  + 1);
	}
	partition_counters[partition_id] ++;
	return partition_id ;
}

template <class IdType> inline
IdType EdgeSplitterManagerExtended<IdType>::getPartitions(){
    return partitions;
}

template <class IdType> inline
std::vector<int64>& EdgeSplitterManagerExtended<IdType>::getPartitionCounters(){
    return partition_counters;
}


template <class IdType> inline
std::string GenericEdgeSplitterManager<IdType>::getPartitionFile(IdType id){
    if(id == this->partitions -1)
        return get_partition_file(this->graph, partition_id);
    return get_block_file(this->graph, partition_id, id);
}

}
#endif
