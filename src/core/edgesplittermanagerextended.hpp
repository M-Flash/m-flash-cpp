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
#include "edgesplittermanager.hpp"

namespace mflash{

template <class IdType>
class EdgeSplitterManagerExtended{//: public EdgeSplitterManager< IdType > {

    public:
        IdType getPartitionId(IdType in_id, IdType out_id);
        IdType countEdge(IdType in_id, IdType out_id);
        std::string getPartitionFile(IdType id);
        std::vector<int64>& getPartitionCounters();
        IdType getPartitions();
        IdType getIdsByPartition();
        bool isInSplit();
        void setInSplit(bool);

        EdgeSplitterManagerExtended( int64 ids_by_partitions, bool in_split, int64 partition_id, std::vector<BlockType> block_types, int64 ids_cache = 0);
        ~EdgeSplitterManagerExtended(){}

    private:
        IdType getPartitionId(IdType in_id, IdType out_id, int32 shift);

        bool in_split;
        IdType ids_by_partition;
        IdType partitions;
        IdType partitionshift = 0;
        IdType cache_shift = 0;
        IdType shift = 0;
        std::vector<int64> partition_counters;
        bool *sparse_block;
        int64 partition_id;
        IdType ids_cache;
        bool cache_partitioning;


};


template <class IdType> EdgeSplitterManagerExtended<IdType>::EdgeSplitterManagerExtended(int64 ids_by_partition, bool in_split, int64 partition_id, std::vector<BlockType> block_types, int64 ids_cache){
    cache_partitioning = (ids_cache != 0);

    if(cache_partitioning && ids_by_partition % ids_cache != 0){
        LOG (ERROR)<< "ids by partititon must be multiple of ids in cache";
        assert(false);
    }
    if(!is2nNumber(ids_by_partition)){
      LOG (ERROR)<< "ids by partititon must be multiple of 2^n";
      assert(false);
    }


    cache_shift = (ids_cache != 0?log2(ids_cache) : 0);
    partitionshift = log2(ids_by_partition);
    shift = (cache_partitioning? cache_shift: partitionshift);

	this->ids_by_partition = ids_by_partition;
	this->ids_cache = ids_cache;
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
  if (!in_split)
    in_id = out_id;
  in_id>>=shift;
  if (sparse_block[in_id >>partitionshift])
      return this->partitions-1;
  return in_id;
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
IdType EdgeSplitterManagerExtended<IdType>::getIdsByPartition(){
    return ids_by_partition;
}

template <class IdType> inline
bool EdgeSplitterManagerExtended<IdType>::isInSplit(){
    return in_split;
}

template <class IdType> inline
void EdgeSplitterManagerExtended<IdType>:: setInSplit(bool in_split){
    this->in_split = in_split;
}



template <class IdType> inline
std::vector<int64>& EdgeSplitterManagerExtended<IdType>::getPartitionCounters(){
    return partition_counters;
}


template <class IdType> inline
std::string EdgeSplitterManagerExtended<IdType>::getPartitionFile(IdType id){
    if(cache_shift){
        id = (int)(id/(ids_by_partition/ids_cache));
    }
    if(id == this->partitions -1){
        return get_partition_file("", partition_id);
    }
    return get_block_file("", partition_id, id);
}

}
#endif
