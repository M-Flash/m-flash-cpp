// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include "../src/mflash_basic_includes.hpp"
#include "../src/algorithm/pagerank.hpp"

using namespace std;
using namespace mflash;

int main(int argc, char ** argv){
    mflash_init(argc, argv);

    /* Parameters */
    std::string filename    = get_option_string("file"); // Base filename
    int niters              = get_option_int("niters", 4);

    std::string pg_ranks 	= get_parent_directory(filename) + "pg";


    Matrix<EmptyField, int32> matrix (filename);
    PrimitiveVector<float, int32> pvector(pg_ranks);
    PageRank::run(matrix, pvector, niters);
    return 0;
}
