/*
 * matrixworker.hpp
 *
 *  Created on: Mar 1, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MATRIXWORKER_HPP_
#define MFLASH_CPP_CORE_MATRIXWORKER_HPP_

#include <map>
#include <string>
#include <thread>

#include "../core/array.hpp"
#include "../core/matrix.hpp"
#include "../core/type.hpp"
#include "../core/util.hpp"
#include "../core/vector.hpp"
#include "../core2/edgelistthread.hpp"

using namespace std;

namespace mflash {

struct VectorPointer {
	AbstractVector *vector;
	void ** pointer;

	VectorPointer(AbstractVector *vector) {
		this->vector = vector;
		pointer = new void*;
	}
};

template<class VSource, class VDestination, class E>
class InitializeOperator;

template<class VSource, class VDestination, class E>
class AppyOperator;

template<class E>
class MatrixWorker;

template<class E>
class MatrixWorker {
public:
	Matrix<E> * matrix;
	map<string, VectorPointer*> source_map;
	map<string, VectorPointer*> destination_map;

	//used to control when the matrix worker set the data or it is loaded for the stream processor.
	bool load_vertex_data;

	AbstractVector *default_source;
	AbstractVector *default_destination;

	int64 field_count;
	void** value_pointers;

	GenericArray *source_pointer;
	GenericArray *destination_pointer;
	GenericArray **array_pointers;
	AbstractVector **vector_pointers;

	template<class VSource, class VDestination>
	void initialize_fields();

	void clean_fields();

	template<class VSource, class VDestination>
	void load_fields(FieldType type, int64 offset);
	void store_fields(int64 offset);

	int64 get_field_count() {
		return source_map.size() + destination_map.size();
	}

	BlockProperties** block_preprocessing(int block_count);

	MatrixWorker(Matrix<E> &matrix, bool load_vertex_data);
	MatrixWorker(Matrix<E> &matrix) :
			MatrixWorker(matrix, true) {
	}

	Matrix<E> get_matrix() {
		return *matrix;
	}

	//bool add_field(FieldType type, string fieldname, AbstractVector &vector);

	void set_default_source_field(AbstractVector *vector);
	void set_default_destination_field(AbstractVector *vector);

	bool add_field(FieldType type, string fieldname, AbstractVector &vector);

	bool remove_field(FieldType type, string fieldname);
	void set_field_pointer(FieldType type, string fieldname, void **pointer);

	int64 block_size();

	template<class VSource, class VDestination>
	void operate(MAlgorithm<VSource, VDestination, E> &algorithm);

};

template<class E>
inline MatrixWorker<E>::MatrixWorker(Matrix<E> &matrix, bool load_vertex_data) {
	//this->algorithm = algorithm;
	this->matrix = &matrix;
	//this->out_array = out_array;
	//this->in_array = in_array;
	this->load_vertex_data = load_vertex_data;
	this->default_source = 0;
	this->default_destination = 0;
	this->vector_pointers = 0;
	this->destination_pointer = 0;
	this->source_pointer = 0;
	this->value_pointers = 0;
	this->field_count = 0;
	this->array_pointers = 0;

}

/*template <class E>
 inline bool MatrixWorker<E>::add_field(FieldType type, string fieldname, AbstractVector &vector){
 return add_field(type, fieldname, vector, false);
 }*/

template<class E>
inline void MatrixWorker<E>::set_default_source_field(AbstractVector *vector) {
	this->default_source = vector;
}

template<class E>
inline void MatrixWorker<E>::set_default_destination_field(
		AbstractVector *vector) {
	this->default_destination = vector;
}

template<class E>
inline bool MatrixWorker<E>::add_field(FieldType type, string fieldname,
		AbstractVector &vector) {

	/*  if(default_field){
	 if(FieldType::SOURCE == type){
	 default_source = vector;
	 }else{
	 default_destination = vector;
	 }
	 return;
	 }
	 */
	if (FieldType::SOURCE == type) {
		source_map[fieldname] = new VectorPointer(&vector);
	} else {
		destination_map[fieldname] = new VectorPointer(&vector);
	}

	return true;
}

