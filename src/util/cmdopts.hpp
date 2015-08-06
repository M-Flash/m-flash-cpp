


/**
 * @file
 * @author  Hugo Gualdron<gualdron@usp.br>
 * @version 1.0
 *
 * @section LICENSE
 *
 * Copyright [2012] [Aapo Kyrola, Guy Blelloch, Carlos Guestrin / Carnegie Mellon University]
 * Copyright [2014] [Hugo Gualdron, Jose Fernando Rodrigues Junior, Duen Horng Chau / Sao Paulo University , Georgia Tech University]
 * 
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

 * @section DESCRIPTION
 *
 * Graph conversion and parsing routines.
 *
 * NOTICE:
 * This file was adapted from mflash project https://github.com/mflash/mflash-cpp/blob/master/src/util/cmdopts.hpp
 *
 */

#ifndef MFLASH_CMDOPTS_DEF
#define MFLASH_CMDOPTS_DEF

#include <assert.h>
#include <string>
#include <iostream>
#include <stdint.h>

#include "configfile.hpp"
#include "mflashnames.hpp"

namespace mflash {
    
    bool _cmd_configured = false;
    
    int _argc;
    char **_argv;
    std::map<std::string, std::string> conf;
    
    
    void  set_conf(std::string key, std::string value) {
        conf[key] = value;
    }
    
    // Config file
    std::string  get_config_option_string(const char *option_name) {
        if (conf.find(option_name) != conf.end()) {
            return conf[option_name];
        } else {
            std::cout << "ERROR: could not find option " << option_name << " from config.";
            assert(false);
        }
    }
    
     std::string  get_config_option_string(const char *option_name,
                                                 std::string default_value) {
        if (conf.find(option_name) != conf.end()) {
            return conf[option_name];
        } else {
            return default_value;
        }
        
    }
    int  get_config_option_int(const char *option_name, int default_value) {
        if (conf.find(option_name) != conf.end()) {
            return atoi(conf[option_name].c_str());
        } else {
            return default_value;
        }
    }
    
    int  get_config_option_int(const char *option_name) {
        if (conf.find(option_name) != conf.end()) {
            return atoi(conf[option_name].c_str());
        } else {
            std::cout << "ERROR: could not find option " << option_name << " from config.";
            assert(false);
        }
    }
    
    uint64_t  get_config_option_long(const char *option_name, uint64_t default_value) {
        if (conf.find(option_name) != conf.end()) {
            return atol(conf[option_name].c_str());
        } else {
            return default_value;
        }
    }
    double  get_config_option_double(const char *option_name, double default_value) {
        if (conf.find(option_name) != conf.end()) {
            return atof(conf[option_name].c_str());
        } else {
            return default_value;
        }
    }
    
    void set_argc(int argc, const char ** argv);
    void set_argc(int argc, const char ** argv) {
        _argc = argc;
        _argv = (char**)argv;
        _cmd_configured = true;
        conf = loadconfig(filename_config_local(), filename_config());
        
        /* Load --key=value type arguments into the conf map */
        std::string prefix = "--";
        for (int i = 1; i < argc; i++) {
            std::string arg = std::string(_argv[i]);
            
            if (arg.substr(0, prefix.size()) == prefix) {
                arg = arg.substr(prefix.size());
                size_t a = arg.find_first_of("=", 0);
                if (a != arg.npos) {
                    std::string key = arg.substr(0, a);
                    std::string val = arg.substr(a + 1);
                    
                    std::cout << "[" << key << "]" << " => " << "[" << val << "]" << std::endl;
                    conf[key] = val;
                }
            }
        }

    }
    
    void mflash_init(int argc, const char ** argv);
    void mflash_init(int argc, const char ** argv) {
        set_argc(argc, argv);
    }
    
    void mflash_init(int argc, char ** argv){
    	mflash_init(argc, const_cast<const char**>(argv));
    }

    void check_cmd_init() {
        if (!_cmd_configured) {
            std::cout << "ERROR: command line options not initialized." << std::endl;
            std::cout << "       You need to call set_argc() in the beginning of the program." << std::endl;
        }
    }
    
    

    
    std::string  get_option_string(const char *option_name,
                                         std::string default_value)
    {
        check_cmd_init();
        int i;
        
        for (i = _argc - 2; i >= 0; i -= 1)
            if (strcmp(_argv[i], option_name) == 0)
                return std::string(_argv[i + 1]);
        return get_config_option_string(option_name, default_value);
    }
    
    std::string  get_option_string(const char *option_name)
    {
        int i;
        check_cmd_init();
        
        for (i = _argc - 2; i >= 0; i -= 1)
            if (strcmp(_argv[i], option_name) == 0)
                return std::string(_argv[i + 1]);
        return get_config_option_string(option_name);
    }
    
    std::string  get_option_string_interactive(const char *option_name, std::string options)
    {
        int i;
        check_cmd_init();
        
        for (i = _argc - 2; i >= 0; i -= 1)
            if (strcmp(_argv[i], option_name) == 0)
                return std::string(_argv[i + 1]);
        if (conf.find(option_name) != conf.end()) {
            return conf[option_name];
        } 

        std::cout << "Please enter value for command-line argument [" << std::string(option_name) << "]"<< std::endl;
        std::cout << "  (Options are: " << options << ")" << std::endl;
        
        std::string val;
        std::cin >> val;
        
        return val;
    }
    
    
    
    
    
    int  get_option_int(const char *option_name, int default_value)
    {
        int i;
        check_cmd_init();
        
        for (i = _argc - 2; i >= 0; i -= 1)
            if (strcmp(_argv[i], option_name) == 0)
                return atoi(_argv[i + 1]);
        
        return get_config_option_int(option_name, default_value);
    }
    
    int  get_option_int(const char *option_name)
    {
        int i;
        check_cmd_init();
        
        for (i = _argc - 2; i >= 0; i -= 1)
            if (strcmp(_argv[i], option_name) == 0)
                return atoi(_argv[i + 1]);
        
        return get_config_option_int(option_name);

    }

    
    
    uint64_t  get_option_long(const char *option_name, uint64_t default_value)
    {
        int i;
        check_cmd_init();
        
        for (i = _argc - 2; i >= 0; i -= 1)
            if (strcmp(_argv[i], option_name) == 0)
                return atol(_argv[i + 1]);
        return get_config_option_long(option_name, default_value);
    }
    
    float  get_option_float(const char *option_name, float default_value)
    {
        int i;
        check_cmd_init();
        
        for (i = _argc - 2; i >= 0; i -= 1)
            if (strcmp(_argv[i], option_name) == 0)
                return (float)atof(_argv[i + 1]);
        return (float) get_config_option_double(option_name, default_value);
    }
    
} // End namespace


#endif


