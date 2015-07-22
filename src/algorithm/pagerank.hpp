/*
 * pagerank.cpp
 *
 *  Created on: Mar 3, 2015
 *      Author: hugo
 */


#include "../core/malgorithm.hpp"
#include "../core/type.hpp"
#include "../core/primitivevector.hpp"

//using namespace mflash;

namespace mflash{


#define RESISTANCE 0.85

template <class V, class E, class IdType>
  class DegreeOperator : public MAlgorithm<V, V, E, IdType>{
    public:
      inline void initialize(MatrixWorker<E, IdType> &worker, Element<V,IdType> &out_element){
        *(out_element.value) = 0;
      }
      inline void gather(MatrixWorker<E, IdType> &worker, Element<V,IdType> &in_element, Element<V, IdType> &out_element, E &edge_data){
        *(out_element.value) += 1;
      }
      inline void process(Element<V, IdType> &accumulator1, Element<V,IdType> &accumulator2, Element<V, IdType> &out_accumulator){
        *(out_accumulator.value) = *(accumulator1.value) + *(accumulator2.value);
      }
      inline void apply(MatrixWorker<E, IdType> &worker, Element<V,IdType> &out_element){
        if(*(out_element.value) != 0){
            *(out_element.value)= (V)1/ *(out_element.value) ;
        }
      }
      inline bool is_initialized(){return true;}
      inline bool is_applied(){return true;}
  };


	template <class V, class E, class IdType>
	class PageRankOperator : public MAlgorithm<V, V, E, IdType>{
		public:
		IdType n;
	    inline void initialize(MatrixWorker<E, IdType> &worker, Element<V, IdType> &out_element){
			*(out_element.value) = 0;
		}
		inline void gather(MatrixWorker<E, IdType> &worker, Element<V, IdType> &in_element, Element<V, IdType> &out_element, E &edge_data){
			*(out_element.value) += *(in_element.value);
		}
		inline void process(Element<V, IdType> &accumulator1, Element<V, IdType> &accumulator2, Element<V, IdType> &out_accumulator){
			*(out_accumulator.value) = *(accumulator1.value) + *(accumulator2.value);
		}
		inline void apply(MatrixWorker<E, IdType> &worker, Element<V, IdType> &out_element){
			*(out_element.value) += RESISTANCE * *(out_element.value) + ( (V)1 - RESISTANCE)/n;
		}
		inline bool is_initialized(){return true;}
		inline bool is_applied(){return true;}

	};


	class PageRank {
	public:
	  template <class V, class E, class IdType>
	  static inline void run(Matrix<E, IdType> &matrix, PrimitiveVector<V, IdType> &destination, int iterations){

	  set_conf("elementsize", to_string(2 * sizeof(V)));
	  matrix.load();

	  string path = get_parent_directory(matrix.get_file()) + FILE_SEPARATOR;
      string degree_file = get_out_degree_file(matrix.get_file());
      string next_file = path + "next";

      PrimitiveVector<V, IdType> degree_vec(degree_file);
      PrimitiveVector<V, IdType> *current_vec = &destination;
      PrimitiveVector<V, IdType> *next_vec = new PrimitiveVector<V, IdType> (next_file);
      PrimitiveVector<V, IdType>* tmp;

      DegreeOperator<V,E, IdType> degree_operator;
      PageRankOperator<V,E, IdType> pg_operator;

      //creating degree file
      if(!exist_file(degree_file)){
          matrix.operate(degree_operator, degree_vec, degree_vec);
      }
      Matrix<E, IdType> m = m.transpose();
      LOG (INFO) << "PR  = 1/N";



      destination.fill((V)1/matrix.size());
      for (int iteration = 0; iteration < iterations; iteration++){
          LOG (INFO) << endl<< "========== ITERATION "<< iteration <<" ==========";
          LOG (INFO) << "PR = PR ./ OUTDEGREE";
          destination.multiply(degree_vec);
          LOG (INFO) << "NEXT_PR = 0.15/N + 0.85 * M*PR ";
/*
          Vector<V, IdType> *current_vec_v = current_vec;
          Vector<V, IdType> *next_vec_v = next_vec;
*/

          m.operate(pg_operator, *current_vec, *next_vec);//, true, true, iteration%2!=0);
          tmp = current_vec;
          current_vec = next_vec;
          next_vec = tmp;
      }
    }

	};

}
