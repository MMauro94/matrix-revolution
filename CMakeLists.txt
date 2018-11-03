cmake_minimum_required(VERSION 3.12)
project(matrix)

set(CMAKE_CXX_STANDARD 14)

add_executable(matrix main.cpp Matrix2.h Matrix.h MatrixData.h ArrayMatrixData.h matrix_exceptions.h MatrixDataWrapper.h CommonMatrixData.h MatrixAccessors.h)