/*
 * blockiterator.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: Hugo Gualdron
 */

#ifndef MFLASH_CPP_CORE_BLOCKITERATOR_HPP_
#define MFLASH_CPP_CORE_BLOCKITERATOR_HPP_

#include <string>

using namespace std;

namespace mflash{

	class Block{
			string block_file;
			int col;
			int row;
		public:
			Block(string block_file, int col, int row){
				this->block_file = block_file;
				this->row = row;
				this->col = col;
			}
			bool exist(){return exist_file(block_file);};
			int64 size(){return file_size(block_file);};
			string get_file(){return block_file;}
			int get_row(){return row;}
			int get_col(){return col;}
	};


	class BlockIterator{
			const static int DIRECTIONS = 4;
			string file;
			int blocks;
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
			BlockIterator(string file, int blocks, int direction);
			BlockIterator(string file, int blocks) : BlockIterator(file, blocks, 0){};

			bool has_next();
			Block next();


	};

	BlockIterator::BlockIterator(string file, int blocks, int direction){
		this->file= file;
		this->blocks = blocks;
		this->direction = direction;

		if(direction<0 || direction>=DIRECTIONS){
			throw 10;//new IndexOutOfBoundsException(String.format("The direction value must be between 0 and %d", DIRECTIONS-1));
		}

		row_offset = 0;
		col_offset = 0;
		row_sign = 		-1;
		col_sign = 		-1;

		if((direction==2||direction==3)){
			row_offset = 	blocks-1;
			row_sign = 1;
		}

		if((direction==2||direction==3) && blocks%2!=0){
			col_offset = 	blocks-1;
			col_sign = 1;
		}

	}

	int BlockIterator::get_row(){
		return row_offset-(row_sign)*i;
	}

	int BlockIterator::get_col(){
		return col_offset-(col_sign)*j;
	}

	bool BlockIterator::has_next(){
		return count!=blocks*blocks;
	}

	Block BlockIterator::next(){

		/*if(current != null && current.isOpen()){
			current.close();
		}*/
		if(i%2==0){
			if(j<blocks-1){
				j++;
			}else{
				i++;
				j=blocks-1;
			}
		}else{
			if(j>0){
				j--;
			}else{
				i++;
				j=0;
			}
		}
		if(count<blocks*blocks){
			count++;

	/*		//code for resolve mistake
			i =14;
			j =4;
			count=blocks*blocks;
			//end code
	*/
			string block_file;

			if(direction%2 ==0)
				block_file = get_block_file(this->file, get_col(), get_row());
			else
				block_file = get_block_file(this->file, get_row(), get_col());
			//System.out.println(blockFile);
			//if("/media/hugo/data/graphs/.G-FLASH/14_9.block".equals(blockFile.getAbsolutePath())){
			//	System.out.println();
			//}

			Block block(block_file, get_col(), get_row());
			return (block);
		}

		throw 12;
	}
}
#endif /* MFLASH_CPP_CORE_BLOCKITERATOR_HPP_ */
