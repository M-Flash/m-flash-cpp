/*
 * vector
 *
 *  Created on: Feb 28, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_PRIMITIVEVECTOR_HPP_
#define MFLASH_CPP_CORE_PRIMITIVEVECTOR_HPP_

#include <cstdlib>
#include <ctime>
#include <string>

#include "../core/operator.hpp"
#include "../core/type.hpp"
#include "../core/vector.hpp"

using namespace std;

namespace mflash{

	template <class V, class IdType >
	class PrimitiveVectorBinarySum : public BinaryOperator<V, IdType>{
		public:
			inline void apply(Element<V, IdType> &left, Element<V, IdType> &right, Element<V, IdType> &out){
				(*out.value) = (*left.value) + (*right.value);
			}
	};

	template <class V, class IdType >
		class PrimitiveVectorBinaryMul : public BinaryOperator<V, IdType>{
			public:
				inline void apply(Element<V, IdType> &left, Element<V, IdType> &right, Element<V, IdType> &out){
					(*out.value) = (*left.value) * (*right.value);
				}
		};

	template <class V, class IdType >
		class PrimitiveVectorProductPoint: public BinaryReducer<V,IdType>{
			public:
				inline void initialize(V &out) {
					out = 0;
				}
				inline void sum(V &left, V &right, V &out){
					out = right + left;
				}
				inline void apply(Element<V,IdType> &left, Element<V,IdType> &right, Element<V,IdType> &out){
					(*out.value) = (*left.value) * (*right.value);
				}
		};

	template <class V, class IdType >
		class PrimitiveVectorPNorm: public UnaryReducer<V, IdType>{
				int p;
				bool even;

			public:
				PrimitiveVectorPNorm(int p){
					this->p = p;
					even = !(p%2);
				}

				inline void initialize(V &out) {
					out = 0;
				}
				inline void sum(V &left, V &right, V &out){
					out = right + left;
				}
				inline void apply(Element<V, IdType> &left, Element<V, IdType> &out){
					if(even){
							(*out.value) = pow((*left.value), p);
					}else{
							(*out.value) = pow(abs((*left.value)), p);
					}
				}
		};

		template <class V, class IdType>
		class PrimitiveVectorFillRandom: public ZeroOperator<V,IdType>{
			public:
				PrimitiveVectorFillRandom(){srand (time(NULL));}
				inline void apply(Element<V, IdType> &out){
					(*out.value) = (V)rand()/ RAND_MAX;
				}
		};

		template <class V, class IdType>
		class PrimitiveVectorFill: public ZeroOperator<V,IdType>{
				V value;
			public:
				PrimitiveVectorFill(V value){this->value = value;}
				inline void apply(Element<V, IdType> &out){
					(*out.value) = value;
				}
		};

		template <class V, class IdType>
		class PrimitiveVectorMultiplyConstant: public UnaryOperator<V,IdType>{
				V value;
			public:
				PrimitiveVectorMultiplyConstant(V value){this->value = value;}
				inline void apply(Element<V, IdType> &element, Element<V, IdType> &out){
					(*out.value) = *(element.value) * value;
				}
		};

		template <class V, class IdType>
		class PrimitiveVectorAddConstant: public UnaryOperator<V,IdType>{
				V value;
			public:
				PrimitiveVectorAddConstant(V value){this->value = value;}
				inline void apply(Element<V, IdType> &element, Element<V, IdType> &out){
					(*out.value) = *(element.value) + value;
				}
		};



	template <class V, class IdType>
	class PrimitiveVector : public Vector<V,IdType>{
		bool transpose_;

		public:
			PrimitiveVector(string file, int64 size = 0, int64 elements_by_block = 0, bool transpose = false) :
				Vector<V,IdType>::Vector(file, size, elements_by_block){
				this->transpose_ = transpose;
			}

			void add(V value);
			void add(V value, PrimitiveVector &output);
			void add(PrimitiveVector &vector);
			void add(PrimitiveVector &vector, PrimitiveVector &output);

			void multiply(V value);
			void multiply(V value, PrimitiveVector &output);
			V multiply(PrimitiveVector &vector);
			V multiply(PrimitiveVector &vector, PrimitiveVector &output);

			V pnorm(int p);

			void fill(V p);
			void fill_random();

			bool is_transpose(){return transpose_;}
			PrimitiveVector<V, IdType> transpose(){
				PrimitiveVector<V, IdType> v(this->file, this->size, this->elements_by_block, !transpose_);
				return (v);
			};

			void linear_combination(int n, V constants[], PrimitiveVector<V, IdType> *vectors[]);
			//static V operate(Operator<V> &operator_, PrimitiveVector<V, IdType> &output,  int n, PrimitiveVector<V, IdType> vectors[]);
	};

	/**
	 * output = this + vector2
	 */
	template <class V, class IdType>
	void PrimitiveVector<V, IdType>::add(PrimitiveVector &vector2, PrimitiveVector &output){
		PrimitiveVectorBinarySum<V,IdType> operator_;
		Vector<V,IdType>::operate(operator_, output,  2, new Vector<V,IdType>[2]{*this, vector2});
	}

	/**
	 * output += vector2
	 */
	template <class V, class IdType>
	void PrimitiveVector<V, IdType>::add(PrimitiveVector &vector2){
		add(vector2, *this);
	}

	/**
	 * this += value
	 */
	template <class V, class IdType>
	void PrimitiveVector<V, IdType>::add(V value){
			PrimitiveVectorAddConstant<V,IdType> operator_(value);
			Vector<V,IdType>::operate(operator_, *this,  1, new Vector<V,IdType>[1]{*this});
	}

	/**
	 * this += value
	 */
	template <class V, class IdType>
	void PrimitiveVector<V, IdType>::add(V value, PrimitiveVector<V, IdType> &out){
			PrimitiveVectorAddConstant<V,IdType> operator_(value);
			Vector<V,IdType>::operate(operator_, out,  1, new Vector<V,IdType>[1]{*this});
	}

	/**
	 * this *= value
	 */
	template <class V, class IdType>
	void PrimitiveVector<V, IdType>::multiply(V value){
			PrimitiveVectorMultiplyConstant<V,IdType> operator_(value);
			Vector<V,IdType>::operate(operator_, *this,  1, new Vector<V,IdType> *[1]{this});
	}

	/**
	 * this *= value
	 */
	template <class V, class IdType>
	void PrimitiveVector<V, IdType>::multiply(V value, PrimitiveVector &output){
			PrimitiveVectorMultiplyConstant<V,IdType> operator_(value);
			Vector<V,IdType>::operate(operator_, output,  1, new Vector<V,IdType> *[1]{this});
	}

	/**
	 * output = this * vector2 or this (.) vector where (.) is product point.
	 */
	template <class V, class IdType>
	V PrimitiveVector<V, IdType>::multiply(PrimitiveVector &vector2, PrimitiveVector &output){
		//product point
		if(this->is_transpose() != vector2.is_transpose()){
				PrimitiveVectorProductPoint<V,IdType> operator_;
				return Vector<V,IdType>::operate(operator_, *this,  2, new Vector<V,IdType>*[2]{this, &vector2});
		}else{
				//multiplication element to element
				PrimitiveVectorBinarySum<V,IdType> operator_;
				Vector<V,IdType>::operate(operator_, output,  2, new Vector<V,IdType> *[2]{this, &vector2});
				V v = 0;
				return v;
		}
	}

	/**
	* this *= vector2 or product point.
	*/
	template <class V, class IdType>
	V PrimitiveVector<V, IdType>::multiply(PrimitiveVector &vector2){
		return multiply(vector2, *this);
	}


	/**
	 * output = this * vector2 or this (.) vector where (.) is product point.
	 */
	template <class V, class IdType>
	V PrimitiveVector<V, IdType>::pnorm(int p){
			PrimitiveVectorPNorm<V,IdType> operator_(p);
			V value = Vector<V,IdType>::operate(operator_, *this,  1, new Vector<V,IdType>*[1]{this});
			return pow(value, (double)1/p);
	}

	/**
	 * output = this * vector2 or this (.) vector where (.) is product point.
	 */
	template <class V, class IdType>
	void PrimitiveVector<V, IdType>::fill(V value){
			PrimitiveVectorFill<V,IdType> operator_(value);
			Vector<V,IdType>::operate(operator_, *this,  1, new Vector<V,IdType>*[1]{this});
	}

	/**
	 * output = this * vector2 or this (.) vector where (.) is product point.
	 */
	template <class V, class IdType>
	void PrimitiveVector<V, IdType>::fill_random(){
			PrimitiveVectorFillRandom<V,IdType> operator_;
			Vector<V,IdType>::operate(operator_, *this,  1, new Vector<V,IdType>*[1]{this});
	}

	template <class V, class IdType>
	void PrimitiveVector<V, IdType>::linear_combination(int n, V constants[], PrimitiveVector<V, IdType> *vectors[]){
		PrimitiveVectorBinarySum<V,IdType> soperator;
		PrimitiveVectorBinaryMul<V,IdType> moperator;

		if (n < 0){
				return;
		}

		if (n == 1){
				vectors[0]->multiply(constants[0], *this);
				return;
		}

		Vector<V,IdType> *vvectors [n];
		for(int i = 0; i < n; i++){
				vvectors[i] = vectors[i];
		}

		Vector<V,IdType>::operate(moperator, soperator, *this,n, constants, vvectors);

/*
		LinearOperator<V> loperator (constants, &soperator, &moperator);
		OperationListener * listener = &loperator;
		this->add_listener(listener);
		Vector<V,IdType>::operate(loperator, *this, n,  vvectors);
		this->remove_listener(listener);*/
	}

}
#endif /* MFLASH_CPP_CORE_PRIMITIVEVECTOR_HPP_ */
