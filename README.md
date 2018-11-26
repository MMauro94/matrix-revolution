# matrix-revolution
A simple C++ library to work with matrices.

## Context
This library was made for the Advanced Algorithms class at Università Ca' Foscari by Mauro Molin (857855) and Marco Marangoni (858450).

## Basics
The basic idea for this project is to share data between subsequent calls. For example, taking the transposed matrix doesn't require a copy of the data.

The behavior of the copy constructior deep copies the data into a new matrix.

The library is templated to the type of the data contained in the matrix.

## Usage
The class to use is called `Matrix`. To instantiate a new matrix you can use the following code:
```c++
Matrix<int> m = Matrix(10, 20); //Creates a 10x20 matrix of integers
```
//TODO more examples

## Implementation details
//TODO
