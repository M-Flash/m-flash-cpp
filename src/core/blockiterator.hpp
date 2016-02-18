// Copyright (C) 2014 Hugo Gualdron <gualdron@usp.br>, Sao Paulo University
// Copyright (C) 2014 Jose Fernando Rodrigues Junior, Sao Paulo University
// Copyright (C) 2014 Duen Horng Chau, Georgia Tech University
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef MFLASH_CPP_CORE_BLOCKITERATOR_HPP_
#define MFLASH_CPP_CORE_BLOCKITERATOR_HPP_

#include "../mflash_basic_includes.hpp"

using namespace std;

namespace mflash{
	template<class E, class IdType>
	class Matrix;

	class Block{
			string block_file;
			int col;
			int row;
			BlockType type;
		public:
			Block(string block_file, int col, int row, BlockType type){
				this->block_file = block_file;
				this->row = row;
				this->col = col;
				this->type = type;
			}
			bool exist(){return exist_file(block_file);};
			int64 size(){return file_size(block_file);};
			string get_file(){return block_file;}
			int get_row(){return row;}
			int get_col(){return col;}
			BlockType getBlockType(){return type;}
			bool isSparse(){return BlockType::SPARSE == type;}
			bool isDense(){return BlockType::DENSE == type;}
	};


	template<class E, class IdType>
	class BlockIterator{
			const static int DIRECTIONS = 4;
			string file;
			Matrix<E, IdType> *matrix;
			MatrixProperties *matrix_properties;

			int partitions;
			int direction;

			int i = 0;
			int j = -1;
			//FileStream current;

			int row_offset;
			int col_offset;
			int row_sign;
			int col_sign;

			int count = 0;

			int get_row();
			int get_col();

		public:
			BlockIterator(Matrix<E, IdType> *matrix, int direction = 0);
			bool has_next();
			Block next();


	};

	template<class E, class IdType>
	BlockIterator<E, IdType>::BlockIterator(Matrix<E, IdType> *matrix, int direction){
		this->matrix = matrix;
		this->matrix_properties = &matrix->get_matrix_properties();
		this->file = matrix->get_file();
		this->partitions = matrix_properties->partitions;
		this->direction = direction;

		if(direction<0 || direction>=DIRECTIONS){
			LOG(ERROR) << "The direction value must be between 0 and " << DIRECTIONS-1;
			assert(false);
		}

		row_offset = 0;
		col_offset = 0;
		row_sign = 		-1;
		col_sign = 		-1;

		if((direction==2||direction==3)){
			row_offset = 	partitions-1;
			row_sign = 1;
		}

		if((direction==2||direction==3) && partitions%2!=0){
			col_offset = 	partitions-1;
			col_sign = 1;
		}

	}

	template<class E, class IdType>
	int BlockIterator<E, IdType>::get_row(){
		return row_offset-(row_sign)*i;
	}

	template<class E, class IdType>
	int BlockIterator<E, IdType>::get_col(){
		return col_offset-(col_sign)*j;
	}

	template<class E, class IdType>
	bool BlockIterator<E, IdType>::has_next(){
		return count!=partitions*partitions;
	}

	template<class E, class IdType>
	Block BlockIterator<E, IdType>::next(){
		if(i%2==0){
			if(j<partitions-1){
				j++;
			}else{
				i++;
				j=partitions-1;
			}
		}else{
			if(j>0){
				j--;
			}else{
				i++;
				j=0;
			}
		}
		if(count<partitions*partitions){
			count++;
			string block_file;

			int col, row;
			if(direction%2 ==0){
				col = get_row();
				row = get_col();
			}else{
				col = get_col();
				row = get_row();
			}
			block_file = get_block_file(this->file, row, col);

			Block block(block_file, get_col(), get_row(),  getBlockType<E,IdType>(partitions, matrix_properties->vertices_partition, matrix_properties->getEdgesBlock(row, col), matrix->source_size()));
			return (block);
		}

		LOG(ERROR) << "Iterator is after the last position";
		assert(false);
	}
}
#endif /* MFLASH_CPP_CORE_BLOCKITERATOR_HPP_ */
