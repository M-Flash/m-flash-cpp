/*
 * operator.hpp
 *
 *  Created on: Feb 28, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_OPERATOR_HPP_
#define MFLASH_CPP_CORE_OPERATOR_HPP_

#include <iostream>

#include "type.hpp"

namespace mflash{

	template <class V>
	class Operator{virtual void dummy(){}
	};

	template <class V, class E>
	class EdgeOperator: public Operator<V>{
	public:
		virtual void gather(Element<V> &in, Element<V> &out, E &edge) = 0;
	};

	template <class V>
	class ZeroOperator: public Operator<V>{
	public:
		virtual void apply(Element<V> &element) = 0;
	};

	template <class V>
	class ZeroOperatorExtended: public Operator<V>{
	public:
		virtual V apply() = 0;
	};

	template <class V>
	class UnaryOperator: public Operator<V>{
	public:
		virtual void apply(Element<V> & element, Element<V>& out) = 0;
	};

	template <class V>
	class BinaryOperator: public Operator<V>{
	public:
		virtual void apply(Element<V> &left, Element<V> &right, Element<V> &out) = 0;
	};

	template <class V>
	class Reducer{
	public:
		virtual void initialize(V &out) = 0;
		virtual void sum(V &left, V &right, V &out) = 0;
	};



	template <class V>
	class UnaryReducer : public Reducer<V>, public UnaryOperator<V>{
	};

	template <class V>
	class BinaryReducer: public Reducer<V>, public BinaryOperator<V>{
	};


}
#endif /* MFLASH_CPP_CORE_OPERATOR_HPP_ */
