// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef MFLASH_CPP_CORE_MATRIXWORKER_HPP_
#define MFLASH_CPP_CORE_MATRIXWORKER_HPP_

#include <algorithm>
#include <map>
#include <string>
#include <utility>

#include "../util/cmdopts.hpp"
#include "../util/easylogging++.h"
#include "array.hpp"
#include "blockiterator.hpp"
#include "edgelistthread.hpp"
#include "malgorithm.hpp"
#include "mapped_stream.hpp"
#include "matrix.hpp"
#include "operator.hpp"
#include "splitterbuffer.hpp"
#include "type.hpp"
#include "util.hpp"
#include "vector.hpp"

using namespace std;

namespace mflash {

struct VectorPointer {
	AbstractVector *vector;
	void ** pointer;

	VectorPointer(AbstractVector *vector) {
		this->vector = vector;
		pointer = new void*;
	}
};

template<class VSource, class VDestination, class E, class IdType, class MALGORITHM>
class InitializeOperator;

template<class VSource, class VDestination, class E, class IdType, class MALGORITHM>
class AppyOperator;

template <class E, class IdType, class VSource, class VDestination>
class EdgeListThread;

template<class E, class IdType>
class MatrixWorker {
public:
	Matrix<E, IdType> * matrix;
	MatrixProperties matrixProperties;

	map<string, VectorPointer*> source_map;
	map<string, VectorPointer*> destination_map;

	AbstractVector *default_source;
	AbstractVector *default_destination;

	int64 field_count;
	void** value_pointers;

	GenericArray *source_pointer;
	GenericArray *destination_pointer;
	GenericArray **array_pointers;
	AbstractVector **vector_pointers;

	template<class VSource, class VDestination>
	void initialize_fields();

	void clean_fields();

	template<class VSource, class VDestination>
	void load_fields(FieldType type, int64 offset);
	void store_fields(int64 offset);

	int64 get_field_count() {
		return source_map.size() + destination_map.size();
	}

	template<class VSource, class VDestination>
	void preprocessing();

	MatrixWorker(Matrix<E, IdType> &matrix);

	Matrix<E, IdType> get_matrix() {
		return *matrix;
	}

	//bool add_field(FieldType type, string fieldname, AbstractVector &vector);

	void set_default_source_field(AbstractVector *vector);
	void set_default_destination_field(AbstractVector *vector);

	bool add_field(FieldType type, string fieldname, AbstractVector &vector);

	bool remove_field(FieldType type, string fieldname);
	void set_field_pointer(FieldType type, string fieldname, void **pointer);

	int64 element_size();

	int64 source_size();

	int64 destination_size();

	int64 elements_partition();

	template<class VSource, class VDestination, class MALGORITHM>
	void operate(MALGORITHM &algorithm);

	GenericArray* get_source_array(){
		return source_pointer;
	}

