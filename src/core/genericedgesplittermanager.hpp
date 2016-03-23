// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CORE_GenericEdgeGenericEdgeSplitterManager_HPP_
#define CORE_GenericEdgeGenericEdgeSplitterManager_HPP_

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
class GenericEdgeSplitterManager{//: public EdgeSplitterManager<IdType>{

    public:
        IdType getPartitionId(IdType in_id, IdType out_id);
        IdType countEdge(IdType in_id, IdType out_id);
        std::string getPartitionFile(IdType id);
        std::vector<int64>& getPartitionCounters();
        IdType getPartitions();
        IdType getIdsByPartition();
        bool isInSplit();
        void setInSplit(bool in_split);

        GenericEdgeSplitterManager( int64 ids_by_partitions, bool in_split = true,  int64 partitions = 0);
        ~GenericEdgeSplitterManager(){}

    private:
        bool in_split;
        IdType ids_by_partition;
        IdType partitions;
        IdType partitionshift = 0;
        std::vector<int64> partition_counters;


};


template <class IdType>
GenericEdgeSplitterManager<IdType>::GenericEdgeSplitterManager(int64 ids_by_partition, bool in_split, int64 partitions){

	if(!is2nNumber(ids_by_partition)){
	  LOG (ERROR)<< "ids by partititon must be multiple of 2^n";
	  assert(false);
	}
	partitionshift = log2(ids_by_partition);

	this->ids_by_partition = ids_by_partition;
	this->partitions = partitions;
	this->in_split = in_split;
	this->partition_counters.resize(partitions);// = new std::vector<int64>(partitions);

}

template <class IdType> inline
IdType GenericEdgeSplitterManager<IdType>::getPartitionId(IdType in_id, IdType out_id){
  in_id = (in_split? in_id: out_id);
#if IdType == int32
  if(partitionshift >=32)
      return 0;
#endif
  return in_id >>=partitionshift;
}


template <class IdType> inline
IdType GenericEdgeSplitterManager<IdType>::countEdge(IdType in_id, IdType out_id){
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
IdType GenericEdgeSplitterManager<IdType>::getIdsByPartition(){
    return ids_by_partition;
}

template <class IdType> inline
IdType GenericEdgeSplitterManager<IdType>::getPartitions(){
    return partitions;
}

template <class IdType> inline
bool GenericEdgeSplitterManager<IdType>::isInSplit(){
    return in_split;
}
template <class IdType> inline
void GenericEdgeSplitterManager<IdType>:: setInSplit(bool in_split){
    this->in_split = in_split;
}



template <class IdType> inline
std::vector<int64>& GenericEdgeSplitterManager<IdType>::getPartitionCounters(){
    return partition_counters;
}


template <class IdType> inline
std::string GenericEdgeSplitterManager<IdType>::getPartitionFile(IdType id){
    return get_partition_file("", id, "");
}

}
#endif
