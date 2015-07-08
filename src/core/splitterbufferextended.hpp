

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
	std::vector<int64> block_counters;
	int64 bpartitions;

	IdType countEdge(IdType in_id, IdType out_id);
public:
	void printCounters();

public:
	SplitterBufferExtended(std::string graph, int64 edge_data_size, int64 buffer_size, int64 ids_by_partitions, bool in_split = false, int64 partitions = 0 );
};

template <class IdType> inline
SplitterBufferExtended<IdType>::SplitterBufferExtended(std::string graph, int64 edge_data_size, int64 ids_by_partition, int64 buffer_size, bool in_split, int64 partitions)
: SplitterBuffer<IdType>(graph, edge_data_size, ids_by_partition, buffer_size, partitions, in_split){
	this->bpartitions = partitions;
	this->partition_counters.resize(partitions * partitions);

}

template <class IdType> inline
IdType SplitterBufferExtended<IdType>::countEdge(IdType in_id, IdType out_id){
	IdType partition_id = SplitterBuffer<IdType>::countEdge(in_id, out_id);
	IdType partition2_id = SplitterBuffer<IdType>::getPartitionId(out_id, in_id);

	int64 newPartitions = max(partition2_id, partition_id)+1;
	//checking block_counters
	if ( newPartitions > bpartitions){
		printCounters();
		block_counters.resize( newPartitions * newPartitions);

		//reallocating counters
		for(int i = bpartitions-1; i > 0; i--){
			memcpy(block_counters.data() +  i * newPartitions, block_counters.data() +  i * bpartitions, sizeof(int64) * bpartitions);
			memset(block_counters.data() +  i * bpartitions, 0, sizeof(int64) * (newPartitions-bpartitions));
		}
		bpartitions = newPartitions;
	}
	IdType id  = partition_id * bpartitions + partition2_id;
	block_counters[id] ++;
	return partition_id ;
}


template <class IdType>
void SplitterBufferExtended<IdType>::printCounters(){
	for (int i = 0 ; i<bpartitions; i++){
		for (int j = 0 ; j<bpartitions; j++){
			std::cout<<block_counters[i * bpartitions + j]<<"\t";
		}
		std::cout<<std::endl;
	}
}

}
#endif
