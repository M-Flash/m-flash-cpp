
/*
 * matrix.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MATRIX_HPP_
#define MFLASH_CPP_CORE_MATRIX_HPP_

#include <algorithm>
#include <iostream>
#include <string>

#include "../core/array.hpp"
#include "../core/operator.hpp"
#include "../core/type.hpp"
#include "../core/util.hpp"
#include "../core/vector.hpp"
#include "../core/blockiterator.hpp"
#include "../../log/easylogging++.h"

namespace mflash {

template<class VSource, class VDestination, class E, class IdType>
class MAlgorithm;

template<class E, class IdType>
class MatrixWorker;

/*

class MatrixInterface {
	virtual int64 size();
	virtual string get_file();
	virtual bool is_transpose();
	virtual int id_size();
	virtual int64 get_elements_by_block();
	virtual int size_edge();
	virtual int size_edge_data();
	virtual ~MatrixInterface();

};*/

template<class E, class IdType = int>
class Matrix { //: MatrixInterface{

	string file;
	//int64 n;
	//int64 m;
	bool transpose_;

	Mode mode;
	MatrixWorker<E, IdType> *worker;

	MatrixProperties *properties;

	//BlockType get_block_type(int block_count, int64 vertex_size_bytes,int64 edge_size_bytes);
public:
	Matrix(string file, bool transpose = false, Mode mode = Mode::VECTOR_REPLICATION/*, int64 size, int64 element_by_block*/);
	//Matrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode);
	int64 size() {
		return properties != NULL? properties->vertices : 0;//max(n, m);
	}
	Matrix<E, IdType> transpose();

	string get_file() {
		return file;
	}
	bool is_transpose() {
		return transpose_;
	}
	int id_size() {
		return sizeof(IdType);
	}

	MatrixProperties get_matrix_properties() {
		MatrixProperties properties;
		properties = *this->properties;
		return properties;
	}

	void set_matrix_properties(MatrixProperties &newProperties){
		if(properties != NULL) delete properties;

		properties = new MatrixProperties();
		*properties = newProperties;
	}


	int64 get_elements_by_block() {
		if(properties != NULL){
			return properties->vertices_partition;
		}
		return 0;
	}

	int size_edge();
	int size_edge_data();

	bool add_field(FieldType type, string fieldname, AbstractVector &vector) {
		return worker->add_field(type, fieldname, vector);
	}
	//bool add_field(FieldType type, string fieldname, AbstractVector *vector){return worker->add_field(type, fieldname, *vector);}

	bool remove_field(FieldType type, string fieldname) {
		return worker->remove_field(type, fieldname);
	}

	template<class MALGORITHM, class VDestination, class VSource>
	void operate(MALGORITHM &algorithm, Vector<VSource, IdType> &inVector, Vector<VDestination, IdType> &outVector);

	template<class MALGORITHM, class VDestination, class VSource = VDestination>
	void operate(MALGORITHM &algorithm, Vector<VDestination, IdType> &outVector);

	template<class MALGORITHM, class VDestination, class VSource>
	void operate(MALGORITHM &algorithm);
/*

	template<class MALGORITHM, class VDestination, class VSource>
	void operate(MALGORITHM &algorithm, PrimitiveVector<VSource, IdType> &inVector, PrimitiveVector<VDestination, IdType> &outVector);
*/

	template<class MALGORITHM, class VDestination, class VSource>
	void operate(MALGORITHM &algorithm,
	Vector<VSource, IdType> *inVector, Vector<VDestination, IdType> *outVector);

	void load();

};

//Matrix<E>::Matrix(string filebool transpose, int64 size, int64 element_by_block, Mode mode){
template<class E, class IdType>
Matrix<E, IdType>::Matrix(string graph, bool transpose, Mode mode) {
	this->file = graph;
	this->transpose_ = !transpose;
	this->mode = mode;
	this->properties = NULL;
	//check if
	if(exist_file(get_properties_file(graph))){
		MatrixProperties properties = load_matrix_properties(graph);
		set_matrix_properties(properties);
	}

	this->worker = new MatrixWorker<E, IdType>(*this);
	//this->n = 0;
	//this->m = 0;

}

