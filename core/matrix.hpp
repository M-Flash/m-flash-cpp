/*
 * matrix.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MATRIX_HPP_
#define MFLASH_CPP_CORE_MATRIX_HPP_

#include <algorithm>
#include <string>
#include <iostream>

#include "array.hpp"
#include "blockiterator.hpp"
#include "edgelistthread.hpp"
#include "malgorithm.hpp"
#include "matrixworker.hpp"
#include "operator.hpp"
#include "type.hpp"
#include "vector.hpp"

using namespace std;

namespace mflash{

	template <class V, class E>
	class MAlgorithm;


	template <class V, class E>
	class MatrixWorker;


	template <class V, class E>
	class Matrix{
		string file;
		int64 n;
		int64 m;
		bool transpose_;

		int64 element_by_block_;

		Mode mode;
		ElementIdSize element_id_size;

		//mode depends of the behaviour with one thread

		public:
			Matrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode);
			//Matrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode);
			int64 size(){return max(n,m);}
			Matrix<V,E> transpose();
			string get_file(){return file;}
			bool is_transpose(){return transpose_;}
			ElementIdSize get_element_id_size(){return element_id_size;}
			int64 get_elements_by_block(){return element_by_block_;}

			void operate(MAlgorithm<V,E> &algorithm, Vector<V> &inVector, Vector<V> &outVector);


	};

	template <class V, class E>
	class InitializeOperator : public UnaryOperator<V>{
		MAlgorithm<V,E> *algorithm;
		MatrixWorker<V,E> *worker;

		public:
			InitializeOperator(MAlgorithm<V,E> *algorithm, MatrixWorker<V,E> *worker){
					this->algorithm = algorithm;
					this->worker = worker;
			}

			void apply(Element<V> & element, Element<V>& out){
				algorithm->initialize(*worker, out);
			}

	};

/*
	template <class V, class E>
	class AppyOperator: public UnaryOperator<V>{
		MAlgorithm<V,E> *algorithm;
		MatrixWorker<V,E> *worker;

		public:
			AppyOperator(MAlgorithm<V,E> *algorithm, MatrixWorker<V,E> *worker){
					this->algorithm = algorithm;
					this->worker = worker;
			}

			void apply(Element<V> & element, Element<V>& out){
				algorithm->apply(* (this->worker), out);
			}

	};
*/

	template<class V, class E>
	Matrix<V,E>::Matrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode){
		this->file = file;
		this->n = size;
		this->m = size;
		this->transpose_ = !transpose;
		this->mode = mode;
		this->element_by_block_ = element_by_block;
		this->element_id_size = ElementIdSize::SIMPLE;
	}

	template<class V, class E>
	Matrix<V,E> Matrix<V,E>::transpose(){
		Matrix<V,E> t(this);
		t.transpose_ = !t.transpose;
		return (t);
	}

	//first implementation without replicates :)

	template<class V, class E>
	void Matrix<V,E>::operate(MAlgorithm<V,E> &algorithm, Vector<V> &inVector, Vector<V> &outVector){

		const int64 block_size = this->element_by_block_ ;
		const int64 blocks = this->size() / this->element_by_block_  + (this->size() % this->element_by_block_ == 0?0:1);

		//initializing arrays to store the the in-vertex states and out-accumulators
		Array<V>  outAccumulator (this->element_by_block_);
		Array<V>  in (this->element_by_block_);

		MatrixWorker<V, E> outWorker (&algorithm, this, &outAccumulator, &outAccumulator);

		MatrixWorker<V, E> mWorker (&algorithm, this, &in, &outAccumulator);

		//block iteration
		BlockIterator iterator(file, blocks, is_transpose()?1:0);
		int row = -1;
		int lastCol = -1;
		int64 inOffset = 0;
		int64 outOffset = 0;

		InitializeOperator<V,E> initialize_operator (&algorithm, &outWorker);
		//AppyOperator<V,E> apply_operator (&algorithm, &worker);

		while(iterator.has_next()){
			Block block = iterator.next();
			if(!block.exist()){
					continue;
			}

			inOffset = block.get_col()*block_size;
			cout<< "- PROCESSING BLOCK "<< block.get_col() << " -> " << block.get_row() << endl;
			//logger.info("- PROCESSING BLOCK {} -> {}", block.getCol(), block.getRow());

			if(lastCol != block.get_col()){
				lastCol = block.get_col();
				//logger.info("--- LOADING IN-ELEMENT STATES BEETWEEN ");
				in.set_limit(inVector.load_region(inOffset, block_size, in.address()));
				in.set_offset(inOffset);
				//logger.info("--- IN-ELEMENT STATES BEETWEEN {} AND {} LOADED", in.getOffset(), in.getOffset() + in.getLimit());
			}

			if(block.get_row() != row){
				//slicing the output vertex stream
				if(row != -1){
					//making summarization
					//sumReplicates(algorithm, worker, outAccumulator, replicates);
//					logger.info("--- STORING OUT-ELEMENT STATES BEETWEEN {} AND {} ", outAccumulator.getOffset(), outAccumulator.getOffset() + outAccumulator.getLimit());
					outVector.store_region(outOffset, block_size, outAccumulator.address());
	//				logger.info("--- OUT-ELEMENT STATES BEETWEEN {} AND {} STORED", outAccumulator.getOffset(), outAccumulator.getOffset() + outAccumulator.getLimit());
				}
				row = block.get_row();

				outOffset = block.get_row()*block_size;
				//outAccumulator.setLimit(limit);

			//	logger.info("--- LOADING OUT-ELEMENT STATES ");
				outAccumulator.set_limit(outVector.load_region(outOffset, block_size, outAccumulator.address()));
				outAccumulator.set_offset(outOffset);
				//logger.info("--- OUT-ELEMENT STATES BEETWEEN {} AND {} LOADED", outAccumulator.getOffset(), outAccumulator.getOffset() + outAccumulator.getLimit());

				//initializing out_vector values
				if(algorithm.isInitialized()){
				//	logger.info("--- INITIALIZING ON OUT-ELEMENT STATES BEETWEEN {} AND {} ", outAccumulator.getOffset(), outAccumulator.getOffset() + outAccumulator.getLimit());
						outAccumulator.operate(initialize_operator, outAccumulator, outAccumulator, outAccumulator);
					//outAccumulator.operate( (UnaryOperatorExtended<Element<V>>)(v)-> {algorithm.initialize(worker, v); return v;}, outAccumulator);
				}else{
			//		logger.info("--- INITIALIZING ON OUT-ELEMENT STATES BEETWEEN {} AND {} OMITTED", outAccumulator.getOffset(), outAccumulator.getOffset() + outAccumulator.getLimit());
				}

				//copying values
				/*if(mode.equals(Mode.VECTOR_REPLICATION)){
				//	logger.info("--- REPLICATING OUT-ELEMENT STATES BEETWEEN {} AND {} ", outAccumulator.getOffset(), outAccumulator.getOffset() + outAccumulator.getLimit());
					for(int i = 0; i<Util.MATRIX_THREADS-1; i++){
						replicates[i].setLimit(outAccumulator.getLimit());
						replicates[i].setOffsetBytes(outAccumulator.getOffsetBytes());
						outAccumulator.copy(replicates[i]);
					}
				}*/

			}

			//only one worker
			EdgeListThread<V,E>* thread = new EdgeListThread<V,E>(block.get_file(), mWorker, 0);
			thread->call();

			//Stream stream (block.get_file());

				//stream.setReverse(true);
			//	createAndSummitWorkers(ecs, mode, stream, algorithm, in,  outAccumulator, replicates);

/*
				for(int j=0;j<Util.MATRIX_THREADS;j++){
					ecs.take();
				}
*/

				//closing stream
				//stream.close_stream();
		}

		if ( algorithm.isApplied() ){
//				outAccumulator.operate(apply_operator, outAccumulator, outAccumulator, outAccumulator);
		}
		//making summarization
		//sumReplicates(algorithm, worker, outAccumulator, replicates);
		//logger.info("--- STORING OUT-ELEMENT STATES BEETWEEN {} AND {} ", outAccumulator.getOffset(), outAccumulator.getOffset() + outAccumulator.getLimit());
		outVector.store_region(outOffset, block_size, outAccumulator.address());
		//logger.info("--- OUT-ELEMENT STATES BEETWEEN {} AND {} STORED", outAccumulator.getOffset(), outAccumulator.getOffset() + outAccumulator.getLimit());

		//logger.info("- FREEING RESOURCES");
		//in.free();
		//outAccumulator.free();
		//pool.shutdown();
	//	logger.info("- FINALIZING OPERATION");
	}

}



#endif /* MFLASH_CPP_CORE_MATRIX_HPP_ */
