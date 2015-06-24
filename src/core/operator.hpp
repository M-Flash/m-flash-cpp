/*
 * operator.hpp
 *
 *  Created on: Feb 28, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_OPERATOR_HPP_
#define MFLASH_CPP_CORE_OPERATOR_HPP_

#include <iostream>

#include "../core/type.hpp"

namespace mflash {

template<class V>
class Operator {
	virtual void dummy() {}
	virtual ~Operator(){}
};

template<class V>
class ZeroOperator: public Operator<V> {
public:
	virtual void apply(Element<V> &element) = 0;
	virtual ~ZeroOperator(){}
};

template<class V>
class ZeroOperatorExtended: public Operator<V> {
public:
	virtual V apply() = 0;
	virtual ~ZeroOperatorExtended(){};
};

template<class V>
class UnaryOperator: public Operator<V> {
public:
	virtual void apply(Element<V> & element, Element<V>& out) = 0;
	virtual ~UnaryOperator();
};

template<class V1, class V2 = V1, class VOut = V1>
class BinaryOperator: public Operator<VOut> {
public:
	virtual void apply(Element<V1> &left, Element<V2> &right,
			Element<VOut> &out) = 0;
	virtual ~BinaryOperator(){}
};

template<class V>
class Reducer {
public:
	virtual void initialize(V &out) = 0;
	virtual void sum(V &left, V &right, V &out) = 0;
	virtual ~Reducer(){}
};

template<class V>
class UnaryReducer: public Reducer<V>, public UnaryOperator<V> {
};

template<class V1, class V2 = V1, class VOut = V1>
class BinaryReducer: public Reducer<VOut>, public BinaryOperator<V1, V2, VOut> {
};

}
#endif /* MFLASH_CPP_CORE_OPERATOR_HPP_ */