template<class E>
inline bool MatrixWorker<E>::remove_field(FieldType type, string fieldname) {

	if (FieldType::SOURCE == type) {
		source_map.erase(fieldname);
	} else {
		destination_map.erase(fieldname);
	}
	return true;
}

template<class E>
inline void MatrixWorker<E>::set_field_pointer(FieldType type, string fieldname,
		void **pointer) {
	VectorPointer *vpointer;

	if (FieldType::SOURCE == type) {
		vpointer = source_map[fieldname];
	} else {
		vpointer = destination_map[fieldname];
	}
	*(vpointer->pointer) = pointer;
}

template<class E>
inline int64 MatrixWorker<E>::block_size() {

	int64 size = 0;
	//check the size using the available memory and the fields registered.
	map<string, VectorPointer*>::iterator iter;
	VectorPointer *vpointer;
	for (iter = source_map.begin(); iter != source_map.end(); iter++) {
		vpointer = iter->second;
		size += vpointer->vector->element_size();
	}
	if (default_source != 0) {
		size += default_source->element_size();
	}

	if (default_destination != 0) {
		size += default_destination->element_size();
	}

	if (size != 0) {
		return MEMORY_SIZE_BYTES / size;
	}

	return 0;
}

template<class E>
template<class VSource, class VDestination>
void MatrixWorker<E>::initialize_fields() {
	field_count = source_map.size() + destination_map.size();
	value_pointers = new void*[field_count];
	array_pointers = new GenericArray*[field_count];//(GenericArray*) malloc(sizeof(GenericArray) * field_count);//  new GenericArray[field_count];
	vector_pointers = new AbstractVector*[field_count];

	VectorPointer *vpointer;
	map<string, VectorPointer*>::iterator iter;

	int64 block_size = this->block_size();
	int pos = 0;

	if (default_source != 0) {
		source_pointer = new Array<VSource>(block_size);
	}

	if (default_destination != 0) {
		destination_pointer = new Array<VDestination>(block_size);
	}

	for (iter = source_map.begin(); iter != source_map.end(); iter++, pos++) {
		vpointer = iter->second;
		value_pointers[pos] = *(vpointer->pointer);
		array_pointers[pos] = new GenericArray(vpointer->vector->element_size(),
				block_size);
		vector_pointers[pos] = vpointer->vector;
	}

	for (iter = destination_map.begin(); iter != destination_map.end();
			iter++, pos++) {
		vpointer = iter->second;
		value_pointers[pos] = *(vpointer->pointer);
		array_pointers[pos] = new GenericArray(vpointer->vector->element_size(),
				block_size);
		vector_pointers[pos] = vpointer->vector;
	}

}

template<class E>
void MatrixWorker<E>::clean_fields() {
	if (default_source != 0) {
		delete[] source_pointer;
	}

	if (default_destination != 0) {
		delete[] destination_pointer;
	}

	for (int i = 0; i < field_count; i++) {
		array_pointers[i]->free_memory();
	}

	delete[] value_pointers;
	delete[] vector_pointers;
	delete[] array_pointers;
	//free (array_pointers);

	field_count = 0;
	value_pointers = 0;
	array_pointers = 0;
	vector_pointers = 0;

}

template<class E>
template<class VSource, class VDestination>
inline void MatrixWorker<E>::load_fields(FieldType type, int64 offset) {

	int64 block_size = this->block_size();
	int pos = 0;
	int size = 0;

	if (FieldType::SOURCE == type) {
		pos = 0;
		size = source_map.size();

		if (default_source != 0) {
			Array<VSource> *source_wrapped =
					dynamic_cast<Array<VSource>*>(source_pointer);
			source_wrapped->set_limit(
					default_source->load_region(offset, block_size,
							source_pointer->address()));
			source_wrapped->set_offset(offset);
		}
	} else {
		pos = source_map.size();
		size = destination_map.size();

		if (default_destination != 0) {
			Array<VDestination> *destination_wrapped = dynamic_cast<Array<
					VDestination>*>(destination_pointer);
			destination_wrapped->set_limit(
					default_source->load_region(offset, block_size,
							destination_pointer->address()));
			destination_wrapped->set_offset(offset);
		}
	}

	for (int i = pos; i < size; i++) {
		array_pointers[i]->set_limit(
				vector_pointers[i]->load_region(offset, block_size,
						array_pointers[i]->address()));
		array_pointers[i]->set_offset(offset);
	}
}

