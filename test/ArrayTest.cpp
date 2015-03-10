/*
 * ArrayTest.cpp
 *
 *  Created on: Feb 28, 2015
 *      Author: hugo
 */


#include <cmath>
#include <ctime>
#include <iostream>

#include "../core/malgorithm.hpp"
#include "../core/matrixworker.hpp"
#include "../core/operator.hpp"
#include "../core/primitivematrix.hpp"
#include "../core/mapped_stream.hpp"
#include "../core/primitivevector.hpp"
#include "../core/type.hpp"
#include "../core/vector.hpp"
#include "../algorithm/lanczosSO.hpp"
#include "../log/easylogging++.h"

using namespace std;
using namespace mflash;

INITIALIZE_EASYLOGGINGPP

struct PairShort{
	short data1;
	short data2;
};

template <class V = PairShort, class E = EmptyType>
class SpMV2 : public MAlgorithm<V,E>{
	public:
			inline void initialize(MatrixWorker<PairShort, E> &worker, Element<PairShort> &out_element){
				out_element.value->data1 = 0;
				out_element.value->data2 = 1;
			}
			inline void gather(MatrixWorker<PairShort, E> &worker, Element<PairShort> &in_element, Element<PairShort> &out_element, E &edge_data){

			}
			inline  void sum(Element<PairShort> &accumulator1, Element<PairShort> &accumulator2, Element<PairShort> &out_accumulator){}
			inline  void apply(MatrixWorker<PairShort, E> &worker, Element<V> &out_element) {}
			inline  bool isInitialized(){
				return true;
			}
			inline  bool isApplied(){
				return false;
			}
};


template <class V, class E>
class SpMV : public MAlgorithm<V,E>{
	public:
			inline void initialize(MatrixWorker<V, E> &worker, Element<V> &out_element){
				*(out_element.value) = 0;
			}
			inline void gather(MatrixWorker<V, E> &worker, Element<V> &in_element, Element<V> &out_element, E &edge_data){
				*(out_element.value) += *(in_element.value);
			}
			inline  void sum(Element<V> &accumulator1, Element<V> &accumulator2, Element<V> &out_accumulator){}
			inline  void apply(MatrixWorker<V, E> &worker, Element<V> &out_element) {}
			inline  bool isInitialized(){
				return true;
			}
			inline  bool isApplied(){
				return false;
			}
};

class PrintOperator : public ZeroOperator<float>{
	public:
		void apply(Element<float> &element){
			cout<<"["<<element.id<<"]="<<*(element.value)<<endl;
		}
};

class CustoOperator : public ZeroOperator<float>{
	public:
		void apply(Element<float> &element){
			*(element.value) = 1;
		}
};

class CustomOperator2 : public BinaryOperator<float>{
	public:
		void apply(Element<float> &v1, Element<float> &v2, Element<float> &out){
			*(out.value) = *(v1.value) + 2 * *(v2.value);
		}
};

class CustomOperator3 : public UnaryReducer<float>{
	public:
		void initialize(float &v1){
			v1 = 0;
		}

		void sum(float &v1, float &v2 , float &out){
			out = v1 + v2;
		}

		void apply(Element<float> &v1, Element<float> &out){
			*(out.value) = *(v1.value) * 2.001;
		}
};

class CustomOperator4 : public BinaryReducer<float>{
	public:
		void initialize(float &v1){
			v1 = 0;
		}

		void sum(float &v1, float &v2 , float &out){
			out = v1 + v2;
		}

		void apply(Element<float> &v1, Element<float> &v2, Element<float> &out){
			*(out.value) = *(v1.value) * 2;
		}
};


int main(){
	int64 size = 97;
	int64 bsize = 10;
	//float value;
	cout<< sizeof(EmptyType)<< endl;
	CustoOperator op;// = new CustoOperator;
	CustomOperator2 op2;
	CustomOperator3 op3;
	CustomOperator4 op4;
	PrintOperator prindst ;
//cout<< sizeof(size_t)<< endl;
	//MappedStream stw ("/hugo/datasets/undirected-1GB-double/.G-FLASH/0_0.block", 2927861104, 19558831);
/*

	PrimitiveVector<float> pvector("/tmp/v31", size, bsize);
	PrimitiveVector<float> pvector2("/tmp/v42", size, bsize);
	PrimitiveVector<float> pout("/tmp/ou1t", size, bsize);

	pvector.fill(3);
	pvector.operate(prindst);
	//Vector<float>::operate(prindst, pvector,  0, new Vector<float>*[0]{});

	pvector2.fill(2);
	pvector2.operate(prindst);
	//Vector<float>::operate(prindst, pvector2,  0, new Vector<float>*[0]{});

	//PrimitiveVector<float> *pvectors ;//new PrimitiveVector<float>[2]{pvector, pvector2};

	float constants [] = {5,6};
	pout.linear_combination(2, constants, new PrimitiveVector<float>*[3]{&pvector, &pvector2});
	//Vector<float>::operate(prindst, pout,  0, new Vector<float>*[0]{});
	pout.operate(prindst);

	cout<< "Linear combination = "<< pout.pnorm(1) << endl;

	pvector.fill_random();
	cout<< pvector.pnorm(2) << endl;
	PrimitiveVector<float> t = pvector.transpose();
	cout<< pvector.multiply(t) << endl;
	cout<< pow(pvector.pnorm(2),2) << endl;

	pvector.multiply((float)(1/pvector.pnorm(2)));
	cout<< pvector.pnorm(2) << endl;
*/

	size = 1413511394;
	bsize = 67108864;
	SpMV<float, EmptyType> spmv;

	PrimitiveVector<double> *vector = new PrimitiveVector<double>("/hugo/datasets/undirected-1GB-double/v1", size, bsize);
	PrimitiveVector<double> *out = new PrimitiveVector<double>("/hugo/datasets/undirected-1GB-double/v2", size, bsize);
//	vector->fill(1);


	//vector->fill(1);
	//value = vector->operate(op, *vector, 0, new Vector<float>[0]{/**vector*/});
	//exit(0);
//	value = vector->operate(op, *vector, 0, new Vector<float>[0]{/**vector*/});
	//value = vector->operate(op3, *vector, 0, new Vector<float>[0]{/**vector*/});
	//cout << value << endl;


	time_t timer1;
	time_t timer2;
	time(&timer1);
	PrimitiveMatrix< double, EmptyType> matrix ("/hugo/datasets/undirected-1GB-double/yahoo", size, false, bsize, Mode::UNSAFE);
	//matrix.multiply(*vector, *out);

	LanczosSO lanczos (matrix, 20, 6);
	lanczos.run();


	//matrix.multiply(*vector, *out);
	//matrix.operate(spmv, *vector, *out);
	time(&timer2);
	time_t final = timer2 - timer1;
	cout<<"Time: "<< final << " seconds";
	time(&timer1);
	matrix.multiply(*vector, *out);
	//matrix.operate(spmv, *vector, *out);
	time(&timer2);
	final = timer2 - timer1;
	cout<<"Time: "<< final << " seconds";
/*



	Array<float> *array = new Array<float>(size);
	Array<float>::operate(op, *array);
	value = Array<float>::operate(op3, *array);
	cout << value << endl;
	value = Array<float>::operate(op4, *array, *array);
	cout << value << endl;

	Array<float>::operate(op2, *array, *array, *array);

	cout<< *(array->get_element(size-1)) << endl;*/
}