	GenericArray* get_destination_array(){
		return destination_pointer;
	}

};

template<class E, class IdType>
inline MatrixWorker<E, IdType>::MatrixWorker(Matrix<E,IdType> &matrix) {
	this->matrix = &matrix;
	this->default_source = 0;
	this->default_destination = 0;
	this->vector_pointers = 0;
	this->destination_pointer = 0;
	this->source_pointer = 0;
	this->value_pointers = 0;
	this->field_count = 0;
	this->array_pointers = 0;

}

/*template <class E>
 inline bool MatrixWorker<E, IdType>::add_field(FieldType type, string fieldname, AbstractVector &vector){
 return add_field(type, fieldname, vector, false);
 }*/

template<class E, class IdType>
inline void MatrixWorker<E, IdType>::set_default_source_field(AbstractVector *vector) {
	this->default_source = vector;
}

template<class E, class IdType>
inline void MatrixWorker<E, IdType>::set_default_destination_field(
		AbstractVector *vector) {
	this->default_destination = vector;
}

template<class E, class IdType>
inline bool MatrixWorker<E, IdType>::add_field(FieldType type, string fieldname,
		AbstractVector &vector) {

	/*  if(default_field){
	 if(FieldType::SOURCE == type){
	 default_source = vector;
	 }else{
	 default_destination = vector;
	 }
	 return;
	 }
	 */
	if (FieldType::SOURCE == type) {
		source_map[fieldname] = new VectorPointer(&vector);
	} else {
		destination_map[fieldname] = new VectorPointer(&vector);
	}

	return true;
}

template<class E, class IdType>
inline bool MatrixWorker<E, IdType>::remove_field(FieldType type, string fieldname) {

	if (FieldType::SOURCE == type) {
		source_map.erase(fieldname);
	} else {
		destination_map.erase(fieldname);
	}
	return true;
}

template<class E, class IdType>
inline void MatrixWorker<E, IdType>::set_field_pointer(FieldType type, string fieldname,
		void **pointer) {
	VectorPointer *vpointer;

	if (FieldType::SOURCE == type) {
		vpointer = source_map[fieldname];
	} else {
		vpointer = destination_map[fieldname];
	}
	*(vpointer->pointer) = pointer;
}

template<class E, class IdType>
inline int64 MatrixWorker<E, IdType>::element_size() {
	return source_size() + destination_size();
}


template<class E, class IdType>
inline int64 MatrixWorker<E, IdType>::source_size() {
	int64 size = 0;
	//check the size using the available memory and the fields registered.
	map<string, VectorPointer*>::iterator iter;
	VectorPointer *vpointer;
	for (iter = source_map.begin(); iter != source_map.end(); iter++) {
		vpointer = iter->second;
		size += vpointer->vector->element_size();
	}
	if (default_source != 0) {
		size += default_source->element_size();
	}

	return size;
}


template<class E, class IdType>
inline int64 MatrixWorker<E, IdType>::destination_size() {
	int64 size = 0;
	//check the size using the available memory and the fields registered.
	map<string, VectorPointer*>::iterator iter;
	VectorPointer *vpointer;
	for (iter = destination_map.begin(); iter != destination_map.end(); iter++) {
		vpointer = iter->second;
		size += vpointer->vector->element_size();
	}
	if (default_destination!= 0) {
		size += default_destination->element_size();
	}

	return size;
}



template<class E, class IdType>
inline int64 MatrixWorker<E, IdType>::elements_partition() {
	int64 size = element_size();

	if (matrix->get_elements_by_block() != 0){
		return matrix->get_elements_by_block();
	}

	if (size != 0) {
		return getVeticesByPartition(size);
	}

	return 0;
}

template<class E, class IdType>
template<class VSource, class VDestination>
void MatrixWorker<E, IdType>::initialize_fields() {
	field_count = source_map.size() + destination_map.size();
	value_pointers = new void*[field_count];
	array_pointers = new GenericArray*[field_count];//(GenericArray*) malloc(sizeof(GenericArray) * field_count);//  new GenericArray[field_count];
	vector_pointers = new AbstractVector*[field_count];

	VectorPointer *vpointer;
	map<string, VectorPointer*>::iterator iter;

	int64 vertices_by_partition = matrix->get_elements_by_block();
	int pos = 0;

	if (default_source != 0) {
		source_pointer = new Array<VSource, IdType>(vertices_by_partition);
	}

	if (default_destination != 0) {
		destination_pointer = new Array<VDestination, IdType>(vertices_by_partition);
	}

	for (iter = source_map.begin(); iter != source_map.end(); iter++, pos++) {
		vpointer = iter->second;
		value_pointers[pos] = *(vpointer->pointer);
		array_pointers[pos] = new GenericArray(vpointer->vector->element_size(),
				vertices_by_partition);
		vector_pointers[pos] = vpointer->vector;
	}

	for (iter = destination_map.begin(); iter != destination_map.end();
			iter++, pos++) {
		vpointer = iter->second;
		value_pointers[pos] = *(vpointer->pointer);
		array_pointers[pos] = new GenericArray(vpointer->vector->element_size(),
				vertices_by_partition);
		vector_pointers[pos] = vpointer->vector;
	}

}

template<class E, class IdType>
void MatrixWorker<E, IdType>::clean_fields() {
	if (default_source != 0) {
		delete source_pointer;
	}

	if (default_destination != 0) {
		delete destination_pointer;
	}

	for (int32 i = 0; i < field_count; i++) {
		array_pointers[i]->free_memory();
	}

	delete[] value_pointers;
	delete[] vector_pointers;
	delete[] array_pointers;
	//free (array_pointers);

	field_count = 0;
	value_pointers = 0;
	array_pointers = 0;
	vector_pointers = 0;

}

template<class E, class IdType>
template<class VSource, class VDestination>
inline void MatrixWorker<E, IdType>::load_fields(FieldType type, int64 offset) {

	int64 elements_partition = this->elements_partition();
	int pos = 0;
	int size = 0;

	if (FieldType::SOURCE == type) {
		pos = 0;
		size = source_map.size();

		if (default_source != 0) {
			Array<VSource, IdType> *source_wrapped =dynamic_cast<Array<VSource, IdType>*>(source_pointer);
			source_wrapped->set_limit(default_source->load_region(offset, elements_partition,source_pointer->address()));
			source_wrapped->set_offset(offset);
		}
	} else {
		pos = source_map.size();
		size = destination_map.size();

		if (default_destination != 0) {
			Array<VDestination, IdType> *destination_wrapped = dynamic_cast<Array<VDestination, IdType>*>(destination_pointer);
			destination_wrapped->set_limit(default_destination->load_region(offset, elements_partition,destination_pointer->address()));
			destination_wrapped->set_offset(offset);
		}
	}

	for (int i = pos; i < size; i++) {
		array_pointers[i]->set_limit(vector_pointers[i]->load_region(offset, elements_partition,array_pointers[i]->address()));
		array_pointers[i]->set_offset(offset);
	}
}

template<class E, class IdType>
inline void MatrixWorker<E, IdType>::store_fields(int64 offset) {
	if (default_destination != 0) {
		default_destination->store_region(offset, this->elements_partition(),
				destination_pointer->address());
	}
}

template<class E, class IdType>
template<class VSource, class VDestination, class MALGORITHM>
void MatrixWorker<E, IdType>::operate(MALGORITHM &algorithm) {
	const int64 elements = matrix->size();
	const int64 vertices_by_partition = matrix->get_elements_by_block();
//	vertices_by_partition = vertices_by_partition == 0 ? matrix->get_elements_by_block() : vertices_by_partition;

	matrixProperties = matrix->get_matrix_properties();

	//block iteration
	BlockIterator<E, IdType> iterator(matrix, matrix->is_transpose() ? 1 : 0);

	int row = -1;
	int last_col = -1;
	int64 source_offset = 0;
	int64 destination_offset = 0;
	int64 source_limit = 0;
	int64 destination_limit = 0;

	LOG (INFO)<< "- MATRIX OPERATION STARTED";
	LOG (INFO)<< "- EDGE PREPROCESSING STARTED";
	preprocessing<VSource, VDestination>();
	LOG (INFO)<< "- EDGE PREPROCESSING FINISHED";

	initialize_fields<VSource, VDestination>();

	Array<VDestination, IdType> *destination_wrapped = dynamic_cast<Array<VDestination, IdType>*>(destination_pointer);

	InitializeOperator<VSource, VDestination, E, IdType, MALGORITHM> initialize_operator(&algorithm, this);
	AppyOperator<VSource, VDestination, E, IdType, MALGORITHM> apply_operator(&algorithm, this);


	algorithm.before_iteration(0, *this);

	//int block_id = -1;
	while (iterator.has_next()) {
		Block block = iterator.next();
		//block_id = block.get_row() * matrixProperties.partitions + block.get_col();

		source_offset = block.get_col() * vertices_by_partition;
		source_limit = source_offset+ min(source_offset + vertices_by_partition, elements - source_offset)- 1;

		if (block.get_row() != row) {
			if (row != -1) {
				//making summarization
				//sumReplicates(algorithm, worker, outAccumulator, replicates);
				LOG (INFO)<< "--- STORING OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit;
				store_fields(destination_offset);
				LOG (INFO) << "--- OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " STORED";

			}
			row = block.get_row();

			destination_offset = block.get_row()*vertices_by_partition;
			destination_limit = destination_offset+min(destination_offset + vertices_by_partition, elements-destination_offset) -1;


			if(algorithm.is_destination_loaded() && default_destination != 0) {
				LOG (INFO)<< "--- LOADING OUT-ELEMENT STATES";
				load_fields<VSource, VDestination>(FieldType::DESTINATION, destination_offset);
				LOG (INFO) << "--- OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " LOADED";
			}else{
				LOG (INFO)<< "--- LOADING OUT-ELEMENT STATES OMITTED";
			}
			//initializing out_vector values
			if(algorithm.is_initialized() && default_destination != 0) {
				LOG (INFO) << "--- INITIALIZING ON OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit;
				destination_wrapped->operate(initialize_operator, *destination_wrapped, *destination_wrapped, *destination_wrapped);
				LOG (INFO) << "--- OUT-ELEMENT STATES INITIALIZED";
			} else {
				LOG (INFO) << "--- INITIALIZING ON OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " OMITTED";
			}

			//PROCESSING

			//copying values
			/*if(mode.equals(Mode.VECTOR_REPLICATION)){
			 //  logger.info("--- REPLICATING OUT-ELEMENT STATES BEETWEEN {} AND {} ", outAccumulator->getOffset(), outAccumulator->getOffset() + outAccumulator->getLimit());
			 for(int i = 0; i<Util.MATRIX_THREADS-1; i++){
			 replicates[i].setLimit(outAccumulator->getLimit());
			 replicates[i].setOffsetBytes(outAccumulator->getOffsetBytes());
			 outAccumulator->copy(replicates[i]);
			 }
			 }*/
			Block sparseblock(get_partition_file(matrix->get_file(), block.get_row() , "updates"), block.get_row(), -1, BlockType::SPARSE);

			if(sparseblock.exist()){
				LOG (INFO)<< "- PROCESSING SPARSE PARTITION "<< block.get_row();
				EdgeListThread<E, IdType, VSource, VDestination> sparse_thread(*this, 0, sparseblock);
				sparse_thread.call(algorithm);
				LOG (INFO)<< "- END PROCESSING SPARSE PARTITION "<< block.get_row();
			}else{
				LOG (INFO)<< "- PROCESSING HAS BEEN OMITTED FOR SPARSE PARTITION "<< block.get_row() << " BECAUSE IT DOES NOT CONTAIN SPARSE BLOCKS";
			}
		}

		LOG (INFO)<< "- PROCESSING BLOCK "<< block.get_col() << " -> " << block.get_row();
		if (!block.exist()) {
			LOG (INFO)<< "- BLOCK "<< block.get_col() << " -> " << block.get_row() << " WITHOUT EDGES ";
			LOG (INFO)<< "- " << block.get_file();
			continue;
		}


		if(block.isSparse()){
			LOG (INFO)<< "- BLOCK "<< block.get_col() << " -> " << block.get_row() << " IS SPARSE SO IT WAS PROCESSED BEFORE";
			continue;
		}

		if (last_col != block.get_col()) {
			last_col = block.get_col();
			if(algorithm.is_source_loaded() && default_source != 0) {
				LOG (INFO)<< "--- LOADING IN-ELEMENT STATES";
				load_fields<VSource, VDestination>(FieldType::SOURCE,source_offset);
				LOG (INFO)<< "--- IN-ELEMENT STATES BEETWEEN " << source_offset << " AND " << source_limit<< " LOADED";
			}else{
				LOG (INFO)<< "--- LOADING IN-ELEMENT STATES OMIITED";
			}
		}
		LOG (INFO)<< "--- PROCESSING EDGES FROM DENSE BLOCK: " << "----" << block.get_file();
		EdgeListThread<E, IdType, VSource, VDestination> thread(*this, 0, block);
		thread.call(algorithm);
		LOG (INFO)<< "--- PROCESSING EDGES FINALIZED";
	}

	if (algorithm.is_applied() && default_destination != 0) {
		LOG (INFO)<< "--- APPLYING ON OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " STORED";
		destination_wrapped->operate(apply_operator, *destination_wrapped, *destination_wrapped, *destination_wrapped);
		LOG (INFO) << "--- OUT-ELEMENT STATES APPLYED";
	} else {
		LOG (INFO) << "--- APPLYIN ON OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " OMITTED";
	}

	if(algorithm.is_destination_stored() && default_destination != 0) {
		LOG (INFO)<< "--- STORING OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit;
		store_fields(destination_offset);
		LOG (INFO)<< "--- OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " STORED";
	}else{
		LOG (INFO)<< "--- STORING OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " OMITTED";
	}
	//making summarization
	//sumReplicates(algorithm, worker, outAccumulator, replicates);
	algorithm.after_iteration(0, *this);
	LOG (INFO)<< "- MATRIX OPERATION FINISHED";
	clean_fields();

}

template<class E, class IdType>
template<class VSource, class VDestination>
void MatrixWorker<E, IdType>::preprocessing() {

	int64 vertex_size = max(sizeof(VSource), sizeof(VDestination));

	//block iteration
	BlockIterator<E,IdType> iterator(matrix, matrix->is_transpose() ? 1 : 0);

	int64 elements_partitions = matrixProperties.vertices_partition;
	int row = -1;
	bool in_loaded = false;

	// only one field
	int64 edge_size_extended = getEdgeSize<E, IdType>() + source_size();
	SplitterBuffer<IdType> *splitter = new SplitterBuffer<IdType>(matrix->get_file(), edge_size_extended, matrixProperties.vertices_partition * vertex_size, matrixProperties.vertices_partition, false, "updates", 1*matrixProperties.partitions);

	int64 source_offset;
	while (iterator.has_next()) {
		Block block = iterator.next();
		if (block.exist() && block.isSparse()) {
			LOG (INFO)<< "- PRE-PROCESSING BLOCK "<< block.get_row() << " -> " << block.get_col();
			if(row != block.get_row()) {
				row = block.get_row();
				in_loaded = false;
			}
			if(!in_loaded) {
				source_offset = block.get_col() * elements_partitions;
				LOG (INFO)<< "--- LOADING IN-ELEMENT STATES";
				load_fields<VSource, VDestination>(FieldType::SOURCE,source_offset);
				LOG (INFO)<< "--- IN-ELEMENT STATES BEETWEEN " << source_offset << " AND " << source_offset  + elements_partitions<< " LOADED";
				in_loaded = true;
			}

			/*
			 * PROCESSING EDGES
			 */

			MappedStream stream(block.get_file());
			IdType from, to;

			while(stream.has_remain()){
				from = stream.next<IdType>();
				to = stream.next<IdType>();
				splitter->add(from, to, source_pointer != 0? source_pointer->get_element(from): 0);
			}
			stream.close_stream();
			LOG (INFO)<< "- BLOCK "<< block.get_row() << " -> " << block.get_col() << " PRE-PROCESSED";
		}
	}
	splitter->flush();
	delete splitter;
}

template<class VSource, class VDestination, class E, class IdType, class MALGORITHM>
class InitializeOperator: public UnaryOperator<VDestination,IdType> {
	MALGORITHM *algorithm;
	MatrixWorker<E, IdType> *worker;

public:
	InitializeOperator(MALGORITHM *algorithm,
			MatrixWorker<E, IdType> *worker) {
		this->algorithm = algorithm;
		this->worker = worker;
	}

