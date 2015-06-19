/*
 * ArrayTest.cpp
 *
 *  Created on: Feb 28, 2015
 *      Author: hugo
 */


#include <cmath>
#include <ctime>
#include <iostream>

#include "../core2/malgorithm.hpp"
#include "../core2/primitivematrix.hpp"
#include "../core2/primitivevector.hpp"
#include "../log/easylogging++.h"

INITIALIZE_EASYLOGGINGPP


using namespace std;
using namespace mflash;

struct PairShort{
	short data1;
	short data2;
};
/*

template <class VSource = PairShort, class VDestination = PairShort, class E = EmptyField>
class SpMV2 : public MAlgorithm<VSource, VDestination,E>{
	public:
			inline void initialize(MatrixWorker<E> &worker, Element<PairShort> &out_element){
				out_element.value->data1 = 0;
				out_element.value->data2 = 1;
			}
			inline void gather(MatrixWorker<E> &worker, Element<PairShort> &in_element, Element<PairShort> &out_element, E &edge_data){

			}
			inline  void sum(Element<PairShort> &accumulator1, Element<PairShort> &accumulator2, Element<PairShort> &out_accumulator){}
			inline  void apply(MatrixWorker<E> &worker, Element<V> &out_element) {}
			inline  bool isInitialized(){
				return true;
			}
			inline  bool isApplied(){
				return false;
			}
};


template <class VSource, class VDestination, class E>
class SpMV : public MAlgorithm<VSource, VDestination,E>{
	public:
			inline void initialize(MatrixWorker<E> &worker, Element<V> &out_element){
				*(out_element.value) = 0;
			}
			inline void gather(MatrixWorker<E> &worker, Element<V> &in_element, Element<V> &out_element, E &edge_data){
				*(out_element.value) += *(in_element.value);
			}
			inline  void sum(Element<V> &accumulator1, Element<V> &accumulator2, Element<V> &out_accumulator){}
			inline  void apply(MatrixWorker<E> &worker, Element<V> &out_element) {}
			inline  bool isInitialized(){
				return true;
			}
			inline  bool isApplied(){
				return false;
			}
};
*/

class AuxMAlgorithm : public MAlgorithm<long, double, EmptyField>{
  int ** degree;
  inline void before_iteration(int iteration, MatrixWorker<EmptyField> &worker){
    //degree = (int**)worker.get_field_pointer(FieldType::SOURCE, "degree");
  }
  inline void initialize(MatrixWorker<EmptyField> &worker, Element<double> &out_element){
    *(out_element.value) = 0;
  }
  inline void gather(MatrixWorker<EmptyField> &worker, Element<long> &in_element, Element<double> &out_element, EmptyField &edge_data){
    *(out_element.value) += *(in_element.value /*+ **degree*/);
  }
  inline  void sum(Element<double> &accumulator1, Element<double> &accumulator2, Element<double> &out_accumulator){}
  inline  void apply(MatrixWorker<EmptyField> &worker, Element<double> &out_element) {}
  inline  bool is_initialized(){return true;}
  inline  bool is_applied(){return true;}
};

class PrintOperator : public ZeroOperator<int>{
	public:
		void apply(Element<int> &element){
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
	cout<< sizeof(EmptyField)<< endl;
	CustoOperator op;// = new CustoOperator;
	CustomOperator2 op2;
	CustomOperator3 op3;
	CustomOperator4 op4;
	PrintOperator prindst ;

	size = 1413511394;
	bsize = 67108864;
	int64 sum = 0;


	MappedStream stream ("/hugo/datasets/undirected-1GB-double/.G-FLASH/0_0.block");
	while(stream.has_remain()){
	    sum += stream.next_int();
	}
	stream.close_stream();
	cout<< "sum" <<sum <<endl;


	PrimitiveVector<long> pvector("/hugo/datasets/undirected-1GB-double/v1", size, bsize);
	PrimitiveVector<int> aux("/hugo/datasets/undirected-1GB-double/v2", size, bsize);
	PrimitiveVector<double> pout("/hugo/datasets/undirected-1GB-double/v3", size, bsize);
	//aux.operate(prindst);
	//pvector.fill(1);
	//aux.fill(2);


/*	PrimitiveVector<float> pvector2("/tmp/v42", size, bsize);
  PrimitiveVector<float> pout("/tmp/ou1t", size, bsize);

  pvector.fill(3);
  pvector.operate(prindst);
*/

	AuxMAlgorithm algorithm;

  Matrix<EmptyField> matrix ("/hugo/datasets/undirected-1GB-double/yahoo", size, false, bsize, Mode::UNSAFE);
  //matrix.add_field(FieldType::SOURCE, "degree", aux);
  matrix.operate<long, double>(algorithm, pvector, pout);
	return 0;
}




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
/*

  size = 1413511394;
  bsize = 67108864;
  //SpMV<float,float, EmptyField> spmv;

  PrimitiveVector<double> *vector = new PrimitiveVector<double>("/hugo/datasets/undirected-1GB-double/v1", size, bsize);
  PrimitiveVector<double> *out = new PrimitiveVector<double>("/hugo/datasets/undirected-1GB-double/v2", size, bsize);
//  vector->fill(1);
*/


  //vector->fill(1);
  //value = vector->operate(op, *vector, 0, new Vector<float>[0]{/**vector*/});
  //exit(0);
//  value = vector->operate(op, *vector, 0, new Vector<float>[0]{/**vector*/});
  //value = vector->operate(op3, *vector, 0, new Vector<float>[0]{/**vector*/});
  //cout << value << endl;


/*  time_t timer1;
  time_t timer2;
  time(&timer1);
  PrimitiveMatrix<EmptyField> matrix ("/hugo/datasets/undirected-1GB-double/yahoo", size, false, bsize, Mode::UNSAFE);*/
  //matrix.multiply(*vector, *out);

/*
  LanczosSO lanczos (matrix, 20, 6);
  lanczos.run();
*/


  //matrix.multiply(*vector, *out);
  //matrix.operate(spmv, *vector, *out);
/*  time(&timer2);
  time_t final = timer2 - timer1;
  cout<<"Time: "<< final << " seconds";
  time(&timer1);
  matrix.multiply(*vector, *out);
  //matrix.operate(spmv, *vector, *out);
  time(&timer2);
  final = timer2 - timer1;
  cout<<"Time: "<< final << " seconds";*/
/*



  Array<float> *array = new Array<float>(size);
  Array<float>::operate(op, *array);
  value = Array<float>::operate(op3, *array);
  cout << value << endl;
  value = Array<float>::operate(op4, *array, *array);
  cout << value << endl;

  Array<float>::operate(op2, *array, *array, *array);

  cout<< *(array->get_element(size-1)) << endl;*/
