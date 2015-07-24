/*
 * eigenwrapper.hpp
 *
 *  Created on: Mar 4, 2015
 *      Author: hugo
 */

#ifndef CORE_EIGENWRAPPER_HPP_
#define CORE_EIGENWRAPPER_HPP_

#include "../../Eigen/Dense"

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