template<class E, class IdType>
Matrix<E, IdType> Matrix<E, IdType>::transpose() {
	Matrix <E,IdType > t(*this);
	t.transpose_ = !t.transpose_;
	return (t);
}

template<class E, class IdType>
int Matrix<E, IdType>::size_edge_data() {
	int64 size = 0;
#if E != EmptyType
	size = sizeof(E);
#endif
	return size;
}

template<class E, class IdType>
int Matrix<E, IdType>::size_edge() {
	//We are considering always the format (source_id, destination_id, edge_data)
	int64 size = 0;
	size += 2 * sizeof(IdType);
	return size;
}

//first implementation without replicates :)

template<class E, class IdType>
template<class MALGORITHM, class VDestination, class VSource>
void Matrix<E, IdType>::operate(MALGORITHM &algorithm,
		Vector<VSource, IdType> *inVector, Vector<VDestination, IdType> *outVector) {

	worker->set_default_destination_field(0);
	worker->set_default_source_field(0);

	worker->set_default_source_field(inVector);
	worker->set_default_destination_field(outVector);

	load();

	//resizing vectors
	if(outVector != 0){
		outVector->resize(size());
	}
	if(inVector != 0){
		inVector->resize(size());
	}

	worker->operate<VSource, VDestination>(algorithm);

	worker->set_default_destination_field(0);
	worker->set_default_source_field(0);
}

template<class E, class IdType>
template<class MALGORITHM, class VDestination, class VSource>
void Matrix<E, IdType>::operate(MALGORITHM &algorithm,
		Vector<VSource, IdType> &inVector, Vector<VDestination, IdType> &outVector) {
	operate<MALGORITHM, VDestination, VSource>(algorithm, &inVector, &outVector);
}

template<class E, class IdType>
template<class MALGORITHM, class VDestination, class VSource>
void Matrix<E, IdType>::operate(
		MALGORITHM &algorithm,
		Vector<VDestination, IdType> &outVector) {
	operate<MALGORITHM, VDestination, VSource>(algorithm, 0, &outVector);
}

/*
template<class E, class IdType>
template<class VSource, class VDestination, class MALGORITHM>
void Matrix<E, IdType>::operate(
		MALGORITHM &algorithm,
		PrimitiveVector<VDestination> &outVector) {
	Vector<VDestination> *destination = &outVector;
	operate<EmptyField, VDestination>(algorithm, 0, &destination);
}
*/


template<class E, class IdType>
template<class MALGORITHM, class VDestination, class VSource>
void Matrix<E, IdType>::operate(MALGORITHM &algorithm) {
	operate<EmptyField, EmptyField>(algorithm, 0, 0);
}


/*
template<class E, class IdType>
template<class MALGORITHM, class VDestination, class VSource>
void Matrix<E, IdType>::operate(MALGORITHM &algorithm,
		PrimitiveVector<VSource, IdType> &inVector, PrimitiveVector<VDestination, IdType> &outVector) {
	operate<MALGORITHM, VDestination, VSource>(algorithm, &inVector, &outVector);
}
*/



template<class E, class IdType>
void Matrix<E, IdType>::load(){
	int64 element_size = worker->element_size();
	if(properties != NULL && properties->vertices_partition == getVeticesByPartition(element_size)){
		LOG(INFO) << "Graph formatting ommitted because current partitioning is compatible.";
	}else{
		MatrixProperties properties = convert<E,IdType>(file, validateElementSize(element_size));
		set_matrix_properties(properties);
	}
	//convert<E, IdType>(file, worker->element_size());
}




}
#endif /* MFLASH_CPP_CORE_MATRIX_HPP_ */
