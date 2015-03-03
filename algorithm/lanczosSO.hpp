/*
 * lanczosSO.hpp
 *
 *  Created on: Mar 3, 2015
 *      Author: hugo
 */

#ifndef ALGORITHM_LANCZOSSO_HPP_
#define ALGORITHM_LANCZOSSO_HPP_

#include <string>

#include "../core/matrix.hpp"
#include "../Eigen/Dense"

using namespace std;

namespace mflash{

	class LancsosSO{
			Matrix<float, EmptyType> *matrix;
			int iterations;
			int k;

		public:
			void run();
	};

	void LancsosSO::run(){
		string path = mflash::get_parent_directory(matrix->get_file());

		const int64 block_size = matrix->get_elements_by_block();



	}

}

#endif /* ALGORITHM_LANCZOSSO_HPP_ */
