// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef MFLASH_CPP_CORE_TYPE_HPP_
#define MFLASH_CPP_CORE_TYPE_HPP_

typedef unsigned int int32;
typedef unsigned long long int64;

namespace mflash {

template<class V, class IdType>
class Element {
public:
	IdType id;
	V* value;

	void set_value(V value) {
		*(this->value) = value;
	}
};

class EmptyField {
};

enum ElementIdType {
	SIMPLE, DOUBLE,
};

enum Mode {
	UNSAFE, VECTOR_REPLICATION,
//ATOMIC
};

enum GraphFormatterMode {
	ADJACENCY_LIST, EDGE_LIST
};

enum FieldType {
	SOURCE, DESTINATION, BOTH
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
	int64 idSize;
	int64 partitions;
	int64 vertices_partition;
	int64 *edges_by_block;

	MatrixProperties(){
	  vertices = 0;
	  idSize = 0;
	  partitions = 0;
	  vertices_partition = 0;
	  edges_by_block = NULL;
	}

	MatrixProperties(int64 vertices, int64 idSize, int64 partitions, int64 vertices_partition, int64 edges_by_block[]){
		this->vertices = vertices;
		this->idSize = idSize;
		this->partitions = partitions;
		this->vertices_partition = vertices_partition;
		this->edges_by_block = new int64[partitions * partitions];
		memcpy(this->edges_by_block, edges_by_block, sizeof(int64) * partitions * partitions);
	}
	MatrixProperties(const MatrixProperties &p): MatrixProperties(p.vertices, p.idSize, p.partitions, p.vertices_partition, p.edges_by_block){
	}

	MatrixProperties& operator= (const MatrixProperties &p){
		vertices = p.vertices;
		idSize = p.idSize;
		partitions = p.partitions;
		vertices_partition = p.vertices_partition;
		if(edges_by_block != NULL){
		  delete [] edges_by_block;
		}
		this->edges_by_block = new int64[partitions * partitions];
		memcpy(this->edges_by_block, p.edges_by_block, sizeof(int64) * partitions * partitions);
		return *this;
	}

	~MatrixProperties(){
	  if(edges_by_block != NULL){
	    delete [] edges_by_block;
	    edges_by_block = NULL;
	  }

	}

	int64 getEdgesBlock(int32 row, int32 col){
		if(row >= 0 && row <partitions && col >= 0 && col <partitions){
			return edges_by_block[row * partitions + col];
		}
		return 0;
	}
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