template<class E>
inline void MatrixWorker<E>::store_fields(int64 offset) {
	if (default_destination != 0) {
		default_destination->store_region(offset, block_size(),
				destination_pointer->address());
	}
}

template<class E>
template<class VSource, class VDestination>
void MatrixWorker<E>::operate(MAlgorithm<VSource, VDestination, E> &algorithm) {
	const int64 elements = matrix->size();
	int64 block_size = this->block_size();
	block_size = block_size == 0 ? matrix->get_elements_by_block() : block_size;
	const int64 blocks = matrix->size() / block_size
			+ (matrix->size() % block_size == 0 ? 0 : 1);
	string stream_file = get_stream_file(matrix->get_file());

	/*
	 *CHECK IF  VSource = Source Field
	 *
	 */

	//initializing arrays to store the the in-vertex states and out-accumulators
	//MatrixWorker<VSource, VDestination, E> outWorker (&algorithm, this, outAccumulator, outAccumulator);
	//MatrixWorker<V, E> mWorker (&algorithm, this, in, outAccumulator);
	//block iteration
	BlockIterator iterator(matrix->get_file(), blocks,
			matrix->is_transpose() ? 1 : 0);

	int row = -1;
	int last_col = -1;
	int64 source_offset = 0;
	int64 destination_offset = 0;
	int64 source_limit = 0;
	int64 destination_limit = 0;

	LOG (INFO)<< "- MATRIX OPERATION STARTED";
	algorithm.before_iteration(0, *this);

	initialize_fields<VSource, VDestination>();

	Array<VDestination> *destination_wrapped =
			dynamic_cast<Array<VDestination>*>(destination_pointer);

	InitializeOperator<VSource, VDestination, E> initialize_operator(&algorithm,
			this);
	AppyOperator<VSource, VDestination, E> apply_operator(&algorithm, this);

	BlockProperties **block_properties;

	LOG (INFO)<< "- EDGE PREPROCESSING STARTED";
	block_properties = block_preprocessing(blocks);
	LOG (INFO)<< "- EDGE PREPROCESSING FINISHED";

	int block_id = -1;
	while (iterator.has_next()) {
		Block block = iterator.next();
		block_id = block.get_row() * blocks + block.get_col();
		BlockProperties *properties = block_properties[block_id];

		source_offset = block.get_col() * block_size;
		source_limit = min(source_offset + block_size, elements - source_offset)
				- 1;

		LOG (INFO)<< "- PROCESSING BLOCK "<< block.get_col() << " -> " << block.get_row();

		if (!block.exist()) {
			LOG (INFO)<< "- BLOCK "<< block.get_col() << " -> " << block.get_row() << " WITHOUT EDGES ";
			LOG (INFO)<< "- " << block.get_file();
			continue;
		}

		if (last_col != block.get_col()
				&& BlockType::DENSE == properties->type) {
			last_col = block.get_col();
			LOG (INFO)<< "--- LOADING IN-ELEMENT STATES";
			load_fields<VSource, VDestination>(FieldType::SOURCE,
					source_offset);
			LOG (INFO)<< "--- IN-ELEMENT STATES BEETWEEN " << source_offset << " AND " << source_limit<< " LOADED";
		}

		if (block.get_row() != row) {
			if (row != -1) {
				//making summarization
				//sumReplicates(algorithm, worker, outAccumulator, replicates);
				LOG (INFO)<< "--- STORING OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit;
				store_fields(destination_offset);
				LOG (INFO) << "--- OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " STORED";

			}
			row = block.get_row();

			destination_offset = block.get_row()*block_size;
			destination_limit = min(destination_offset + block_size, elements-destination_offset) -1;

			LOG (INFO)<< "--- LOADING OUT-ELEMENT STATES";
			load_fields<VSource, VDestination>(FieldType::DESTINATION, source_offset);
			LOG (INFO) << "--- OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " LOADED";

			//initializing out_vector values
			if(algorithm.is_initialized() && default_destination != 0) {
				LOG (INFO) << "--- INITIALIZING ON OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit;
				destination_wrapped->operate(initialize_operator, *destination_wrapped, *destination_wrapped, *destination_wrapped);
				LOG (INFO) << "--- OUT-ELEMENT STATES INITIALIZED";
			} else {
				LOG (INFO) << "--- INITIALIZING ON OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " OMITTED";
			}

			//copying values
			/*if(mode.equals(Mode.VECTOR_REPLICATION)){
			 //  logger.info("--- REPLICATING OUT-ELEMENT STATES BEETWEEN {} AND {} ", outAccumulator->getOffset(), outAccumulator->getOffset() + outAccumulator->getLimit());
			 for(int i = 0; i<Util.MATRIX_THREADS-1; i++){
			 replicates[i].setLimit(outAccumulator->getLimit());
			 replicates[i].setOffsetBytes(outAccumulator->getOffsetBytes());
			 outAccumulator->copy(replicates[i]);
			 }
			 }*/

		}
		LOG (INFO)<< "--- READING EDGES SINCE : " << "----" << block.get_file();
		//string file, BlockProperties &properties, int64 edge_size, int id, bool transpose, ElementIdSize &element_id_size,AbstractProcessor &edge_processor){
		AbstractProcessor *processor = new EdgeProcessor<VSource, VDestination,
				E>(*this, algorithm);
		EdgeListThread *thread_;
		EdgeListThread *thread2_;
		ElementIdSize id_size = matrix->get_element_id_size();
		if (BlockType::DENSE == properties->type) {
			LOG (INFO)<< "----MODE: " << "M-FLASH";
			thread_ = new EdgeListThread(block.get_file(), *properties, 0, 0, matrix->is_transpose(), id_size, processor);
			//thread2_ = new EdgeListThread(block.get_file(), *properties, 0, 1, matrix->is_transpose(), id_size, processor);
		} else {
			LOG (INFO) << "----MODE: " << "X-STREAM";
			thread_ = new EdgeListThread(stream_file, *properties, 0, 0, matrix->is_transpose(), id_size, processor);
			//thread_ = new EdgeListThread(stream_file, *properties, 0, 1, matrix->is_transpose(), id_size, processor);

		}
		//if(block_id != 0){
		//std::thread first (&EdgeListThread::call, thread_);
		//std::thread second(&EdgeListThread::call, thread2_);
		//first.join();
		thread_->call();
		//second.join();
		// }
		LOG (INFO)<< "--- READING EDGES FINALIZED";
		//Stream stream (block.get_file());

		//stream.setReverse(true);
		//  createAndSummitWorkers(ecs, mode, stream, algorithm, in,  outAccumulator, replicates);

		/*
		 for(int j=0;j<Util.MATRIX_THREADS;j++){
		 ecs.take();
		 }
		 */

		//closing stream
		//stream.close_stream();
	}

	if (algorithm.is_applied() && default_destination != 0) {
		LOG (INFO)<< "--- APPLYING ON OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " STORED";
		destination_wrapped->operate(apply_operator, *destination_wrapped, *destination_wrapped, *destination_wrapped);
		LOG (INFO) << "--- OUT-ELEMENT STATES APPLYED";
	} else {
		LOG (INFO) << "--- APPLYIN ON OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " OMITTED";
	}

	LOG (INFO)<< "--- STORING OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit;
	store_fields(destination_offset);
	LOG (INFO)<< "--- OUT-ELEMENT STATES BEETWEEN " << destination_offset << " AND " << destination_limit << " STORED";
	//making summarization
	//sumReplicates(algorithm, worker, outAccumulator, replicates);
	algorithm.after_iteration(0, *this);
	LOG (INFO)<< "- MATRIX OPERATION FINISHED";
	clean_fields();

}

