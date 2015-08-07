// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CORE_EIGENWRAPPER_HPP_
#define CORE_EIGENWRAPPER_HPP_

#include <eigen3/Eigen/Dense>

typedef Eigen::MatrixXd mat;

inline void mat2array(mat &m, int64 n, double * array){
	memcpy (array, m.data(), n * sizeof(double));
}

inline void array2mat(int64 n, double * array, mat &m){
	if(n<=0 || n>m.cols() * m.rows()) n = m.cols() * m.rows();
	memcpy (m.data(), array, n * sizeof(double));
}

inline void array2mat(int64 n, double* array, mat *m){
  array2mat(n, array, *m);
}

inline void swap_cols(mat &m , int64 col_ids[], int n_cols=0){
	if (n_cols == 0) n_cols = m.cols();
	mat tmp (m);

	for ( int64 i = 0; i < n_cols; i++){
			tmp.col(col_ids[i]) = m.col(i);
	}
	m = tmp;
}

inline void swap_cols(mat *m , int64 *col_ids, int n_cols=0){
	swap_cols(*m, col_ids, n_cols);
}

#endif /* CORE_EIGENWRAPPER_HPP_ */
