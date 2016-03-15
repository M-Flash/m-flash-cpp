// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CORE_EdgeSplitterManager_HPP_
#define CORE_EdgeSplitterManager_HPP_

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "../util/easylogging++.h"
#include "type.hpp"
#include "util.hpp"

namespace mflash{

template <class IdType>
class EdgeSplitterManager{
  public:
	virtual IdType getPartitionId(IdType in_id, IdType out_id);
	virtual IdType countEdge(IdType in_id, IdType out_id);
	virtual std::string getPartitionFile(IdType id);
	virtual std::vector<int64>& getPartitionCounters();
	virtual IdType getPartitions();
	virtual IdType getIdsByPartition();
	virtual bool isInSplit();

	virtual ~EdgeSplitterManager();
};

}
#endif
