#ifndef MATRIX_INVALIDMATRIXSIZEEXCEPTION_H
#define MATRIX_INVALIDMATRIXSIZEEXCEPTION_H

#include <exception>

class InvalidMatrixException : public std::exception {
    public:
        explicit InvalidMatrixException(const char message[]) : std::exception(message) {}

};

class InvalidMatrixSizeException : public InvalidMatrixException {
    public:
        explicit InvalidMatrixSizeException(const char message[]) : InvalidMatrixException(message) {}
};

class SquareMatrixRequiredException : public InvalidMatrixSizeException {
    public:
        explicit SquareMatrixRequiredException() : InvalidMatrixSizeException("The matrix must be square! (rows==columns") {}
};

class VectorRequiredException : public InvalidMatrixSizeException {
    public:
        explicit VectorRequiredException() : InvalidMatrixSizeException("The matrix must be a vector! (columns==1") {}
};

#endif //MATRIX_INVALIDMATRIXSIZEEXCEPTION_H
