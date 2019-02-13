//
// Created by MMarco on 13/02/2019.
//

#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H


#include <string>
#include <iostream>

class Utils {
	public :
		static void error(const std::string &errorMessage) {
			std::cout << errorMessage << std::endl;
			throw errorMessage;
		}

		static unsigned ceilDiv(unsigned a, unsigned b) {
			return 1 + ((a - 1) / b);
		}

};


#endif //MATRIX_UTILS_H