template<class E>
BlockProperties** MatrixWorker<E>::block_preprocessing(int block_count) {
	//const int64 elements = matrix->size();
	int64 block_size = this->block_size();
	block_size = block_size == 0 ? matrix->get_elements_by_block() : block_size;

	//const int64 blocks = matrix->size() / block_size  + (matrix->size() % block_size== 0?0:1);
	string stream_file = get_stream_file(matrix->get_file());

	//We are not considering the size when the edge has other formats !!!!!!!!!!
	int64 edge_size = 2
			* (matrix->get_element_id_size() == ElementIdSize::SIMPLE ?
					sizeof(int) : sizeof(int64)) + sizeof(E);
	int64 vertex_size = 0;    //sizeof(V);

	BlockProperties** properties = new BlockProperties*[block_count
			* block_count];
	//the direction to explore is inverse because we need to reuse the input vector than the output vector to create the update of stream
	BlockIterator iterator(matrix->get_file(), block_count,
			matrix->is_transpose() ? 0 : 1);

	DirectStreamWriter writer(stream_file, 0, vertex_size * block_size);

	//setting to zero the vertex_size when it is EmptyType
	E tmp;
	EmptyField* emptyType = dynamic_cast<EmptyField*>(&tmp);
	if (emptyType != 0) {
		edge_size -= sizeof(E);
	}

	int block_position = 0;

	int64 stream_offset = 0;
	int64 edge_count;
	int64 new_block_size_bytes;

	//  Array<V>  *in = new Array<V> (block_size);
	//MatrixWorker<E> in_worker (0, this, in, in);
	//int64 inOffset=0;
	int64 block_file_size;
	int row = -1;
	bool in_loaded = false;
	float threashold = 0;
	while (iterator.has_next()) {
		Block block = iterator.next();
		if (block.exist()) {
			block_position = block_count * block.get_col() + block.get_row();
			block_file_size = file_size(block.get_file());
			//inOffset = block.get_row()* block_size;
			edge_count = block_file_size / edge_size;
			new_block_size_bytes = edge_count * (vertex_size + edge_size);
			threashold = 1.0 / block_count + 2 * block_file_size / block_size;

			properties[block_position] = new BlockProperties(BlockType::DENSE,
					0, file_size(block.get_file()));
			//check type of block
			if (threashold < 1 && false) {
				LOG (INFO)<< "- PRE-PROCESSING BLOCK "<< block.get_row() << " -> " << block.get_col();
				if(row != block.get_row()) {
					row = block.get_row();
					in_loaded = false;
				}
				if(!in_loaded) {
					//only one worker
					/* LOG (INFO)<< "--- LOADING IN-ELEMENT STATES";
					 in->set_limit(inVector.load_region(inOffset, block_size, in->address()));
					 in->set_offset(inOffset);
					 in_loaded = true;
					 LOG (INFO)<< "--- IN-ELEMENT STATES LOADED";*/
				}
				//preprocessing block
				//only one worker
				//cout << block.get_file() << endl;

				/* EdgeListWriter<V,E>* thread = new EdgeListWriter<V,E>( &writer, block.get_file(), *properties[block_position],  in_worker);
				 thread->call();*/
				properties[block_position]->type = BlockType::SPARSE;
				properties[block_position]->offset = stream_offset;
				properties[block_position]->size = new_block_size_bytes;
				stream_offset += new_block_size_bytes;
				LOG (INFO)<< "- BLOCK "<< block.get_row() << " -> " << block.get_col() << " PRE-PROCESSED";
			}
		}
		//block_position++;
	}
	writer.close_stream();
	// in->free_memory();
	//delete in;
	return properties;
}

template<class VSource, class VDestination, class E>
class InitializeOperator: public UnaryOperator<VDestination> {
	MAlgorithm<VSource, VDestination, E> *algorithm;
	MatrixWorker<E> *worker;

public:
	InitializeOperator(MAlgorithm<VSource, VDestination, E> *algorithm,
			MatrixWorker<E> *worker) {
		this->algorithm = algorithm;
		this->worker = worker;
	}

	void apply(Element<VDestination> & element, Element<VDestination>& out) {
		algorithm->initialize(*worker, out);
	}

};

template<class VSource, class VDestination, class E>
class AppyOperator: public UnaryOperator<VDestination> {
	MAlgorithm<VSource, VDestination, E> *algorithm;
	MatrixWorker<E> *worker;

public:
	AppyOperator(MAlgorithm<VSource, VDestination, E> *algorithm,
			MatrixWorker<E> *worker) {
		this->algorithm = algorithm;
		this->worker = worker;
	}

	void apply(Element<VDestination> & element, Element<VDestination>& out) {
		algorithm->apply(*(this->worker), out);
	}

};

}

#endif /* MFLASH_CPP_CORE_MATRIXWORKER_HPP_ */
