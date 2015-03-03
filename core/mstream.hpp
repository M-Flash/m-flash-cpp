/*
 * stream.hpp
 *
 *  Created on: Feb 27, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MSTREAM_HPP_
#define MFLASH_CPP_CORE_MSTREAM_HPP_

#include "type.hpp"

enum StreamMode{
	READ,
	READ_WRITE,
	READ_WRITE_APPEND
};

class MStream{
	int64 buffer_size;
	int64 size;
	int64 limit;

	StreamMode mode;

	public:

	int nextInt();
	int64 nextInt64();
	float nextFloat();
	double nextDouble();

};

#endif /* MFLASH_CPP_CORE_MSTREAM_HPP_ */
