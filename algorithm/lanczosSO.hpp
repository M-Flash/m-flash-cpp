/*
 * lanczosSO.hpp
 *
 *  Created on: Mar 3, 2015
 *      Author: hugo
 */

#ifndef ALGORITHM_LANCZOSSO_HPP_
#define ALGORITHM_LANCZOSSO_HPP_

#include <cmath>
#include <sstream>
#include <string>

#include "../core/eigenwrapper.hpp"
#include "../core/matrix.hpp"
#include "../core/primitivematrix.hpp"
#include "../core/primitivevector.hpp"
#include "../core/type.hpp"
#include "../core/util.hpp"
#include "../Eigen/Dense"
#include "../log/easylogging++.h"

using namespace std;

namespace mflash{

	class LanczosSO{
			PrimitiveMatrix<double, EmptyType> *matrix;
			int iterations;
			int k;

			mat build_tridiagonal_matrix(int m, double alpha [], double beta[]);

		public:
			LanczosSO(PrimitiveMatrix<double, EmptyType> &matrix, int iterations, int k);
			void create_ritz_vectors(PrimitiveVector<double> *vectors[], mat &q);
			void run();
	};

	LanczosSO::LanczosSO(PrimitiveMatrix<double, EmptyType> &matrix, int iterations, int k){
		this->matrix = &matrix;
		this->iterations = iterations;
		this->k = k;
	}

	void LanczosSO::run(){
		string path = mflash::get_parent_directory(matrix->get_file());
		cout<<path<<endl;
		string v_file = path + "v.bin";
		string eigens_file = path + "eigen_values.bin";
		//string v_tmp = path + FILE_SEPARATOR + "tmp.bin";
		string r_file = path + "r.bin";

		const int64 block_size = matrix->get_elements_by_block();
		const int64 node_count = matrix->size();

		double epsilon = sqrt(1E-18);
		double *beta = new double[iterations];
		double *alpha = new double[iterations];

		//orthogonal vectors v
		PrimitiveVector<double> *vectors[iterations];// = new PrimitiveVector<double>*[iterations];
		PrimitiveVector<double> v (v_file, node_count, block_size);
		PrimitiveVector<double> r (r_file, node_count, block_size);

		LOG (INFO) << "1: initial values";
		vectors[0] = new PrimitiveVector<double> ( path + "v0", node_count, block_size );
		vectors[0]->fill_random();
		vectors[0]->multiply(((double)1/vectors[0]->pnorm(2)));

		for (int i = 0; i < iterations; i++){
				LOG (INFO) << "3: Find a new basis vector";
				matrix->multiply( *(vectors[i]), v);

				LOG (INFO) << "4:";
				alpha[i] = vectors[i]->transpose().multiply(v);

				LOG (INFO) << "5: v = v - beta[i-1]V[i-1] - alpha[i]V[i]";
				if(i>0){
						v.linear_combination(3, new double[3]{1, -1*beta[i-1], -1*alpha[i]}, new PrimitiveVector<double>*[3]{&v, vectors[i-1], vectors[i]});
				}else{
						v.linear_combination(2, new double[2]{1, -1*alpha[i]}, new PrimitiveVector<double>*[2]{&v, vectors[i]});
				}

				LOG (INFO) << "6: beta[i] = ||v||";
				beta[i] = v.pnorm(2);

				LOG (INFO) << "7: build tri-diagonal matrix from alpha and beta";
				mat ti = build_tridiagonal_matrix(i+1, alpha, beta);

				LOG (INFO) << "8: ";
				Eigen::SelfAdjointEigenSolver<mat> eigensolver(ti);
				mat evalues = eigensolver.eigenvalues();
				mat q = eigensolver.eigenvectors();

				LOG (INFO) << "Iteration " << i << ", EigenValues: ";
				LOG (INFO) << evalues;

				mat mtmp(i+1,1);// = alpha;
				array2mat(i+1, alpha, mtmp);
				LOG (INFO) << "Alphas: ";
				LOG (INFO) << mtmp;
				array2mat(i+1, beta, mtmp);
				LOG (INFO) << "Betas: ";
				LOG (INFO) << mtmp;

				//Max singular value
				double max_sv = abs((double)evalues(0));
				for (int j = 1; j <= i; j++){
						max_sv = max(max_sv, abs((double)evalues(j)));
				}
				LOG (INFO) << "Max Singular Value = " << max_sv;

				bool so = false;

				LOG (INFO) << "9: Reorthogonalization";
				for (int j = 0; j <= i; j++){
						if (beta[i] * abs((double) q(i,j)) <= epsilon * max_sv){
							 LOG (INFO) << "Reorthogonalization for ritz vector = "<< j;

							 double *constants = new double[i+1];
							 for ( int k = 0; k < i+1; k++ ){
									 constants[k] = (double)q(k,j);
							 }
							 r.linear_combination(i+1, constants, vectors);
							 //-(r*v)
							 double constant = -1 * r.transpose().multiply(v);
							 //v=v-(r*v)r
							 v.linear_combination(2, new double[2]{1, constant},  new PrimitiveVector<double>*[2]{&v, &r});
							 so = true;
							 delete constants;
						}
				}
				LOG (INFO) << "15:";
				if ( so ){
						LOG (INFO) << "16: Recompute normalization constant beta["<< i <<"]";
						beta[i] = v.pnorm(2);
				}

				LOG (INFO) << "18:";
				if ( beta[i] == 0){
						iterations = i+1;
						break;
				}

				if ( i < iterations-1 ){
						LOG (INFO) << "21: V[i+1] = v/beta[i]";
						stringstream filename;
						filename << path << "v" << (i+1);
						vectors[i+1] = (PrimitiveVector<double> *) new PrimitiveVector<double>(filename.str(), node_count, block_size);
						v.multiply(1/beta[i], *vectors[i+1]);
				}
		}
		LOG (INFO) << "Creating EigenValues";
		mat ti = build_tridiagonal_matrix(iterations, alpha, beta);
		Eigen::SelfAdjointEigenSolver<mat > eigensolver(ti);
		mat evalues = eigensolver.eigenvalues();
		mat q = eigensolver.eigenvectors();

		/*double *eigenValues  = new double[iterations];
		mat2array(evalues, eigenValues);*/

		int64 *ids = sort_and_get_indexes(iterations, evalues.data(), false);
		swap_cols(q, ids, k);
		//reducing q to k columns
		q.resize(k,k);

		//storing eigen values
		PrimitiveVector<double> eigens(eigens_file, k);
		eigens.store_region(0, k, evalues.data());


		LOG (INFO) << "Creating RitzVectors";
		create_ritz_vectors(vectors, q);

		delete ids;
		//delete [] vectors;
		delete [] alpha;
		delete [] beta;
		//delete eigenValues;
	}

