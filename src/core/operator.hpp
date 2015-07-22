/*
 * operator.hpp
 *
 *  Created on: Feb 28, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_OPERATOR_HPP_
#define MFLASH_CPP_CORE_OPERATOR_HPP_

#include "type.hpp"

namespace mflash {

template<class V, class IdType>
class Operator {
public:
	virtual void dummy() {}
	virtual ~Operator(){}
};

template<class V, class IdType>
class ZeroOperator: public Operator<V, IdType> {
public:
	virtual void apply(Element<V, IdType> &element) = 0;
	virtual ~ZeroOperator(){}
};

template<class V, class IdType>
class ZeroOperatorExtended: public Operator<V, IdType> {
public:
	virtual V apply() = 0;
	virtual ~ZeroOperatorExtended(){};
};

template<class V, class IdType>
class UnaryOperator: public Operator<V, IdType> {
public:
	virtual void apply(Element<V, IdType> & element, Element<V, IdType> & out) = 0;
	virtual ~UnaryOperator(){};
};

template<class V1, class IdType, class V2 = V1, class VOut = V1>
class BinaryOperator: public Operator<VOut, IdType> {
public:
	virtual void apply(Element<V1,IdType> &left, Element<V2, IdType> &right,
			Element<VOut, IdType> &out) = 0;
	virtual ~BinaryOperator(){}
};

template<class V>
class Reducer {
public:
	virtual void initialize(V &out) = 0;
	virtual void sum(V &left, V &right, V &out) = 0;
	virtual ~Reducer(){}
};

template<class V, class IdType>
class UnaryReducer: public Reducer<V>, public UnaryOperator<V, IdType> {
};

template<class V1, class IdType, class V2 = V1, class VOut = V1>
class BinaryReducer: public Reducer<VOut>, public BinaryOperator<V1, IdType, V2, VOut> {
};

}
#endif /* MFLASH_CPP_CORE_OPERATOR_HPP_ */
