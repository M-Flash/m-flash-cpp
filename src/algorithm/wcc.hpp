// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef ALGORITHM_WCC_HPP_
#define ALGORITHM_WCC_HPP_

#include "../core/malgorithm.hpp"
#include "../core/type.hpp"
#include "../core/primitivevector.hpp"


namespace mflash{

  template <class V, class E, class IdType >
    class PrimitiveUnionFindOperator{
    public:
        V*counts;
        V *sets;
        //int64 counter =0;

        /* Find operator of Union-Find with path compression */
        inline V find(V x) {
            while (sets[x] != x) {
                x = sets[x] = sets[sets[x]];
            }
            return sets[x];
        }


        inline void before_iteration(Matrix<E, IdType> &matrix) {
          int64 vertex_count = matrix.size();
          counts = new V[vertex_count];
          sets = new V[vertex_count];

          for(int64 i = 0; i < vertex_count; i++){
              counts[i] = 1;
              sets[i] = i;
          }
        }

        inline void after_iteration(MatrixWorker<E, IdType> &worker){
         int64 vertex_count = worker.get_in_array()->size();
         for(int64 i = 0; i < vertex_count; i++){
             sets[i] = find(i);
         }
        }
        inline void run(Matrix<E, IdType> &matrix){
          before_iteration(matrix);
          MappedStream stream(get_block_file(matrix.get_file(),0,0));
          char * ptr = stream.current_ptr;
          char * last_ptr = stream.last_ptr;
          while(ptr < last_ptr){
              int64 setSrc = find(*( (int*)ptr));
              int64 setDst = find(*( (int*)ptr+4));

              // If in same component, nothing to do, otherwise, Unite
              if (setSrc != setDst) {
                  if (counts[setSrc] > counts[setDst]) {
                      // A is bigger set, merge with A
                      sets[setDst] = setSrc;
                      counts[setSrc] += counts[setDst];
                  } else {
                      // or vice versa
                      sets[setSrc] = setDst;
                      counts[setDst] += counts[setSrc];
                  }
              }
              ptr+=8;
          }
          stream.close_stream();
        }
    };

  template <class V, class E, class IdType>
  class WCCAlgorithmUnionFindOperator {//: public MAlgorithm<V, V, E, IdType>{
  public:
      V*counts;
      V *sets;
      //int64 counter =0;

      /* Find operator of Union-Find with path compression */
      inline V find(V x) {
          while (sets[x] != x) {
              x = sets[x] = sets[sets[x]];
          }
          return sets[x];
      }
      inline void before_iteration(int iteration, MatrixWorker<E, IdType>  &worker) {
        counts = (V*)worker.get_source_array()->address();
        sets = (V*)worker.get_destination_array()->address();

        int64 vertex_count = worker.get_matrix().size();
        for(int64 i = 0; i < vertex_count; i++){
            counts[i] = 1;
            sets[i] = i;
        }
      }
      inline void initialize(MatrixWorker<E, IdType>  &worker,Element<V, IdType> &destination){}
      inline void gather(MatrixWorker<E, IdType> &worker, Element<V, IdType> &in_element, Element<V, IdType> &out_element, E &edge_data){
        int64 setDst = find(out_element.id);
        int64 setSrc = find(in_element.id);
        // If in same component, nothing to do, otherwise, Unite
        if (setSrc != setDst) {
            if (counts[setSrc] > counts[setDst]) {
                // A is bigger set, merge with A
                sets[setDst] = setSrc;
                counts[setSrc] += counts[setDst];
            } else {
                // or vice versa
                sets[setSrc] = setDst;
                counts[setDst] += counts[setSrc];
            }
        }
      }
      inline void process(Element<V, IdType> &accumulator1, Element<V, IdType> &accumulator2, Element<V, IdType> &out_accumulator){}
      inline void apply(MatrixWorker<E, IdType> &worker, Element<V, IdType> &out_element){}
      inline bool is_initialized(){return false;}
      inline bool is_source_loaded(){return false;}
      inline bool is_destination_loaded(){return false;}
      inline bool is_destination_stored(){return true;}
      inline bool is_applied(){return false;}

      inline void after_iteration(int iteration, MatrixWorker<E, IdType>  &worker) {
        int64 vertex_count = worker.get_matrix().size();
        for(int64 i = 0; i < vertex_count; i++){
            sets[i] = find(i);
        }
      }

  };

  template <class V, class E, class IdType>
    class WCCAlgorithmIterativeOperator{//: public MAlgorithm<V, V, E, IdType>{
      public:
        bool state = false;
        bool initialize_ = true;
        int64 counter =0;

        inline void initialize(MatrixWorker<E, IdType> &worker, Element<V, IdType> &out_element){
          *(out_element.value) = out_element.id;
        }
        inline void gather(MatrixWorker<E, IdType>  &worker, Element<V, IdType> &source,Element<V, IdType> &destination, E &edge_data){
          //counter++;
          if(*(destination.value) > *(source.value)){
              counter ++;
              state = true;
              *(destination.value) = *(source.value);
          }
        }
        inline void process(Element<V, IdType> &accumulator1, Element<V, IdType> &accumulator2, Element<V, IdType> &out_accumulator){
          *(out_accumulator.value) = min(*(accumulator1.value), *(accumulator2.value));
        }
        inline void apply(MatrixWorker<E, IdType>  &worker,Element<V, IdType> &out_element){}
        inline bool is_initialized(){return initialize_;}
        inline bool is_applied(){return false;}

        inline void before_iteration(int iteration, MatrixWorker<E, IdType>  &worker) {
          counter = 0;
        }
        inline void after_iteration(int iteration, MatrixWorker<E, IdType>  &worker) {
          cout<< "CHANGES = "<<counter << endl;
        }

		inline bool is_destination_loaded(){return true;}
		inline bool is_source_loaded(){return true;}
		inline bool is_destination_stored(){return true;}
    };

  class WCC{
    public:
    template <class V, class E, class IdType>
    inline static void run(Matrix<E, IdType> &matrix, PrimitiveVector<V, IdType> &vector, bool iterative=false){
       int iteration = 0;
 	   set_conf("elementsize", to_string(2 * sizeof(V)));
       matrix.load();
       LOG(INFO) << "WEAK CONNECTED COMPONENT STARTED"<<endl;
       if(matrix.get_elements_by_block() <= matrix.size() || iterative){
           Matrix<E, IdType> m = matrix.transpose();
           LOG(INFO) << "ITERATIVE WEAK CONNECTED COMPONENT STARTED"<<endl;
           WCCAlgorithmIterativeOperator<V, E, IdType> algorithm;
           algorithm.initialize_ = true;
           do{
               algorithm.state = false;
               m.operate(algorithm, vector, vector);
               LOG(INFO) << "ITERATION "<< iteration++ <<endl;
               m = m.transpose();
               algorithm.initialize_ = false;
           }while(algorithm.state);
           LOG(INFO) << "WEAK CONNECTED COMPONENT FINISHED"<<endl;
       }else{
           LOG(INFO) << "UNION-FIND WEAK CONNECTED COMPONENT STARTED"<<endl;
           /*PrimitiveUnionFindOperator<V, E> algorithm;
           algorithm.run(matrix);*/
           WCCAlgorithmUnionFindOperator<V, E, IdType> algorithm;
           matrix.operate(algorithm, vector, vector/*, false,false*/);
           LOG(INFO) << "UNION-FIND WEAK CONNECTED COMPONENT FINISHED"<<endl;
       }
     }
  };
}


#endif /* ALGORITHM_WCC_HPP_ */
