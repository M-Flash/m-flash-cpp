

/**
 * @file
 * @author  Aapo Kyrola <akyrola@cs.cmu.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * Copyright [2012] [Aapo Kyrola, Guy Blelloch, Carlos Guestrin / Carnegie Mellon University]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 
 *
 * @section DESCRIPTION
 *
 * Returns standard filenames for all the data files used by GraphChi.
 * All functions expect a "basefilename".
 * You can specify environment variable "GRAPHCHI_ROOT", which is the
 * root directory for the GraphChi configuration and source directories.
 */

#ifndef MFLASH_FILENAMES_DEF
#define MFLASH_FILENAMES_DEF

#include <fstream>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>
#include "../mflash_basic_includes.hpp"


namespace mflash{
    
    /**
     * Vertex data file
     */
    template <typename VertexDataType>
    static std::string filename_vertex_data(std::string basefilename) {
        std::stringstream ss;
        ss << basefilename;
        ss << "." << sizeof(VertexDataType) << "B.vout";
        return ss.str();
    }
    
    static std::string filename_degree_data(std::string basefilename)  {
        return basefilename + "_degs.bin";
    }
    
    static std::string filename_intervals(std::string basefilename, int nshards) {
        std::stringstream ss;
        ss << basefilename;
        ss << "." << nshards << ".intervals";
        return ss.str();
    }
    
    
    static std::string  get_part_str(int p, int nshards) {
        char partstr[32];
        sprintf(partstr, ".%d_%d", p, nshards);
        return std::string(partstr);
    }
    
    template <typename EdgeDataType>
    static std::string filename_shard_edata(std::string basefilename, int p, int nshards) {
        std::stringstream ss;
        ss << basefilename;
#ifdef DYNAMICEDATA
        ss << ".dynamic.";
#else
        ss << ".edata.";
#endif
#ifndef GRAPHCHI_DISABLE_COMPRESSION
        ss << ".Z.";
#endif
        ss << "e" << sizeof(EdgeDataType) << "B.";
        ss << p << "_" << nshards;
        
        return ss.str();
    }
    
    
    
    
    static std::string dirname_shard_edata_block(std::string edata_shardname, size_t blocksize) {
        std::stringstream ss;
        ss << edata_shardname;
        ss << "_blockdir_" << blocksize;
        return ss.str();
    }
    
    template <typename EdgeDataType>
    static size_t get_shard_edata_filesize(std::string edata_shardname) {
        size_t fsize;
        std::string fname = edata_shardname + ".size";
        std::ifstream ifs(fname.c_str());
        if (!ifs.good()) {
            LOG(FATAL) << "Could not load " << fname << ". Preprocessing forgotten?" << std::endl;
            assert(ifs.good());
        }
        ifs >> fsize;
        ifs.close();
        return fsize;
    }
    
    
    static std::string filename_shard_edata_block(std::string edata_shardname, int blockid, size_t blocksize) {
        std::stringstream ss;
        ss << dirname_shard_edata_block(edata_shardname, blocksize);
        ss << "/";
        ss << blockid;
        return ss.str();
    }
    
    
    static std::string filename_shard_adj(std::string basefilename, int p, int nshards) {
        std::stringstream ss;
        ss << basefilename;
        ss << ".edata_azv.";
        ss << p << "_" << nshards << ".adj";
        return ss.str();
    }
    
    static std::string filename_shard_adjidx(std::string adjfilename) {
        return adjfilename + "idx";
    }
    
    /**
     * Configuration file name
     */
    static std::string filename_config();
    static std::string filename_config() {
        char * chi_root = getenv("MFLASH_ROOT");
        if (chi_root != NULL) {
            return std::string(chi_root) + "/conf/mflash.cnf";
        } else {
            return "conf/mflash.cnf";
        }
    }
    
    /**
     * Configuration file name - local version which can
     * override the version in the version control.
     */
    static std::string filename_config_local();
    static std::string filename_config_local() {
        char * chi_root = getenv("MFLASH_ROOT");
        if (chi_root != NULL) {
            return std::string(chi_root) + "/conf/mflash.local.cnf";
        } else {
            return "conf/mflash.local.cnf";
        }
    }
    
    
    static bool file_exists(std::string sname);
    static bool file_exists(std::string sname) {
        int tryf = open(sname.c_str(), O_RDONLY);
        if (tryf < 0) {
            return false;
        } else {
            close(tryf);
            return true;
        }
    }
    
    
    /**
     * Returns the number of vertices in a graph. The value is stored in a separate file <graphname>.numvertices
     */
    static  size_t get_num_vertices(std::string basefilename);
    static  size_t get_num_vertices(std::string basefilename) {
        std::string numv_filename = basefilename + ".numvertices";
        std::ifstream vfileF(numv_filename.c_str());
        if (!vfileF.good()) {
            LOG(ERROR) << "Could not find file " << numv_filename << std::endl;
            LOG(ERROR) << "Maybe you have old shards - please recreate." << std::endl;
            assert(false);
        }
        size_t n;
        vfileF >> n;
        vfileF.close();
        return n;
    }
    
    template <typename EdgeDataType>
    std::string preprocess_filename(std::string basefilename) {
        std::stringstream ss;
        ss << basefilename;
        ss << "." <<  sizeof(EdgeDataType) << "B.bin";
        return ss.str();
    }
    
    
}

#endif

