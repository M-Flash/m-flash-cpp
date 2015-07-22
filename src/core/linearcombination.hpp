/*
 * cvector.hpp
 *
 *  Created on: Mar 2, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_LINEARCOMBINATION_HPP_
#define MFLASH_CPP_CORE_LINEARCOMBINATION_HPP_

#include "operator.hpp"
#include "type.hpp"
#include "vector.hpp"

namespace mflash{

	class OperationEvent{
		int vector_id;
		public:
			OperationEvent(int vectorId) {
				this->vector_id = vectorId;
			}

			int get_vector_id(){
				return vector_id;
			}
	};

	class OperationListener {
		public:
			virtual void on_change(OperationEvent &e) = 0;
			virtual ~OperationListener();
  };



  /**
	 *
	 * Make the linear combination between vectors. It uses two binary operators soperator (+) and moperator (*).
	 * Y = A*V1 + B*V2 + ... + N*Vn
	 *
	 * @author Hugo Gualdron
	 *
	 * @param <V>
	 */
	template <class V>
	class LinearOperator : public OperationListener, public BinaryOperator<V>{

			V * constants;
			BinaryOperator<V> *soperator;
			BinaryOperator<V> *moperator;

			int currentVectorId;
			Element<V> currentConstant;
			Element<V> lastConstant;

			/**
			 * Attribute used to determined when apply the two constants.
			 */
			bool combined;

		public:
		LinearOperator(V constants[], BinaryOperator<V> *soperator, BinaryOperator<V> *moperator) {
			this->constants = constants;
			this->soperator = soperator;
			this->moperator = moperator;
			this->currentVectorId = -1;
			combined = false;
		}


		void on_change(OperationEvent &e) {
			combined = false;
			if(e.get_vector_id() == 1){
				combined = true;
				lastConstant.value = &constants[0];
				lastConstant.id = 0;
			}

			currentVectorId = e.get_vector_id();

			currentConstant.value = &constants[currentVectorId];
			currentConstant.id = currentVectorId;

		}

		void apply(Element<V> &left, Element<V> &right, Element<V> &out) {
			if(combined){
				moperator->apply(lastConstant, left ,left);
			}
			moperator->apply(currentConstant, right, right);
			soperator->apply(left, right, out);
		}

	};
}
#endif /* MFLASH_CPP_CORE_LINEARCOMBINATION_HPP_ */
