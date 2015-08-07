// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef MFLASH_CPP_CORE_VECTOR_HPP_
#define MFLASH_CPP_CORE_VECTOR_HPP_

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "../core/array.hpp"
#include "../core/operator.hpp"
#include "../core/type.hpp"
#include "../core/util.hpp"
#include "../core/linearcombination.hpp"
#include "../util/easylogging++.h"


using namespace std;

namespace mflash {

class AbstractVector {
public:
	virtual int64 element_size() = 0;
	virtual string get_file() = 0;
	virtual int64 load_region(int64 offset, int64 size, void* address) = 0;
	virtual void store_region(int64 offset, int64 size, void* address) = 0;
	virtual void resize(int64 size) = 0;

	virtual ~AbstractVector(){}
};

template<class V, class IdType>
class Vector: public AbstractVector {
protected:
	string file;
	int64 size;
	bool readonly;
	int64 elements_by_block;

	std::vector<OperationListener*> listeners;

	void invoke_operation_listener(int vector_id);

	static V operate(Operator<V,IdType> &operator_, Vector<V, IdType> &output, int n,
			Vector<V, IdType>* vectors[]);
	virtual ~Vector(){}

public:
	Vector(string file, int64 size = 0, int64 elements_by_block = 0);

	int64 element_size() {return sizeof(V);}

	//int64 get_size(){ return size;}

	string get_file() {return file;}

	void resize(int64 size);

	int64 load_region(int64 offset, int64 size, void* address);

	void store_region(int64 offset, int64 size, void* address);

	void add_listener(OperationListener *listener);

	void remove_listener(OperationListener *listener);

	static void operate(BinaryOperator<V,IdType> &moperator,
			BinaryOperator<V,IdType> &soperator, Vector<V, IdType> &output, int n,
			V constants[], Vector<V, IdType> *vectors[]);

	void operate(ZeroOperator<V,IdType> &operator_);

	void operate(UnaryOperator<V,IdType> &operator_, Vector<V, IdType> &output);

	void operate(BinaryOperator<V,IdType> &operator_, Vector<V, IdType> &vector2,
			Vector<V, IdType> &output);

	V operate(UnaryReducer<V,IdType> &operator_);

