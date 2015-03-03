/*
 * pagerank.cpp
 *
 *  Created on: Mar 3, 2015
 *      Author: hugo
 */


#include "../core/malgorithm.hpp"
#include "../core/type.hpp"

//using namespace mflash;

namespace mflash{


	#define RESISTANCE 0.85
	#define N 1222

	template <class V, class E>
	class PageRank : public MAlgorithm<V, E>{
		public:
			void initialize(MatrixWorker<V, E> &worker, Element<V> &out_element){
				*(out_element.value) = 0;
			}
			void gather(MatrixWorker<V, E> &worker, Element<V> &in_element, Element<V> &out_element, E &edge_data){
				*(out_element.value) += *(in_element.value);
			}
			void sum(Element<V> &accumulator1, Element<V> &accumulator2, Element<V> &out_accumulator){
				*(out_accumulator.value) = *(accumulator1.value) + *(accumulator2.value);
			}
			void apply(MatrixWorker<V, E> &worker, Element<V> &out_element){
				*(out_element.value) += (1 - RESISTANCE)/N;
			}
			bool isInitialized(){return true;}
			bool isApplied(){return true;}
	};
}
