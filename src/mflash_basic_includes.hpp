// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
