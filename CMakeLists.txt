cmake_minimum_required(VERSION 3.12)
project(matrix)

set(CMAKE_CXX_STANDARD 14)
include_directories(.)

add_executable(matrix multiplicationTests2.cpp Matrix.h MatrixData.h MatrixIterator.h MatrixCell.h StaticSizeMatrix.h MultiplyMatrix.h OptimizableMatrixData.h SumMatrix.h ThreadPool.h Utils.cpp Utils.h)
