/****************************************************************************
   kwaymergesort.h (c) 2009,2010,2011 Aaron Quinlan
   Center for Public Health Genomics
   University of Virginia
   All rights reserved.

   MIT License

****************************************************************************/
#ifndef KWAYMERGESORT_H
#define KWAYMERGESORT_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <queue>
#include <cstdio>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h> //for basename()
#include "../core/mapped_stream.hpp"

using namespace std;

bool isRegularFile(const string& filename);
// STLized version of basename()
// (because POSIX basename() modifies the input string pointer)
// Additionally: removes any extension the basename might have.
std::string stl_basename(const std::string& path);


template <class T>
class MERGE_DATA {

public:
    // data
    T data;
    mflash::MappedStream *stream;
    bool (*compFunc)(const T &a, const T &b);

    // constructor
    MERGE_DATA (const T &data,
                mflash::MappedStream *stream,
                bool (*compFunc)(const T &a, const T &b))
    :
        data(data),
        stream(stream),
        compFunc(compFunc)
    {}

    // comparison operator for maps keyed on this structure
    bool operator < (const MERGE_DATA &a) const
    {
        // recall that priority queues try to sort from
        // highest to lowest. thus, we need to negate.
        return !(compFunc(data, a.data));
    }
};


//************************************************
// DECLARATION
// Class methods and elements
//************************************************
template <class T>
class KwayMergeSort {

public:
                                  
    // constructor, using custom comparison function
    KwayMergeSort(const string &inFile,
                 string ouput,
                 bool (*compareFunction)(const T &a, const T &b) = NULL,
                 int  maxBufferSize  = 1000000,
                 bool compressOutput = false,
                 string tempPath     = "./");
    
    // constructor, using T's overloaded < operator.  Must be defined.
    KwayMergeSort(const string &inFile,
                 string ouput,
                 int  maxBufferSize  = 1000000,
                 bool compressOutput = false,
                 string tempPath     = "./");
                                               
    // destructor
    ~KwayMergeSort(void);
     
    void Sort();            // Sort the data
    void SetBufferSize(int bufferSize);   // change the buffer size
    void SetComparison(bool (*compareFunction)(const T &a, const T &b));   // change the sort criteria
    
private:
    string _inFile;
    bool (*_compareFunction)(const T &a, const T &b);
    string _tempPath;
    vector<string>    _vTempFileNames;
    vector<mflash::MappedStream*>  _vTempFiles;
    unsigned int _maxBufferSize;
    unsigned int _runCounter;
    bool _compressOutput;
    bool _tempFileUsed;
    string _out;
    int64 _outOffset;

    // drives the creation of sorted sub-files stored on disk.
    void DivideAndSort();

    // drives the merging of the sorted temp files.
    // final, sorted and merged output is written to "out".
    void Merge();

    void WriteToTempFile(const vector<T> &lines);
    void append2Output(const vector<T> &lineBuffer);
    void OpenTempFiles();
    void CloseTempFiles();
};



//************************************************
// IMPLEMENTATION
// Class methods and elements
//************************************************

// constructor
template <class T>
KwayMergeSort<T>::KwayMergeSort (const string &inFile,
                                string output,
                               bool (*compareFunction)(const T &a, const T &b),
                               int maxBufferSize,
                               bool compressOutput,
                               string tempPath)
    : _inFile(inFile)
    , _out(output)
    , _outOffset(0)
    , _compareFunction(compareFunction)
    , _tempPath(tempPath)
    , _maxBufferSize(maxBufferSize)
    , _runCounter(0)
    , _compressOutput(compressOutput)
{}

// constructor
template <class T>
KwayMergeSort<T>::KwayMergeSort (const string &inFile,
                                string output,
                               int maxBufferSize,
                               bool compressOutput,
                               string tempPath)
    : _inFile(inFile)
    , _out(output)
    , _compareFunction(NULL)
    , _tempPath(tempPath)
    , _maxBufferSize(maxBufferSize)
    , _runCounter(0)
    , _compressOutput(compressOutput)
{}

// destructor
template <class T>
KwayMergeSort<T>::~KwayMergeSort(void)
{}

