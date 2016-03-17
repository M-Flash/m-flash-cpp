// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CORE_GenericEdgeEdgeSplitterManagerWithBlockCounting_HPP_
#define CORE_GenericEdgeEdgeSplitterManagerWithBlockCounting_HPP_

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
class EdgeSplitterManagerWithBlockCounting{//: public EdgeSplitterManager< IdType > {

    public:
        IdType getPartitionId(IdType in_id, IdType out_id);
        IdType countEdge(IdType in_id, IdType out_id);
        std::string getPartitionFile(IdType id);
        std::vector<int64>& getPartitionCounters();
        IdType getPartitions();
        IdType getIdsByPartition();
        bool isInSplit();

        //int64 getBlockPartitions();
        IdType getMaxId();
        std::vector<int64> getCounters();


        EdgeSplitterManagerWithBlockCounting( int64 ids_by_partitions, bool in_split = true,  int64 partitions = 0);
        ~EdgeSplitterManagerWithBlockCounting(){}

    private:
        bool in_split;
        IdType ids_by_partition;
        IdType partitions;
        IdType partitionshift = 0;
        std::vector<int64> partition_counters;
        std::vector<int64> block_counters;
        IdType bpartitions;
        IdType max_id;


};


template <class IdType>
EdgeSplitterManagerWithBlockCounting<IdType>::EdgeSplitterManagerWithBlockCounting(int64 ids_by_partition, bool in_split, int64 partitions){

	if(!is2nNumber(ids_by_partition)){
	  LOG (ERROR)<< "ids by partititon must be multiple of 2^n";
	  assert(false);
	}
	partitionshift = log2(ids_by_partition);

	this->ids_by_partition = ids_by_partition;
	this->partitions = partitions;
	this->in_split = in_split;
	this->partition_counters.resize(partitions);// = new std::vector<int64>(partitions);

	this->bpartitions = partitions;
	this->partition_counters.resize(partitions * partitions);
	this->max_id = 0;

}

template <class IdType> inline
IdType EdgeSplitterManagerWithBlockCounting<IdType>::getPartitionId(IdType in_id, IdType out_id){
  if (in_split)
    return in_id>>partitionshift;
  return out_id>>partitionshift;
}


template <class IdType> inline
IdType EdgeSplitterManagerWithBlockCounting<IdType>::countEdge(IdType in_id, IdType out_id){
	IdType partition_id = getPartitionId(in_id, out_id);
	//checking partition by source
	if ( partition_id >= partitions){
		//LOG(INFO) << "Increasing partitions to "<< partition_id+1;
		partitions = partition_id  + 1;
		partition_counters.resize(partition_id  + 1);
	}
	partition_counters[partition_id] ++;
	//return partition_id ;

	IdType partition2_id = getPartitionId(out_id, in_id);

	int64 newPartitions = max(partition2_id, partition_id) + 1;
	max_id = max(max_id, max(in_id, out_id));
    //checking block_counters
    if (newPartitions > bpartitions) {
        //printCounters();
        block_counters.resize(newPartitions * newPartitions);

        //reallocating counters
        for (int i = bpartitions - 1; i > 0; i--) {
            memcpy(block_counters.data() + i * newPartitions,
            block_counters.data() + i * bpartitions, sizeof(int64) * bpartitions);
            memset(block_counters.data() + i * bpartitions, 0,
            sizeof(int64) * (newPartitions - bpartitions));
        }
        bpartitions = newPartitions;
    }
    IdType id = partition_id * bpartitions + partition2_id;
    block_counters[id]++;
    return partition_id;
}

template <class IdType> inline
IdType EdgeSplitterManagerWithBlockCounting<IdType>::getPartitions(){
    return partitions;
    //return std::max(partitions,bpartitions);
}


template<class IdType>
std::vector<int64> EdgeSplitterManagerWithBlockCounting<IdType>::getCounters() {
  return block_counters;
}


template<class IdType>
IdType EdgeSplitterManagerWithBlockCounting<IdType>::getMaxId() {
  return max_id;
}

template <class IdType> inline
std::vector<int64>& EdgeSplitterManagerWithBlockCounting<IdType>::getPartitionCounters(){
    return partition_counters;
}

template <class IdType> inline
std::string EdgeSplitterManagerWithBlockCounting<IdType>::getPartitionFile(IdType id){
    return get_partition_file("", id, "");
}


}
#endif
