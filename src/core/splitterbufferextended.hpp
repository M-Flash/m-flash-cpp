

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

	IdType countEdge(IdType in_id, IdType out_id);

public:
	SplitterBufferExtended(std::string graph, int64 edge_data_size, int64 buffer_size, int64 ids_by_partitions, int64 partitions = 0, bool in_split = false);
};

template <class IdType> inline
SplitterBufferExtended<IdType>::SplitterBufferExtended(std::string graph, int64 edge_data_size, int64 ids_by_partitions, int64 buffer_size, int64 partitions, bool in_split)
: SplitterBufferExtended<IdType>(graph, edge_data_size, ids_by_partition, buffer_size, partitions, in_split){
	this->partition_counters.resize(partitions * partitions);

}

template <class IdType> inline
IdType SplitterBufferExtended<IdType>::countEdge(IdType in_id, IdType out_id){
	int64 partitions = this->partitions;
	IdType partition_id = SplitterBuffer<IdType>::countEdge(id_id, out_id);
	IdType partition2_id = SplitterBuffer<IdType>::getPartitionId(out_id, id_id);
	//checking block_counters
	if ( this->partitions != current_size){
		block_counters.resize( (this->partitions )* (this->partitions ));
		//reallocating counters
		for(int i = partition_id-1; i > 0; i--){
			memcpy(block_counters.data() + sizeof(int64) * this->partitions, block_counters.data() + sizeof(int64) * partitions, sizeof(int64) * partitions);
		}
	}
	partition_counters[partition_id] ++;
	return partition_id ;
}


}
#endif