	void apply(Element<VDestination, IdType> & element, Element<VDestination, IdType>& out) {
		algorithm->initialize(*worker, out);
	}

};

template<class VSource, class VDestination, class E, class IdType, class MALGORITHM>
class AppyOperator: public UnaryOperator<VDestination, IdType> {
	MALGORITHM *algorithm;
	MatrixWorker<E, IdType> *worker;

public:
	AppyOperator(MALGORITHM *algorithm,
			MatrixWorker<E, IdType> *worker) {
		this->algorithm = algorithm;
		this->worker = worker;
	}

	void apply(Element<VDestination, IdType> & element, Element<VDestination, IdType>& out) {
		algorithm->apply(*(this->worker), out);
	}

};


/*

template<class VSource, class VDestination, class E, class IdType>
class InitializeOperator: public UnaryOperator<VDestination> {
	MAlgorithm<VSource, VDestination, E, IdType> *algorithm;
	MatrixWorker<E, IdType> *worker;

public:
	InitializeOperator(MAlgorithm<VSource, VDestination, E, IdType> *algorithm,
			MatrixWorker<E, IdType> *worker) {
		this->algorithm = algorithm;
		this->worker = worker;
	}

	void apply(Element<VDestination, IdType> & element, Element<VDestination>& out) {
		algorithm->initialize(*worker, out);
	}

};

template<class VSource, class VDestination, class E, class IdType>
class AppyOperator: public UnaryOperator<VDestination> {
	MAlgorithm<VSource, VDestination, E, IdType> *algorithm;
	MatrixWorker<E, IdType> *worker;

public:
	AppyOperator(MAlgorithm<VSource, VDestination, E, IdType> *algorithm,
			MatrixWorker<E, IdType> *worker) {
		this->algorithm = algorithm;
		this->worker = worker;
	}

	void apply(Element<VDestination> & element, Element<VDestination>& out) {
		algorithm->apply(*(this->worker), out);
	}

};
*/

}

#endif /* MFLASH_CPP_CORE_MATRIXWORKER_HPP_ */
