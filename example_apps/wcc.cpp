// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "../src/mflash_basic_includes.hpp"
#include "../src/algorithm/wcc.hpp"

using namespace std;
using namespace mflash;

int main(int argc, char ** argv){
	mflash_init(argc, argv);
	Matrix<EmptyField, int> matrix ("/run/media/hugo/data/datasets/twitter");
	PrimitiveVector<int, int> pvector("/run/media/hugo/data/datasets/wcc");
	WCC::run(matrix, pvector, true);
	return 0;
}