	inline void LanczosSO::create_ritz_vectors(PrimitiveVector<double> *vectors[], mat &q){
	  string path = get_parent_directory(vectors[0]->get_file()) +  FILE_SEPARATOR;
	  int64 node_count = matrix->size();
	  int64 block_size = matrix->get_elements_by_block();
		int nRitzVectors = q.cols();
		int iterations = q.rows();

		PrimitiveVector<double> *ritz;

		for (int i = 0; i < nRitzVectors; i++){
		  stringstream ritz_file;
		  ritz_file << path << "RIT" << i;
			double *constants = new double[iterations];
			for ( int idx = 0; idx < iterations; idx++ ){
			   constants[idx] = (double)q(idx,i);
			}
			ritz = new PrimitiveVector<double>(ritz_file.str(), node_count , block_size);
			ritz->linear_combination(iterations, constants,  vectors);
			delete constants;
			delete ritz;
		}

	}
	mat LanczosSO::build_tridiagonal_matrix(int m, double alpha [], double beta[]){

		mat matrix(m, m);
		matrix.fill(0);

		matrix(0,0) = alpha[0];

		if(m==1) return matrix;

		matrix(0,1) = beta[0];
		matrix(m-1,m-2) = beta[m-2];
		matrix(m-1,m-1) = alpha[m-1];

		for(int i=1; i<m-1; i++){
			matrix(i,i) = 		alpha[i];
			matrix(i,i+1) = 	beta[i];
			matrix(i,i-1) =		beta[i-1];
		}
		return matrix;
	}
}

#endif /* ALGORITHM_LANCZOSSO_HPP_ */
