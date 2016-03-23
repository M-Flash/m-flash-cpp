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
	/* Parameters */
    	std::string filename = 	get_option_string("file"); // Base filename
	int iterative  = 	get_option_int("iterative", 0);
	int niters = 		get_option_int("niters", -1);

	niters = (niters == 0? 1: niters);

    	std::string wcc = get_parent_directory(filename) + "wcc";

	Matrix<EmptyField, int32> matrix (filename);
	matrix = matrix.transpose();
	PrimitiveVector<int32, int32> pvector(wcc);
	WCC::run(matrix, pvector, iterative != 0, niters);
	return 0;
}
