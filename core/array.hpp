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

template<class V>
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

	static V operate(Operator<V> &operator_, Array<V> &left, Array<V> &right,
			Array<V> &out);
	static V operate(ZeroOperator<V> &operator_, Array<V> &left);
	static V operate(UnaryOperator<V> &operator_, Array<V> &left,
			Array<V> &out);
	static V operate(BinaryOperator<V> &operator_, Array<V> &left,
			Array<V> &right, Array<V> &out);
	static V operate(UnaryReducer<V> &operator_, Array<V> &left);
	static V operate(BinaryReducer<V> &operator_, Array<V> &left,
			Array<V> &right);

};

template<class V>
class ThreadDataType {
protected:
	int id;
	Array<V> *left;
	Array<V> *right;
	Array<V> *out;
	int64 step;
	int64 size;
	int64 initIdx;
	int64 stepIdx;

	int64 left_offset;
	int64 right_offset;
	int64 out_offset;
	Operator<V> *operator_;

public:
	ThreadDataType(int id, Operator<V> &operator_, Array<V> &left,
			Array<V> &right, Array<V> &out);

};

template<class V>
class ZeroThreadDataType: public ThreadDataType<V> {
public:
	ZeroThreadDataType(int id, Operator<V>& operator_, Array<V> &left,
			Array<V> &right, Array<V> &out) :
			ThreadDataType<V>(id, operator_, left, right, out) {
	}
	V call();
};

template<class V>
class UnaryThreadDataType: protected ThreadDataType<V> {
public:
	UnaryThreadDataType(int id, Operator<V>& operator_, Array<V> &left,
			Array<V> &right, Array<V> &out) :
			ThreadDataType<V>(id, operator_, left, right, out) {
	}
	V call();
};

template<class V>
class BinaryThreadDataType: public ThreadDataType<V> {
public:
	BinaryThreadDataType(int id, Operator<V>& operator_, Array<V> &left,
			Array<V> &right, Array<V> &out) :
			ThreadDataType<V>(id, operator_, left, right, out) {
	}
	V call();
};

template<class V>
class UnaryReducerThreadDataType: public ThreadDataType<V> {
public:
	UnaryReducerThreadDataType(int id, Operator<V>& operator_, Array<V> &left,
			Array<V> &right, Array<V> &out) :
			ThreadDataType<V>(id, operator_, left, right, out) {
	}
	V call();
};

template<class V>
class BinaryReducerThreadDataType: public ThreadDataType<V> {
public:
	BinaryReducerThreadDataType(int id, Operator<V>& operator_, Array<V> &left,
			Array<V> &right, Array<V> &out) :
			ThreadDataType<V>(id, operator_, left, right, out) {
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
	*(offset_address
			+ (element_shifted ? pos << element_shift : pos * element_size_)) =
			*value;
}

/*
 Array
 */

template<class V>
inline V* Array<V>::get_element(int64 pos) {
	return offset_address_wrapped + pos;
}

template<class V>
inline void Array<V>::set_element(int64 pos, V* value) {
	*(offset_address_wrapped + pos) = *value;
}

template<class V>
inline void Array<V>::set_offset(int64 pos) {
	GenericArray::set_offset(pos);
	offset_address_wrapped = (V*) offset_address;
}

template<class V>
V Array<V>::operate(Operator<V> &operator_, Array<V> &left, Array<V> &right,
		Array<V> &out) {
	V value;
	UnaryReducer<V> * unary_reducer_ptr =
			dynamic_cast<UnaryReducer<V> *>(&operator_);
	BinaryReducer<V> * binary_reducer_ptr =
			dynamic_cast<BinaryReducer<V> *>(&operator_);
	ZeroOperator<V> * zero_ptr = dynamic_cast<ZeroOperator<V> *>(&operator_);
	UnaryOperator<V> * unary_ptr = dynamic_cast<UnaryOperator<V> *>(&operator_);
	BinaryOperator<V> * binary_ptr =
			dynamic_cast<BinaryOperator<V> *>(&operator_);

	if (unary_reducer_ptr != 0) {
		return Array<V>::operate(*unary_reducer_ptr, left);
	}

	if (binary_reducer_ptr != 0) {
		return Array<V>::operate(*binary_reducer_ptr, left, right);
	}

	if (zero_ptr != 0) {
		return Array<V>::operate(*zero_ptr, left);
	}

	if (unary_ptr != 0) {
		return Array<V>::operate(*unary_ptr, left, out);
	}

	if (binary_ptr != 0) {
		return Array<V>::operate(*binary_ptr, left, right, out);
	}

	return value;
}

template<class V>
V Array<V>::operate(ZeroOperator<V> &operator_, Array<V> &left) {
	V value;
	ZeroThreadDataType<V> t(0, operator_, left, left, left);
	t.call();
	return value;
}

template<class V>
V Array<V>::operate(UnaryOperator<V> &operator_, Array<V> &left,
		Array<V> &out) {
	V value;
	UnaryThreadDataType<V> t(0, operator_, left, left, out);
	t.call();
	return value;
}

template<class V>
V Array<V>::operate(BinaryOperator<V> &operator_, Array<V> &left,
		Array<V> &right, Array<V> &out) {
	V value;
	BinaryThreadDataType<V> t(0, operator_, left, right, out);
	t.call();
	return value;
}

template<class V>
V Array<V>::operate(UnaryReducer<V> &operator_, Array<V> &left) {
	UnaryReducerThreadDataType<V> t(0, operator_, left, left, left);
	return t.call();
}

template<class V>
V Array<V>::operate(BinaryReducer<V> &operator_, Array<V> &left,
		Array<V> &right) {
	BinaryReducerThreadDataType<V> t(0, operator_, left, right, left);
	return t.call();
}

/*
 ThreadDataType
 */

template<class V> inline ThreadDataType<V>::ThreadDataType(int id,
		Operator<V> &operator_, Array<V> &left, Array<V> &right,
		Array<V> &out) {
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

template<class V> inline V ZeroThreadDataType<V>::call() {
	V value;
	ZeroOperator<V>* operator_ = (ZeroOperator<V>*) (this->operator_);
	Element<V> element;

	while (this->left_offset < this->size) {
		element.id = this->left_offset;
		element.value = this->out->get_element(this->left_offset);
		operator_->apply(element);
		this->left_offset += this->step;
	}

	return value;

}

template<class V> inline V UnaryThreadDataType<V>::call() {
	UnaryOperator<V>* operator_ = (UnaryOperator<V>*) (this->operator_);
	V value;
	Element<V> element;
	Element<V> out;

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

template<class V> inline V BinaryThreadDataType<V>::call() {
	BinaryOperator<V>* operator_ = (BinaryOperator<V>*) (this->operator_);

	V value;
	Element<V> element1;
	Element<V> element2;
	Element<V> out;

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

template<class V> inline V UnaryReducerThreadDataType<V>::call() {
	UnaryReducer<V>* operator_ = (UnaryReducer<V>*) (this->operator_);

	V tmp;

	Element<V> element;
	Element<V> out;
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

template<class V> inline V BinaryReducerThreadDataType<V>::call() {
	BinaryReducer<V>* operator_ = (BinaryReducer<V>*) (this->operator_);

	V tmp;

	Element<V> element1;
	Element<V> element2;
	Element<V> out;
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
