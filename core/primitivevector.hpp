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

#include "operator.hpp"
#include "type.hpp"
#include "vector.hpp"

using namespace std;

namespace mflash{

	template <class V>
	class PrimitiveVectorBinarySum : public BinaryOperator<V>{
		public:
			inline void apply(Element<V> &left, Element<V> &right, Element<V> &out){
				(*out.value) = (*left.value) + (*right.value);
			}
	};

	template <class V>
		class PrimitiveVectorBinaryMul : public BinaryOperator<V>{
			public:
				inline void apply(Element<V> &left, Element<V> &right, Element<V> &out){
					(*out.value) = (*left.value) * (*right.value);
				}
		};

	template <class V>
		class PrimitiveVectorProductPoint: public BinaryReducer<V>{
			public:
				inline void initialize(V &out) {
					out = 0;
				}
				inline void sum(V &left, V &right, V &out){
					out = right + left;
				}
				inline void apply(Element<V> &left, Element<V> &right, Element<V> &out){
					(*out.value) = (*left.value) * (*right.value);
				}
		};

		template <class V>
		class PrimitiveVectorPNorm: public UnaryReducer<V>{
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
				inline void apply(Element<V> &left, Element<V> &out){
					if(even){
							(*out.value) = pow((*left.value), p);
					}else{
							(*out.value) = pow(abs((*left.value)), p);
					}
				}
		};

		template <class V>
		class PrimitiveVectorFillRandom: public ZeroOperator<V>{
			public:
				PrimitiveVectorFillRandom(){srand (time(NULL));}
				inline void apply(Element<V> &out){
					(*out.value) = (V)rand()/ RAND_MAX;
				}
		};

		template <class V>
		class PrimitiveVectorFill: public ZeroOperator<V>{
				V value;
			public:
				PrimitiveVectorFill(V value){this->value = value;}
				inline void apply(Element<V> &out){
					(*out.value) = value;
				}
		};

		template <class V>
		class PrimitiveVectorMultiplyConstant: public UnaryOperator<V>{
				V value;
			public:
				PrimitiveVectorMultiplyConstant(V value){this->value = value;}
				inline void apply(Element<V> &element, Element<V> &out){
					(*out.value) = *(element.value) * value;
				}
		};

		template <class V>
		class PrimitiveVectorAddConstant: public UnaryOperator<V>{
				V value;
			public:
				PrimitiveVectorAddConstant(V value){this->value = value;}
				inline void apply(Element<V> &element, Element<V> &out){
					(*out.value) = *(element.value) + value;
				}
		};



	template <class V>
	class PrimitiveVector : public Vector<V>{
		bool transpose_;

		public:
			PrimitiveVector(string file, int64 size, int64 elements_by_block = 0, bool transpose = false) :
				Vector<V>::Vector(file, size, elements_by_block){
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
			PrimitiveVector<V> transpose(){
				PrimitiveVector<V> v(this->file, this->size, this->elements_by_block, !transpose_);
				return (v);
			};

			void linear_combination(int n, V constants[], PrimitiveVector<V> *vectors[]);
			//static V operate(Operator<V> &operator_, PrimitiveVector<V> &output,  int n, PrimitiveVector<V> vectors[]);
	};

	/**
	 * output = this + vector2
	 */
	template <class V>
	void PrimitiveVector<V>::add(PrimitiveVector &vector2, PrimitiveVector &output){
		PrimitiveVectorBinarySum<V> operator_;
		Vector<V>::operate(operator_, output,  2, new Vector<V>[2]{*this, vector2});
	}

	/**
	 * output += vector2
	 */
	template <class V>
	void PrimitiveVector<V>::add(PrimitiveVector &vector2){
		add(vector2, *this);
	}

	/**
	 * this += value
	 */
	template <class V>
	void PrimitiveVector<V>::add(V value){
			PrimitiveVectorAddConstant<V> operator_(value);
			Vector<V>::operate(operator_, *this,  1, new Vector<V>[1]{*this});
	}

	/**
	 * this += value
	 */
	template <class V>
	void PrimitiveVector<V>::add(V value, PrimitiveVector<V> &out){
			PrimitiveVectorAddConstant<V> operator_(value);
			Vector<V>::operate(operator_, out,  1, new Vector<V>[1]{*this});
	}

	/**
	 * this *= value
	 */
	template <class V>
	void PrimitiveVector<V>::multiply(V value){
			PrimitiveVectorMultiplyConstant<V> operator_(value);
			Vector<V>::operate(operator_, *this,  1, new Vector<V> *[1]{this});
	}

	/**
	 * this *= value
	 */
	template <class V>
	void PrimitiveVector<V>::multiply(V value, PrimitiveVector &output){
			PrimitiveVectorMultiplyConstant<V> operator_(value);
			Vector<V>::operate(operator_, output,  1, new Vector<V> *[1]{this});
	}

	/**
	 * output = this * vector2 or this (.) vector where (.) is product point.
	 */
	template <class V>
	V PrimitiveVector<V>::multiply(PrimitiveVector &vector2, PrimitiveVector &output){
		//product point
		if(this->is_transpose() != vector2.is_transpose()){
				PrimitiveVectorProductPoint<V> operator_;
				return Vector<V>::operate(operator_, *this,  2, new Vector<V>*[2]{this, &vector2});
		}else{
				//multiplication element to element
				PrimitiveVectorBinarySum<V> operator_;
				Vector<V>::operate(operator_, output,  2, new Vector<V> *[2]{this, &vector2});
				V v = 0;
				return v;
		}
	}

	/**
	* this *= vector2 or product point.
	*/
	template <class V>
	V PrimitiveVector<V>::multiply(PrimitiveVector &vector2){
		return multiply(vector2, *this);
	}


	/**
	 * output = this * vector2 or this (.) vector where (.) is product point.
	 */
	template <class V>
	V PrimitiveVector<V>::pnorm(int p){
			PrimitiveVectorPNorm<V> operator_(p);
			V value = Vector<V>::operate(operator_, *this,  1, new Vector<V>*[1]{this});
			return pow(value, (double)1/p);
	}

	/**
	 * output = this * vector2 or this (.) vector where (.) is product point.
	 */
	template <class V>
	void PrimitiveVector<V>::fill(V value){
			PrimitiveVectorFill<V> operator_(value);
			Vector<V>::operate(operator_, *this,  1, new Vector<V>*[1]{this});
	}

	/**
	 * output = this * vector2 or this (.) vector where (.) is product point.
	 */
	template <class V>
	void PrimitiveVector<V>::fill_random(){
			PrimitiveVectorFillRandom<V> operator_;
			Vector<V>::operate(operator_, *this,  1, new Vector<V>*[1]{this});
	}

	template <class V>
	void PrimitiveVector<V>::linear_combination(int n, V constants[], PrimitiveVector<V> *vectors[]){
		PrimitiveVectorBinarySum<V> soperator;
		PrimitiveVectorBinaryMul<V> moperator;

		if (n < 0){
				return;
		}

		if (n == 1){
				vectors[0]->multiply(constants[0], *this);
				return;
		}

		Vector<V> *vvectors [n];
		for(int i = 0; i < n; i++){
				vvectors[i] = vectors[i];
		}

		Vector<V>::operate(moperator, soperator, *this,n, constants, vvectors);

/*
		LinearOperator<V> loperator (constants, &soperator, &moperator);
		OperationListener * listener = &loperator;
		this->add_listener(listener);
		Vector<V>::operate(loperator, *this, n,  vvectors);
		this->remove_listener(listener);*/
	}

}
#endif /* MFLASH_CPP_CORE_PRIMITIVEVECTOR_HPP_ */
