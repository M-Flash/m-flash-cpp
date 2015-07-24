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

int main(){

	PrimitiveVector<long> pvector("/hugo/datasets/undirected-1GB-double/v1", size, bsize);

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
