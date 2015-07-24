/*
 * GenericArray.hpp
 *
 *  Created on: Feb 28, 2015
 *      Author: Hugo Gualdron
 */

#ifndef MFLASH_CPP_CORE_ARRAY_HPP_
#define MFLASH_CPP_CORE_ARRAY_HPP_

#include "../core/operator.hpp"
#include "../core/type.hpp"
#include "../core/util.hpp"
#include <cmath>

/**
 * The implementation suppose that the architecture is 64bits then we don't need an set of buffer to map all the region on memory.
 */
namespace mflash {

class GenericArray {
protected:
	char* address_;
	int64 limit_;
	int64 element_size_;
	int64 size_;
	int64 offset_;
	char* offset_address;

	bool wasAllocated_;

	int64 element_shift;
	bool element_shifted;

	void init();

public:
	GenericArray() {}

	GenericArray(int64 element_size, int64 size, int64 offset);

	GenericArray(int64 element_size, int64 size) :
			GenericArray(element_size, size, 0) {
	}

	/**
	 * Create an GenericArray pointer using starting in address to size + adress.
	 * The offset is the index offset that represent the first position.
	 */
	GenericArray(char* address, int64 element_size, int64 size, int64 offset);
	virtual ~GenericArray();

	char* address() {
		return address_;
	}
	int64 size();
	int64 limit();
	void set_limit(int64 limit);
	int64 offset();
	void set_offset(int64 offset);
	char* get_element(int64 pos);
	void set_element(int64 pos, char* value);
	bool was_allocated() {return wasAllocated_;}
	void free_memory();

};

template<class V, class IdType>
class Array: public GenericArray {
protected:
	V* address_wrapped;
	V* offset_address_wrapped;

public:

	Array(int64 size, int64 offset) :
			GenericArray(sizeof(V), size, offset) {
		this->address_wrapped = (V*) address_;
		this->offset_address_wrapped = (V*) offset_address;
	}

	Array(int64 size) :
			Array(size, 0) {
	}

	V* address() {
		return address_wrapped;
	}
	V* get_element(int64 pos);
	void set_element(int64 pos, V* value);
	void set_offset(int64 offset);

	static V operate(Operator<V, IdType> &operator_, Array<V, IdType> &left, Array<V, IdType> &right,
			Array<V, IdType> &out);
	static V operate(ZeroOperator<V, IdType> &operator_, Array<V, IdType> &left);
	static V operate(UnaryOperator<V, IdType> &operator_, Array<V, IdType> &left,
			Array<V, IdType> &out);
	static V operate(BinaryOperator<V, IdType> &operator_, Array<V, IdType> &left,
			Array<V, IdType> &right, Array<V, IdType> &out);
	static V operate(UnaryReducer<V, IdType> &operator_, Array<V, IdType> &left);
	static V operate(BinaryReducer<V, IdType> &operator_, Array<V, IdType> &left,
			Array<V, IdType> &right);

};

template<class V, class IdType>
class ThreadDataType {
protected:
	int id;
	Array<V, IdType> *left;
	Array<V, IdType> *right;
	Array<V, IdType> *out;
	int64 step;
	int64 size;
	int64 initIdx;
	int64 stepIdx;

