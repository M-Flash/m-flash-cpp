// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CORE_SPLITTERBUFFERBLOCKCOUNTING_HPP_
#define CORE_SPLITTERBUFFERBLOCKCOUNTING_HPP_

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "../util/easylogging++.h"
#include "array.hpp"
#include "splitterbuffer.hpp"
#include "type.hpp"
#include "util.hpp"

namespace mflash {

template<class IdType>
class SplitterBufferWithBlockCounting: public SplitterBuffer<IdType> {

private:
  std::vector<int64> block_counters;
  int64 bpartitions;
  IdType max_id;
  IdType countEdge(IdType in_id, IdType out_id);
public:
  int64 getPartitions();
  IdType getMaxId();

public:
  SplitterBufferWithBlockCounting(std::string graph, int64 edge_data_size,
      int64 buffer_size, int64 ids_by_partitions, bool in_split = true,
      std::string file_prefix = "", int64 partitions = 0);
  std::vector<int64> getCounters();
};

template<class IdType> inline SplitterBufferWithBlockCounting<IdType>::SplitterBufferWithBlockCounting(
    std::string graph, int64 edge_data_size, int64 buffer_size,
    int64 ids_by_partition, bool in_split, std::string file_prefix,
    int64 partitions) :
    SplitterBuffer<IdType>(graph, edge_data_size, buffer_size, ids_by_partition,
	in_split, file_prefix, partitions) {
  this->bpartitions = partitions;
  this->partition_counters.resize(partitions * partitions);
  max_id = 0;

}

template<class IdType> inline IdType SplitterBufferWithBlockCounting<IdType>::countEdge(
    IdType in_id, IdType out_id) {
  IdType partition_id = SplitterBuffer<IdType>::countEdge(in_id, out_id);
  IdType partition2_id = SplitterBuffer<IdType>::getPartitionId(out_id, in_id);

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

/*
 template <class IdType>
 void SplitterBufferWithBlockCounting<IdType>::printCounters(){
 for (int i = 0 ; i<bpartitions; i++){
 for (int j = 0 ; j<bpartitions; j++){
 std::cout<<block_counters[i * bpartitions + j]<<"\t";
 }
 std::cout<<std::endl;
 }
 }*/

template<class IdType>
std::vector<int64> SplitterBufferWithBlockCounting<IdType>::getCounters() {
  return block_counters;
}

template<class IdType>
int64 SplitterBufferWithBlockCounting<IdType>::getPartitions() {
  return bpartitions;
}

template<class IdType>
IdType SplitterBufferWithBlockCounting<IdType>::getMaxId() {
  return max_id;
}

}
#endif
