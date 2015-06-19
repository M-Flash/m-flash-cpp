/*
 * type.hpp
 *
 *  Created on: Feb 27, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_TYPE_HPP_
#define MFLASH_CPP_CORE_TYPE_HPP_

typedef long long int64;

namespace mflash{

/*  template <class V>
	class AbstractElement{
    public:
      int64 id;
      V* value;

      void set_value(V value){
        *(this->value) = value;
      }

      V& get_value(){
        return value;
      }
  };*/

  template <class V>
	class Element{
		public:
			int64 id;
			V* value;

			void set_value(V value){
				*(this->value) = value;
			}
	};

	class EmptyField{};


	enum ElementIdSize{
		SIMPLE,
		DOUBLE,
		//ATOMIC
	};

	enum Mode{
		UNSAFE,
		VECTOR_REPLICATION,
		//ATOMIC
	};

  enum FieldType{
    SOURCE,
    DESTINATION
  };


	enum BlockType{
	  M_FLASH,
	  X_STREAM
	};

  struct BlockProperties{
    BlockType type;
    int64 offset;
    int64 size;


    BlockProperties(BlockType type, int64 offset, int64 size){
      this->type = type;
      this->offset = offset;
      this->size = size;
    }
  };
}
#endif /* MFLASH_CPP_CORE_TYPE_HPP_ */
