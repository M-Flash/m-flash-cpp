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

#include "../log/easylogging++.h"
#include "array.hpp"
#include "blockiterator.hpp"
#include "edgelistthread.hpp"
#include "malgorithm.hpp"
#include "matrixworker.hpp"
#include "mmappointer.hpp"
#include "operator.hpp"
#include "type.hpp"
#include "util.hpp"
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
		Array<V>  *outAccumulator = new Array<V>(this->element_by_block_);
		Array<V>  *in = new Array<V> (this->element_by_block_);

		MMapPointer<V> *mmappointer = 0;

		MatrixWorker<V, E> outWorker (&algorithm, this, outAccumulator, outAccumulator);
		//MatrixWorker<V, E> mWorker (&algorithm, this, in, outAccumulator);

		//block iteration
		BlockIterator iterator(file, blocks, is_transpose()?1:0);
		int row = -1;
		int lastCol = -1;
		int64 inOffset = 0;
		int64 outOffset = 0;

		InitializeOperator<V,E> initialize_operator (&algorithm, &outWorker);
		AppyOperator<V,E> apply_operator (&algorithm, &outWorker);

		LOG (INFO) << "- MATRIX OPERATION STARTED";
		while(iterator.has_next()){
			Block block = iterator.next();
			if(!block.exist()){
					continue;
			}

			inOffset = block.get_col()*block_size;
			LOG (INFO)<< "- PROCESSING BLOCK "<< block.get_col() << " -> " << block.get_row();

			if(lastCol != block.get_col()){
				lastCol = block.get_col();

				if( block.size() < get_mapping_limit(block_size * sizeof(V))) {
						if(!in->was_allocated()){
								in->~Array();
						}
						delete mmappointer;
						delete in;

						LOG (INFO)<< "--- IN-ELEMENT STATES WAS MAPPED. " << block.get_file();
						mmappointer = new MMapPointer<V>(inVector.get_file(), inOffset, block_size);
						in = new Array<V>(mmappointer->address(), mmappointer->size(), inOffset );
				}else{
					if(!in->was_allocated()){
							delete mmappointer;
							delete in;
							in = new Array<V> (this->element_by_block_);
					}
					LOG (INFO)<< "--- LOADING IN-ELEMENT STATES";
					in->set_limit(inVector.load_region(inOffset, block_size, in->address()));
					in->set_offset(inOffset);
					LOG (INFO)<< "--- IN-ELEMENT STATES BEETWEEN " << in->offset() << " AND " << in->offset()  + in->limit() << " LOADED";
				}
			}

			if(block.get_row() != row){
				if(row != -1){
					//making summarization
					//sumReplicates(algorithm, worker, outAccumulator, replicates);
					LOG (INFO) << "--- STORING OUT-ELEMENT STATES BEETWEEN " << outAccumulator->offset() << " AND " << outAccumulator->offset() + outAccumulator->limit()-1;
					outVector.store_region(outOffset, block_size, outAccumulator->address());
					LOG (INFO) << "--- OUT-ELEMENT STATES BEETWEEN " << outAccumulator->offset() << " AND " << outAccumulator->offset() + outAccumulator->limit()-1 << " STORED";

				}
				row = block.get_row();

				outOffset = block.get_row()*block_size;
				//outAccumulator->setLimit(limit);

				LOG (INFO)<< "--- LOADING OUT-ELEMENT STATES";
				outAccumulator->set_limit(outVector.load_region(outOffset, block_size, outAccumulator->address()));
				outAccumulator->set_offset(outOffset);
				LOG (INFO) << "--- OUT-ELEMENT STATES BEETWEEN " << outAccumulator->offset() << " AND " << outAccumulator->offset() + outAccumulator->limit()-1 << " LOADED";

				//initializing out_vector values
				if(algorithm.isInitialized()){
						LOG (INFO) << "--- INITIALIZING ON OUT-ELEMENT STATES BEETWEEN " << outAccumulator->offset() << " AND " << outAccumulator->offset() + outAccumulator->limit()-1;
						outAccumulator->operate(initialize_operator, *outAccumulator, *outAccumulator, *outAccumulator);
						LOG (INFO) << "--- OUT-ELEMENT STATES INITIALIZED";
				}else{
						LOG (INFO) << "--- INITIALIZING ON OUT-ELEMENT STATES BEETWEEN " << outAccumulator->offset() << " AND " << outAccumulator->offset() + outAccumulator->limit()-1 << " OMITTED";
				}

				//copying values
				/*if(mode.equals(Mode.VECTOR_REPLICATION)){
				//	logger.info("--- REPLICATING OUT-ELEMENT STATES BEETWEEN {} AND {} ", outAccumulator->getOffset(), outAccumulator->getOffset() + outAccumulator->getLimit());
					for(int i = 0; i<Util.MATRIX_THREADS-1; i++){
						replicates[i].setLimit(outAccumulator->getLimit());
						replicates[i].setOffsetBytes(outAccumulator->getOffsetBytes());
						outAccumulator->copy(replicates[i]);
					}
				}*/

			}
			LOG (INFO) << "--- READING EDGES ";
			MatrixWorker<V, E> mWorker (&algorithm, this, in, outAccumulator);
			//only one worker
			EdgeListThread<V,E>* thread = new EdgeListThread<V,E>(block.get_file(), mWorker, 0);
			thread->call();
			LOG (INFO) << "--- READING EDGES FINALIZED";
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

		if(algorithm.isApplied()){
				LOG (INFO) << "--- APPLYING ON OUT-ELEMENT STATES BEETWEEN " << outAccumulator->offset() << " AND " << outAccumulator->offset() + outAccumulator->limit()-1 << " STORED";
				outAccumulator->operate(apply_operator, *outAccumulator, *outAccumulator, *outAccumulator);
				LOG (INFO) << "--- OUT-ELEMENT STATES APPLYED";
		}else{
				LOG (INFO) << "--- APPLYIN ON OUT-ELEMENT STATES BEETWEEN " << outAccumulator->offset() << " AND " << outAccumulator->offset() + outAccumulator->limit()-1 << " OMITTED";
		}


		LOG (INFO) << "--- STORING OUT-ELEMENT STATES BEETWEEN " << outAccumulator->offset() << " AND " << outAccumulator->offset() + outAccumulator->limit()-1;
		outVector.store_region(outOffset, block_size, outAccumulator->address());
		LOG (INFO) << "--- OUT-ELEMENT STATES BEETWEEN " << outAccumulator->offset() << " AND " << outAccumulator->offset() + outAccumulator->limit()-1 << " STORED";
		//making summarization
		//sumReplicates(algorithm, worker, outAccumulator, replicates);

		LOG (INFO) << "- MATRIX OPERATION FINISHED";

		//in->~Array();
		//outAccumulator->~Array();
		delete mmappointer;
		delete in;
		delete outAccumulator;

	}

}



#endif /* MFLASH_CPP_CORE_MATRIX_HPP_ */