	V operate(BinaryReducer<V,IdType> &operator_, Vector<V, IdType> &vector2);
};

template<class V, class IdType>
Vector<V, IdType>::Vector(string file, int64 size, int64 elements_by_block) {
	this->file = file;
	this->size = size;
	this->readonly = false;
	this->elements_by_block =
			(elements_by_block <= 0 ? size : elements_by_block);

}

template<class V, class IdType>
V Vector<V, IdType>::operate(Operator<V,IdType> &operator_, Vector<V, IdType> &output, int n,
		Vector<V, IdType> *vectors[]) {

	int64 size = output.size;
	int64 elements_by_block = output.elements_by_block;


	if(elements_by_block == 0){
		elements_by_block = getVeticesByPartition(2 *sizeof(V));
	}

	int64 blocks = size / elements_by_block
			+ (size % elements_by_block == 0 ? 0 : 1);

	Array<V,IdType>* out = new Array<V,IdType>(elements_by_block);
	Array<V,IdType>* tmp = new Array<V,IdType>(elements_by_block);

	if (n == 0) {
		vectors = new Vector<V, IdType> *[1] { &output };
		n = 1; //vectors[0] = output;
	}

	V final_accumulator;
	V tmp_accumulator;

	Reducer<V> *reducer = dynamic_cast<Reducer<V> *>(&operator_);
	bool default_reducer = false;

	if (reducer == 0) {
		//reducer = new DefaultReducer<V,IdType>();
		default_reducer = true;
	} else {
		reducer->initialize(final_accumulator);
	}

	int64 offset = 0;
	int64 block_size = 0;

	LOG (INFO)<< "VECTOR OPERATION STARTED WITH " << blocks << " BLOCKS";
	for (int64 block = 0; block < blocks; block++) {
		LOG (INFO)<< "PROCESSINGN BLOCK " << block;
		offset = block * elements_by_block;
		block_size = min(elements_by_block, size - offset);

		//loading the first vector on the output
		vectors[0]->load_region(offset, block_size, out->address());
		out->set_limit(block_size);
		out->set_offset(offset);

		int vector_indx = 0;

		BinaryOperator<V,IdType> *binary_operator = dynamic_cast<BinaryOperator<V,IdType> *>( &operator_ );
		/*
		 * When is a binary operator the output is considered the first value, then is not loaded twice
		 */
		if(binary_operator != 0) {
			vector_indx = 1;
		}

		Vector<V, IdType> *v;
		do {
			output.invoke_operation_listener(vector_indx);

			v = vectors[vector_indx];
			//		cout << v->file <<endl;
			//		cout << output.file <<endl;
			if (v->file.compare(output.file) != 0) {
				/*if (v.inMemory) {
				 tmpAccumulator = ThreadDataType.operate(operator, out, new Array<V,IdType>(type, v.address + offset, size, 0), out);
				 } else {*/

				v->load_region(offset, block_size, tmp->address());
				tmp->set_offset(offset);
				tmp->set_limit(block_size);
				tmp_accumulator = Array<V,IdType>::operate(operator_, *out, *tmp, *out);
				//	}
			} else {
				tmp_accumulator = Array<V,IdType>::operate(operator_, *out, *out, *out);
				//tmpAccumulator = ThreadDataType.operate(operator, out, out, out);
			}
			if(!default_reducer) {
				reducer->sum(final_accumulator, tmp_accumulator,final_accumulator);
			}

		}while( ++vector_indx < n);

		// store the output
		if (default_reducer) {
			//logger.debug("Storing block {} ", block);
			output.store_region(offset, block_size, out->address());
		}
	}

	delete out;
	delete tmp;

	LOG (INFO)<< "VECTOR OPERATION FINALIZED";

	if (!default_reducer) {
		return final_accumulator;
	}
	return 0;
}

template<class V, class IdType>
void Vector<V, IdType>::operate(BinaryOperator<V,IdType> &moperator,
		BinaryOperator<V,IdType> &soperator, Vector<V, IdType> &output, int n, V constants[],
		Vector<V, IdType> *vectors[]) {

	if (n < 2)
		throw 12; //error

	LinearOperator<V,IdType> loperator(constants, &soperator, &moperator);
	OperationListener * listener = &loperator;
	output.add_listener(listener);
	Vector<V, IdType>::operate(loperator, output, n, vectors);
	output.remove_listener(listener);

}

template<class V, class IdType>
void Vector<V, IdType>::operate(ZeroOperator<V,IdType> &operator_) {
	operate(operator_, *this, 0, new Vector<V, IdType> *[0] { });
}

template<class V, class IdType>
void Vector<V, IdType>::operate(UnaryOperator<V,IdType> &operator_, Vector<V, IdType> &output) {
	operate(operator_, output, new Vector<V, IdType> *[1] { *this });
}

template<class V, class IdType>
void Vector<V, IdType>::operate(BinaryOperator<V,IdType> &operator_, Vector<V, IdType> &vector2,
		Vector<V, IdType> &output) {
	operate(operator_, output, new Vector<V, IdType> *[2] { *this, &vector2 });
}

template<class V, class IdType>
V Vector<V, IdType>::operate(UnaryReducer<V,IdType> &operator_) {
	operate(operator_, *this, new Vector<V, IdType> *[0] { });
}

template<class V, class IdType>
V Vector<V, IdType>::operate(BinaryReducer<V,IdType> &operator_, Vector<V, IdType> &vector2) {
	operate(operator_, *this, new Vector<V, IdType> *[2] { *this, &vector2 });
}

template<class V, class IdType>
inline void Vector<V, IdType>::resize(int64 size) {
	if(this->size <0)
		return;
	this->size = size;
}

template<class V, class IdType>
inline int64 Vector<V, IdType>::load_region(int64 offset, int64 size, void* address) {
	int64 element_size = this->element_size();
	size = min(size, this->size - offset) * element_size;

	offset *= element_size;

	if (!exist_file(this->file)) {
		return size / element_size;
	}

	ifstream *file = new ifstream(this->file, ios::in | ios::binary | ios::ate);
	if (file_size(this->file) >= offset + size) {
		file->seekg(offset, ios::beg);
		file->read((char*) address, size);
	}
	file->close();
	/*
	 FILE * pFile;
	 pFile = fopen (this->file.c_str(), "r");
	 fseek (pFile , 0 , SEEK_END);
	 if(file_size(this->file) > offset+size){
	 fseek(pFile, offset, SEEK_SET);
	 fread(address, 1,size , pFile);
	 }
	 fclose(pFile);
	 */

	return size / element_size;
}

template<class V, class IdType>
inline void Vector<V, IdType>::store_region(int64 offset, int64 size, void* address) {
	int64 element_size = this->element_size();
	size = min(size, this->size - offset) * element_size;
	offset *= element_size;

	auto properties = ios::out | ios::binary | ios::ate;
	if (exist_file(this->file)) {
		properties |= ios::in;
	}

	ofstream file(this->file, properties);
	if (!file.good()) {
		file.close();
		return;
	}
	file.seekp(offset, ios::beg);
	file.write((char*) address, size);
	file.close();
}

template<class V, class IdType>
void Vector<V, IdType>::add_listener(OperationListener *listener) {
	if (listeners.end()
			!= std::find(listeners.begin(), listeners.end(), listener))
		return;
	listeners.push_back(listener);
}

template<class V, class IdType>
void Vector<V, IdType>::remove_listener(OperationListener *listener) {
	std::vector<OperationListener*>::iterator iter = std::find(
			listeners.begin(), listeners.end(), listener);
	if (listeners.end() == iter)
		return;

	listeners.erase(iter);
}

template<class V, class IdType>
void Vector<V, IdType>::invoke_operation_listener(int vector_id) {
	OperationEvent event(vector_id);
	for (std::vector<OperationListener*>::iterator iter = listeners.begin();
			iter != listeners.end(); ++iter)
		(*iter)->on_change(event);
}
}
#endif /* MFLASH_CPP_CORE_VECTOR_HPP_ */
