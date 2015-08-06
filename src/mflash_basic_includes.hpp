/*
 * mflash_basic_includes.hpp
 *
 *  Created on: Jul 8, 2015
 *      Author: hugo
 */

#ifndef MFLASH_BASIC_INCLUDES_HPP_
#define MFLASH_BASIC_INCLUDES_HPP_

#ifdef DEBUG_BUILD
#  define DEBUG(x) fprintf(stderr, x)
#else
#  define DEBUG(x) do {} while (0)
#endif

#include <assert.h>

#include "util/easylogging++.h"
#include "core/type.hpp"
#include "util/configfile.hpp"
#include "util/cmdopts.hpp"
#include "core/util.hpp"
#include "core/conversions.hpp"
#include "core/mapped_stream.hpp"
#include "core/splitter_buffer_block_counting.hpp"
#include "core/vector.hpp"
#include "core/primitivevector.hpp"
#include "core/matrix.hpp"
#include "core/primitivematrix.hpp"


INITIALIZE_EASYLOGGINGPP



#endif /* MFLASH_BASIC_INCLUDES_HPP_ */
