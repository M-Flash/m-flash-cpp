/*
 * mexception.hpp
 *
 *  Created on: Feb 28, 2015
 *      Author: hugo
 */

#ifndef MFLASH_CPP_CORE_MEXCEPTION_HPP_
#define MFLASH_CPP_CORE_MEXCEPTION_HPP_

#include <exception>
#include <string>

using namespace std;

class MException: public exception
{
	char* message;

	MException(string message){
		this->message = (char*) message.c_str();
	}

	virtual const char* what() const throw()
	{
		return message;
	}
};


#endif /* MFLASH_CPP_CORE_MEXCEPTION_HPP_ */
