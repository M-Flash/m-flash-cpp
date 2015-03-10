/*
 * matrix.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MATRIX_HPP_
#define MFLASH_CPP_CORE_MATRIX_HPP_

#include <algorithm>
#include <iostream>
#include <string>

#include "../log/easylogging++.h"
#include "array.hpp"
#include "blockiterator.hpp"
#include "direct_stream.hpp"
#include "edgelistrewriter.hpp"
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

		int64 elements_by_block_;

		Mode mode;
		ElementIdSize element_id_size;

		//mode depends of the behaviour with one thread

		BlockProperties** block_preprocessing(int block_count, Vector<V> &inVector, Vector<V> &outVector);

		BlockType get_block_type(int block_count, int64 vertex_size_bytes, int64 edge_size_bytes);

		public:
			Matrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode);
			//Matrix(string file, int64 size, bool transpose, int64 element_by_block, Mode mode);
			int64 size(){return max(n,m);}
			Matrix<V,E> transpose();
			string get_file(){return file;}
			bool is_transpose(){return transpose_;}
			ElementIdSize get_element_id_size(){return element_id_size;}
			int64 get_elements_by_block(){return elements_by_block_;}



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
		this->elements_by_block_ = element_by_block;
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

		const int64 block_size = this->elements_by_block_ ;
		const int64 blocks = this->size() / this->elements_by_block_  + (this->size() % this->elements_by_block_ == 0?0:1);
		string stream_file = get_stream_file(file);

		//initializing arrays to store the the in-vertex states and out-accumulators
		Array<V>  *outAccumulator = new Array<V>(this->elements_by_block_);
		Array<V>  *in = new Array<V> (this->elements_by_block_);

		RandomMMapPointer<V> *mmappointer = 0;

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

		BlockProperties **block_properties;

		LOG (INFO) << "- MATRIX OPERATION STARTED";
		LOG (INFO) << "- EDGE PREPROCESSING STARTED";
		block_properties = block_preprocessing(blocks, inVector, outVector);
		LOG (INFO) << "- EDGE PREPROCESSING FINISHED";

		int block_id = -1;
		while(iterator.has_next()){
			Block block = iterator.next();
			block_id = block.get_row() * blocks + block.get_col();
			BlockProperties *properties =  block_properties[block_id];

			inOffset = block.get_col()*block_size;
			LOG (INFO)<< "- PROCESSING BLOCK "<< block.get_col() << " -> " << block.get_row();

			if(!block.exist()){
					LOG (INFO)<< "- BLOCK "<< block.get_col() << " -> " << block.get_row() << " WITHOUT EDGES ";
					LOG (INFO)<< "- " << block.get_file();
					continue;
			}


			if(lastCol != block.get_col() && BlockType::M_FLASH == properties->type){
				lastCol = block.get_col();
					if(!in->was_allocated()){
							delete mmappointer;
							delete in;
							in = new Array<V> (this->elements_by_block_);
					}
					LOG (INFO)<< "--- LOADING IN-ELEMENT STATES";
					in->set_limit(inVector.load_region(inOffset, block_size, in->address()));
					in->set_offset(inOffset);
					LOG (INFO)<< "--- IN-ELEMENT STATES BEETWEEN " << in->offset() << " AND " << in->offset()  + in->limit() << " LOADED";
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
			LOG (INFO) << "--- READING EDGES SINCE : " << "----" << block.get_file();


			EdgeListThread<V,E>* thread;
			if (BlockType::M_FLASH == properties->type){
			  LOG (INFO) << "----MODE: " << "M-FLASH";
			  MatrixWorker<V, E> mWorker (&algorithm, this, in, outAccumulator, true);
			  thread = new EdgeListThread<V,E>(block.get_file(), *properties, mWorker, 0);
			}else {
        LOG (INFO) << "----MODE: " << "X-STREAM";
        MatrixWorker<V, E> mWorker (&algorithm, this, in, outAccumulator, false);
        thread = new EdgeListThread<V,E>(stream_file, *properties, mWorker, 0);
			}

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


	/**
	 *
	 */
  template<class V, class E>
  BlockProperties** Matrix<V,E>::block_preprocessing(int block_count, Vector<V> &inVector, Vector<V> &outVector){
    //We are not considering the size when the edge has other formats !!!!!!!!!!
    int64 edge_size =  2 * (element_id_size == ElementIdSize::SIMPLE? sizeof(int): sizeof(int64)) + sizeof(E);
    int64 vertex_size = sizeof(V);

    BlockProperties** properties = new BlockProperties*[block_count * block_count];
    //the direction to explore is inverse because we need to reuse the input vector than the output vector to create the update of stream
    BlockIterator iterator(file, block_count, is_transpose()?0:1);

    string stream_file = get_stream_file(this->file);
    DirectStreamWriter writer(stream_file, 0,  vertex_size * elements_by_block_);

    //setting to zero the vertex_size when it is EmptyType
    E tmp;
    EmptyType* emptyType = dynamic_cast<EmptyType*>(&tmp);
    if(emptyType != 0){
        edge_size -= sizeof(E);
    }

    int block_position = 0;

    int64 stream_offset = 0;
    int64 edge_count;
    int64 new_block_size_bytes;

    Array<V>  *in = new Array<V> (this->elements_by_block_);
    MatrixWorker<V, E> in_worker (0, this, in, in);
    int64 inOffset=0;
    int64 block_file_size;
    int row = -1;
    bool in_loaded = false;
    float threashold = 0;
    while(iterator.has_next()){
        Block block = iterator.next();
        if(block.exist()){
            block_position = block_count * block.get_col() + block.get_row();
            block_file_size = file_size(block.get_file());
            inOffset = block.get_row()*this->elements_by_block_;
            edge_count = block_file_size/ edge_size;
            new_block_size_bytes = edge_count * (vertex_size + edge_size);
            threashold = 1.0/block_count +  2* block_file_size/this->elements_by_block_ ;

            properties[block_position] = new BlockProperties(BlockType::M_FLASH, 0, file_size(block.get_file()));
            //check type of block
            if(threashold <1 && false){
                LOG (INFO)<< "- PRE-PROCESSING BLOCK "<< block.get_row() << " -> " << block.get_col();
                if(row != block.get_row()){
                    row = block.get_row();
                    in_loaded = false;
                }
                if(!in_loaded){
                    //only one worker
                    LOG (INFO)<< "--- LOADING IN-ELEMENT STATES";
                    in->set_limit(inVector.load_region(inOffset, this->elements_by_block_, in->address()));
                    in->set_offset(inOffset);
                    in_loaded = true;
                    LOG (INFO)<< "--- IN-ELEMENT STATES LOADED";
                }
                //preprocessing block
                //only one worker
                //cout << block.get_file() << endl;

                EdgeListWriter<V,E>* thread = new EdgeListWriter<V,E>( &writer, block.get_file(), *properties[block_position],  in_worker);
                thread->call();
                properties[block_position]->type = BlockType::X_STREAM;
                properties[block_position]->offset = stream_offset;
                properties[block_position]->size = new_block_size_bytes;
                stream_offset += new_block_size_bytes;
                LOG (INFO)<< "- BLOCK "<< block.get_row() << " -> " << block.get_col() << " PRE-PROCESSED";
            }
        }
        //block_position++;
    }
    writer.close_stream();
    in->free_memory();
    delete in;
    return properties;
  }

}



#endif /* MFLASH_CPP_CORE_MATRIX_HPP_ */
