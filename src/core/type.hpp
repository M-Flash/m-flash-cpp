/*
 * type.hpp
 *
 *  Created on: Feb 27, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_TYPE_HPP_
#define MFLASH_CPP_CORE_TYPE_HPP_

typedef long long int64;

namespace mflash {

template<class V>
class Element {
public:
	int64 id;
	V* value;

	void set_value(V value) {
		*(this->value) = value;
	}
};

class EmptyField {
};

enum ElementIdSize {
	SIMPLE, DOUBLE,
//ATOMIC
};

enum Mode {
	UNSAFE, VECTOR_REPLICATION,
//ATOMIC
};

enum GraphFormatterMode {
	ADJACENCY_LIST, EDGE_LIST
};

enum FieldType {
	SOURCE, DESTINATION
};

enum BlockType {
	DENSE, SPARSE
};

enum MatrixProperty{
	VERTICES,
	PARTITIONS, // Beta
	VERTICES_PARTITION,
	EDGES_BLOCKS
};

struct MatrixProperties{
	int64 vertices;
	int64 partitions;
	int64 vertices_partition;
	int64 *edges_by_block;

	MatrixProperties(int64 vertices, int64 partitions, int64 vertices_partition, int64 edges_by_block[]){
		this->vertices = vertices;
		this->partitions = partitions;
		this->vertices_partition = vertices_partition;
		this->edges_by_block = edges_by_block;
	}
	MatrixProperties(){}
};

struct BlockProperties {
	BlockType type;
	int64 offset;
	int64 size;

	BlockProperties(BlockType type, int64 offset, int64 size) {
		this->type = type;
		this->offset = offset;
		this->size = size;
	}
};
}
#endif /* MFLASH_CPP_CORE_TYPE_HPP_ */
