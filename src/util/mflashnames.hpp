

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

#include <stdlib.h>
#include <string>



namespace mflash{
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

    
    
}

#endif

