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
	template <class V>
	class Element{
		public:
			int64 id;
			V* value;

			void set_value(V value){
				*(this->value) = value;
			}

	};

	class EmptyType{};


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
}
#endif /* MFLASH_CPP_CORE_TYPE_HPP_ */