// API for sorting.  
template <class T>
void KwayMergeSort<T>::Sort() { 
    DivideAndSort();
    Merge();
}

// change the buffer size used for sorting
template <class T>
void KwayMergeSort<T>::SetBufferSize (int bufferSize) {
    _maxBufferSize = bufferSize;
}

// change the sorting criteria
template <class T>
void KwayMergeSort<T>::SetComparison (bool (*compareFunction)(const T &a, const T &b)) {
    _compareFunction = compareFunction;
}


template <class T>
void KwayMergeSort<T>::DivideAndSort() {
    mflash::MappedStream input(_inFile.c_str());

    vector<T> lineBuffer;
    lineBuffer.reserve(this->_maxBufferSize/sizeof(T));
    unsigned int totalBytes = 0;  // track the number of bytes consumed so far.

    // track whether or not we actually had to use a temp
    // file based on the memory that was allocated
    _tempFileUsed = false;


    while(input.has_remain()){
        lineBuffer.push_back(input.next<T>());
        totalBytes += sizeof(T);

        // sort the buffer and write to a temp file if we have filled up our quota
        if (totalBytes > _maxBufferSize) {
            if (_compareFunction != NULL)
                sort(lineBuffer.begin(), lineBuffer.end(), *_compareFunction);
            else
                sort(lineBuffer.begin(), lineBuffer.end());
            // write the sorted data to a temp file
            WriteToTempFile(lineBuffer);
            // clear the buffer for the next run
            lineBuffer.clear();
            _tempFileUsed = true;
            totalBytes = 0;
        }
    }

    // handle the run (if any) from the last chunk of the input file.
    if (lineBuffer.empty() == false) {
        // write the last "chunk" to the tempfile if
        // a temp file had to be used (i.e., we exceeded the memory)
        if (_tempFileUsed == true) {
            if (_compareFunction != NULL)
                sort(lineBuffer.begin(), lineBuffer.end(), *_compareFunction);
            else
                sort(lineBuffer.begin(), lineBuffer.end());
            // write the sorted data to a temp file
            WriteToTempFile(lineBuffer);
           // WriteToTempFile(lineBuffer);
        }
        // otherwise, the entire file fit in the memory given,
        // so we can just dump to the output.
        else {
            if (_compareFunction != NULL)
                sort(lineBuffer.begin(), lineBuffer.end(), *_compareFunction);
            else
                sort(lineBuffer.begin(), lineBuffer.end());
            append2Output(lineBuffer);
        }
    }
}


template <class T>
void KwayMergeSort<T>::WriteToTempFile(const vector<T> &lineBuffer) {
    // name the current tempfile
    stringstream tempFileSS;
    if (_tempPath.size() == 0)
        tempFileSS << _inFile << "." << _runCounter;
    else
        tempFileSS << _tempPath << "/" << stl_basename(_inFile) << "." << _runCounter;
    string tempFileName = tempFileSS.str();

    ofstream *output;

    output = new ofstream(tempFileName.c_str(), ios::out | ios::binary | ios::ate);
    output->write((char*) lineBuffer.data(), sizeof(T) * lineBuffer.size());

    // update the tempFile number and add the tempFile to the list of tempFiles
    ++_runCounter;
    output->close();
    delete output;
    _vTempFileNames.push_back(tempFileName);
}


template <class T>
void KwayMergeSort<T>::append2Output(const vector<T> &lineBuffer) {
    int64 size_segment = sizeof(T) * lineBuffer.size();
    ofstream *output;

    auto properties = ios::out | ios::binary | ios::ate;
    if (mflash::exist_file(_out.c_str())) {
        properties |= ios::in;
    }

    output = new ofstream(_out.c_str(), properties);
    output->seekp(_outOffset, ios::beg);
    output->write((char*) lineBuffer.data(), size_segment);
    output->close();
    _outOffset += size_segment;
    std::cout<<"Appending "<<_outOffset<<std::endl;
    delete output;
}


