/*
 * edgeprocessor.hpp
 *
 *  Created on: Mar 13, 2015
 *      Author: hugo
 */

#ifndef CORE2_EDGEPROCESSOR_HPP_
#define CORE2_EDGEPROCESSOR_HPP_


namespace mflash{
  class AbstractProcessor{
      public:
        virtual bool next_edge (int64 in_vertex_id, int64 out_vertex_id, void* edge_data) = 0;
    };

    template <class VSource, class VDestination, class E>
    class EdgeProcessor : public AbstractProcessor{
      Element<VSource> element_source;
      Element<VDestination> element_destination_accumulator;

      Array<VSource> *array_source;
      Array<VDestination> *array_destination;

      MatrixWorker<E> *worker;
      MAlgorithm<VSource, VDestination, E> *algorithm;
      int64 field_count;
      int64 source_field_count;
      int64 destination_field_count;
      int64 i;

      public:
        EdgeProcessor(MatrixWorker<E> &worker, MAlgorithm<VSource, VDestination, E> &algorithm){
          this->worker = &worker;
          this->algorithm = &algorithm;
          array_source = dynamic_cast<Array<VSource>* >(worker.source_pointer);
          array_destination = dynamic_cast<Array<VDestination>* >(worker.destination_pointer);
          this->field_count = worker.get_field_count();
          this->source_field_count = worker.source_map.size();
          this->destination_field_count = worker.destination_map.size();
          i = 0;
        }

        inline bool next_edge (int64 in_vertex_id, int64 out_vertex_id, void* edge_data){
          E *edge = (E*) edge_data;
          element_source.id = in_vertex_id;
          element_destination_accumulator.id = out_vertex_id;

          if(array_source)
            element_source.value = array_source->get_element(in_vertex_id);
          if(array_destination)
            element_destination_accumulator.value = array_destination->get_element(out_vertex_id);

          void ** ptr;
          for( i = 0 ; i < source_field_count; i++){
              ptr = (void**)worker->value_pointers[i];
              *ptr = worker->array_pointers[i]->get_element(in_vertex_id);
          }

          for(; i < field_count; i++){
              ptr = (void**)worker->value_pointers[i];
              *ptr = worker->array_pointers[i]->get_element(out_vertex_id);
          }

          algorithm->gather(*(worker), element_source, element_destination_accumulator, *edge);

          return false;
        }

        bool is_transpose(){
          return worker->get_matrix().is_transpose();
        }

    };

}


#endif /* CORE2_EDGEPROCESSOR_HPP_ */