	int64 left_offset;
	int64 right_offset;
	int64 out_offset;
	Operator<V, IdType> *operator_;

public:
	ThreadDataType(int id, Operator<V, IdType> &operator_, Array<V, IdType> &left,
			Array<V, IdType> &right, Array<V, IdType> &out);

};

template<class V, class IdType>
class ZeroThreadDataType: public ThreadDataType<V, IdType> {
public:
	ZeroThreadDataType(int id, Operator<V, IdType>& operator_, Array<V, IdType> &left,
			Array<V, IdType> &right, Array<V, IdType> &out) :
			ThreadDataType<V, IdType>(id, operator_, left, right, out) {
	}
	V call();
};

template<class V, class IdType>
class UnaryThreadDataType: protected ThreadDataType<V, IdType> {
public:
	UnaryThreadDataType(int id, Operator<V, IdType>& operator_, Array<V, IdType> &left,
			Array<V, IdType> &right, Array<V, IdType> &out) :
			ThreadDataType<V, IdType>(id, operator_, left, right, out) {
	}
	V call();
};

template<class V, class IdType>
class BinaryThreadDataType: public ThreadDataType<V, IdType> {
public:
	BinaryThreadDataType(int id, Operator<V, IdType>& operator_, Array<V, IdType> &left,
			Array<V, IdType> &right, Array<V, IdType> &out) :
			ThreadDataType<V, IdType>(id, operator_, left, right, out) {
	}
	V call();
};

template<class V, class IdType>
class UnaryReducerThreadDataType: public ThreadDataType<V, IdType> {
public:
	UnaryReducerThreadDataType(int id, Operator<V, IdType>& operator_, Array<V, IdType> &left,
			Array<V, IdType> &right, Array<V, IdType> &out) :
			ThreadDataType<V, IdType>(id, operator_, left, right, out) {
	}
	V call();
};

template<class V, class IdType>
class BinaryReducerThreadDataType: public ThreadDataType<V, IdType> {
public:
	BinaryReducerThreadDataType(int id, Operator<V, IdType>& operator_, Array<V, IdType> &left,
			Array<V, IdType> &right, Array<V, IdType> &out) :
			ThreadDataType<V, IdType>(id, operator_, left, right, out) {
	}
	V call();
};

/*
 GenericArray
 */

/**
 * @param is the number of items on the GenericArray.
 */
GenericArray::GenericArray(int64 element_size, int64 size, int64 offset) {
	this->address_ = new char[element_size * size];
	this->wasAllocated_ = true;
	this->size_ = size;
	this->element_size_ = element_size;

	int64 value = log2(element_size);

	if (pow(2, value) == element_size) {
		element_shifted = true;
		element_shift = value;
	} else {
		element_shift = 0;
		element_shifted = false;
	}

	set_offset(offset);
	set_limit(size);
}

/**
 * @param is the number of items on the GenericArray.
 */
GenericArray::GenericArray(char* address, int64 element_size, int64 size,
		int64 offset) {
	this->address_ = address;
	this->wasAllocated_ = false;
	this->size_ = size;
	set_offset(offset);
	set_limit(size);
}

GenericArray::~GenericArray() {
	free_memory();
}

inline void GenericArray::free_memory() {
	if (this->wasAllocated_) {
		delete[] this->address_;
		this->wasAllocated_ = false;
	}
}

inline void GenericArray::set_limit(int64 limit) {
	if (limit < 0 || limit > size_) {
		throw 20; //MException("Limit value out off the interval " + 0 + " and " + size);
	}
	this->limit_ = limit;
}

inline void GenericArray::set_offset(int64 offset) {
	this->offset_ = offset;
	this->offset_address = address_ - offset * element_size_;
}

inline int64 GenericArray::offset() {
	return this->offset_;
}

inline int64 GenericArray::limit() {
	return this->limit_;
}

inline int64 GenericArray::size() {
	return this->size_;
}

inline char* GenericArray::get_element(int64 pos) {
	return offset_address
			+ (element_shifted ? pos << element_shift : pos * element_size_);
}

inline void GenericArray::set_element(int64 pos, char* value) {
	/**(offset_address
			+ (element_shifted ? pos << element_shift : pos * element_size_)) =
			*value;*/
	memcpy(offset_address+ (element_shifted ? pos << element_shift : pos * element_size_), value, element_size_);
}

/*
 Array
 */

template<class V, class IdType>
inline V* Array<V, IdType>::get_element(int64 pos) {
	return offset_address_wrapped + pos;
}

template<class V, class IdType>
inline void Array<V, IdType>::set_element(int64 pos, V* value) {
	//memcpy(offset_address_wrapped + pos, value, element_size_);
	*(offset_address_wrapped + pos) = *value;
}

template<class V, class IdType>
inline void Array<V, IdType>::set_offset(int64 pos) {
	GenericArray::set_offset(pos);
	offset_address_wrapped = (V*) offset_address;
}

template<class V, class IdType>
V Array<V, IdType>::operate(Operator<V, IdType> &operator_, Array<V, IdType> &left, Array<V, IdType> &right,
		Array<V, IdType> &out) {
	V value;
	UnaryReducer<V, IdType> * unary_reducer_ptr =
			dynamic_cast<UnaryReducer<V, IdType> *>(&operator_);
	BinaryReducer<V, IdType> * binary_reducer_ptr =
			dynamic_cast<BinaryReducer<V, IdType> *>(&operator_);
	ZeroOperator<V, IdType> * zero_ptr = dynamic_cast<ZeroOperator<V, IdType> *>(&operator_);
	UnaryOperator<V, IdType> * unary_ptr = dynamic_cast<UnaryOperator<V, IdType> *>(&operator_);
	BinaryOperator<V, IdType> * binary_ptr =
			dynamic_cast<BinaryOperator<V, IdType> *>(&operator_);

	if (unary_reducer_ptr != 0) {
		return Array<V, IdType>::operate(*unary_reducer_ptr, left);
	}

	if (binary_reducer_ptr != 0) {
		return Array<V, IdType>::operate(*binary_reducer_ptr, left, right);
	}

	if (zero_ptr != 0) {
		return Array<V, IdType>::operate(*zero_ptr, left);
	}

	if (unary_ptr != 0) {
		return Array<V, IdType>::operate(*unary_ptr, left, out);
	}

	if (binary_ptr != 0) {
		return Array<V, IdType>::operate(*binary_ptr, left, right, out);
	}

	return value;
}

template<class V, class IdType>
V Array<V, IdType>::operate(ZeroOperator<V, IdType> &operator_, Array<V, IdType> &left) {
	V value;
	ZeroThreadDataType<V, IdType> t(0, operator_, left, left, left);
	t.call();
	return value;
}

template<class V, class IdType>
V Array<V, IdType>::operate(UnaryOperator<V, IdType> &operator_, Array<V, IdType> &left,
		Array<V, IdType> &out) {
	V value;
	UnaryThreadDataType<V, IdType> t(0, operator_, left, left, out);
	t.call();
	return value;
}

template<class V, class IdType>
V Array<V, IdType>::operate(BinaryOperator<V, IdType> &operator_, Array<V, IdType> &left,
		Array<V, IdType> &right, Array<V, IdType> &out) {
	V value;
	BinaryThreadDataType<V, IdType> t(0, operator_, left, right, out);
	t.call();
	return value;
}

template<class V, class IdType>
V Array<V, IdType>::operate(UnaryReducer<V, IdType> &operator_, Array<V, IdType> &left) {
	UnaryReducerThreadDataType<V, IdType> t(0, operator_, left, left, left);
	return t.call();
}

template<class V, class IdType>
V Array<V, IdType>::operate(BinaryReducer<V, IdType> &operator_, Array<V, IdType> &left,
		Array<V, IdType> &right) {
	BinaryReducerThreadDataType<V, IdType> t(0, operator_, left, right, left);
	return t.call();
}

/*
 ThreadDataType
 */

template<class V, class IdType> inline ThreadDataType<V, IdType>::ThreadDataType(int id,
		Operator<V, IdType> &operator_, Array<V, IdType> &left, Array<V, IdType> &right,
		Array<V, IdType> &out) {
	this->id = id;
	this->left = &left;
	this->right = &right;
	this->out = &out;
	this->operator_ = &operator_;
	this->left_offset = 0;
	this->right_offset = 0;
	this->out_offset = 0;
	this->size = left.limit() + left.offset();

	//this->init = id;// * sizeof(V);
	this->step = MFLASH_VECTOR_THREADS; // * sizeof(V);

	this->left_offset = left.offset() + id;

	if (&right != 0) {
		this->right_offset = right.offset() + id;
	}
	if (&out != 0) {
		this->out_offset = out.offset() + id;
	}

	this->initIdx = id;
	this->stepIdx = MFLASH_VECTOR_THREADS;

}

template<class V, class IdType> inline V ZeroThreadDataType<V, IdType>::call() {
	V value;
	ZeroOperator<V, IdType>* operator_ = (ZeroOperator<V, IdType>*) (this->operator_);
	Element<V, IdType> element;

	while (this->left_offset < this->size) {
		element.id = this->left_offset;
		element.value = this->out->get_element(this->left_offset);
		operator_->apply(element);
		this->left_offset += this->step;
	}

	return value;

}

template<class V, class IdType> inline V UnaryThreadDataType<V, IdType>::call() {
	UnaryOperator<V, IdType>* operator_ = (UnaryOperator<V, IdType>*) (this->operator_);
	V value;
	Element<V, IdType> element;
	Element<V, IdType> out;

	while (this->left_offset < this->size) {
		element.id = this->left_offset;
		out.id = this->out_offset;

		element.value = this->left->get_element(this->left_offset);
		out.value = this->out->get_element(this->out_offset);

		operator_->apply(element, out);

		this->left_offset += this->step;
		this->out_offset += this->step;
	}

	//for(int64 i=this->init; i<this->size; i+=this->step, left_offset+=this->step, out_offset+=this->step){

	return value;
}

template<class V, class IdType> inline V BinaryThreadDataType<V, IdType>::call() {
	BinaryOperator<V, IdType>* operator_ = (BinaryOperator<V, IdType>*) (this->operator_);

	V value;
	Element<V, IdType> element1;
	Element<V, IdType> element2;
	Element<V, IdType> out;

	while (this->left_offset < this->size) {

		element1.id = this->left_offset;
		element2.id = this->right_offset;
		out.id = this->out_offset;

		element1.value = this->left->get_element(this->left_offset);
		element2.value = this->right->get_element(this->right_offset);
		out.value = this->out->get_element(this->out_offset);

		operator_->apply(element1, element2, out);

		this->left_offset += this->step;
		this->right_offset += this->step;
		this->out_offset += this->step;
	}
	return value;
}

template<class V, class IdType> inline V UnaryReducerThreadDataType<V, IdType>::call() {
	UnaryReducer<V, IdType>* operator_ = (UnaryReducer<V, IdType>*) (this->operator_);

	V tmp;

	Element<V, IdType> element;
	Element<V, IdType> out;
	out.value = &tmp;

	V value;
	operator_->initialize(value);
	while (this->left_offset < this->size) {
		element.id = this->left_offset;
		out.id = this->left_offset;

		element.value = this->left->get_element(this->left_offset);
		operator_->apply(element, out);
		operator_->sum(value, tmp, value);

		this->left_offset += this->step;
	}
	return value;
}

template<class V, class IdType> inline V BinaryReducerThreadDataType<V, IdType>::call() {
	BinaryReducer<V, IdType>* operator_ = (BinaryReducer<V, IdType>*) (this->operator_);

	V tmp;

	Element<V, IdType> element1;
	Element<V, IdType> element2;
	Element<V, IdType> out;
	out.value = &tmp;

	V value;
	operator_->initialize(value);
	while (this->left_offset < this->size) {
		element1.id = this->left_offset;
		element2.id = this->right_offset;
		out.id = this->left_offset;

		element1.value = this->left->get_element(this->left_offset);
		element2.value = this->right->get_element(this->right_offset);
		operator_->apply(element1, element2, out);
		operator_->sum(value, tmp, value);

		this->left_offset += this->step;
		this->right_offset += this->step;
	}
	return value;
}
}
#endif /* MFLASH_CPP_CORE_ARRAY_HPP_ */