//---------------------------------------------------------
// MergeDriver()
//
// Merge the sorted temp files.
// uses a priority queue, with the values being a pair of
// the record from the file, and the stream from which the record came.
// SEE: http://stackoverflow.com/questions/2290518/c-n-way-merge-for-external-sort, post from Eric Lippert.
//----------------------------------------------------------
template <class T>
void KwayMergeSort<T>::Merge() {

    // we can skip this step if there are no temp files to
    // merge.  That is, the entire inout file fit in memory
    // and thus we just dumped to stdout.
    if (_tempFileUsed == false)
        return;

    // open the sorted temp files up for merging.
    // loads ifstream pointers into _vTempFiles
    OpenTempFiles();

    vector<T> lineBuffer;
    lineBuffer.reserve(this->_maxBufferSize/sizeof(T));
    unsigned int totalBytes = 0;  // track the number of bytes consumed so far.


    // priority queue for the buffer.
    priority_queue< MERGE_DATA<T> > outQueue;

    // extract the first line from each temp file
    T line;
    for (size_t i = 0; i < _vTempFiles.size(); ++i) {
        line = _vTempFiles[i]->next<T>();
        outQueue.push( MERGE_DATA<T>(line, _vTempFiles[i], _compareFunction) );
    }

    // keep working until the queue is empty
    while (outQueue.empty() == false) {
        // grab the lowest element, print it, then ditch it.
        MERGE_DATA<T> lowest = outQueue.top();
        // write the entry from the top of the queue
        lineBuffer.push_back(lowest.data);
        // remove this record from the queue
        outQueue.pop();

        totalBytes+=sizeof(T);

        if(totalBytes>=_maxBufferSize){
            append2Output(lineBuffer);
            lineBuffer.clear();
            totalBytes = 0;
        }
        if(lowest.stream->has_remain()){
            line = lowest.stream->next<T>();
            outQueue.push( MERGE_DATA<T>(line, lowest.stream, _compareFunction) );
        }
    }
    if(totalBytes != 0){
        append2Output(lineBuffer);
    }
    // clean up the temp files.
    CloseTempFiles();
}


template <class T>
void KwayMergeSort<T>::OpenTempFiles() {
    for (size_t i=0; i < _vTempFileNames.size(); ++i) {

        mflash::MappedStream *file;
        file = new mflash::MappedStream(_vTempFileNames[i]);
        _vTempFiles.push_back(file);

    }
}


template <class T>
void KwayMergeSort<T>::CloseTempFiles() {
    // delete the pointers to the temp files.
    for (size_t i=0; i < _vTempFiles.size(); ++i) {
        _vTempFiles[i]->close_stream();
        delete _vTempFiles[i];
    }
    // delete the temp files from the file system.
    for (size_t i=0; i < _vTempFileNames.size(); ++i) {
       remove(_vTempFileNames[i].c_str());  // remove = UNIX "rm"
    }
}


/*
   returns TRUE if the file is a regular file:
     not a pipe/device.

   This implies that the file can be opened/closed/seek'd multiple times without losing information
 */
bool isRegularFile(const string& filename) {
       struct stat buf ;
       int i;

       i = stat(filename.c_str(), &buf);
       if (i!=0) {
               cerr << "Error: can't determine file type of '" << filename << "': " << strerror(errno) << endl;
               exit(1);
       }
       if (S_ISREG(buf.st_mode))
               return true;

       return false;
}


/*
   returns TRUE if the file has a GZIP header.
   Should only be run on regular files.
 */
bool isGzipFile(const string& filename) {
       //see http://www.gzip.org/zlib/rfc-gzip.html#file-format
       struct  {
               unsigned char id1;
               unsigned char id2;
               unsigned char cm;
       } gzip_header;
       ifstream f(filename.c_str(), ios::in|ios::binary);
       if (!f)
               return false;

       if (!f.read((char*)&gzip_header, sizeof(gzip_header)))
               return false;

       if ( gzip_header.id1 == 0x1f
                       &&
                       gzip_header.id2 == 0x8b
                       &&
                       gzip_header.cm == 8 )
               return true;

       return false;
}


string stl_basename(const string &path) {
    string result;

    char* path_dup = strdup(path.c_str());
    char* basename_part = basename(path_dup);
    result = basename_part;
    free(path_dup);

    size_t pos = result.find_last_of('.');
    if (pos != string::npos )
        result = result.substr(0,pos);

    return result;
}


#endif /* KWAYMERGESORT_H */

