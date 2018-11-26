# matrix-revolution
A simple C++ library to work with matrices.

## Context
This library was made for the [Advanced Algorithms and Programming Methods](https://www.unive.it/data/insegnamento/274858) class held by professor [Andrea Torsello](https://www.unive.it/data/5115/5590629) at [Ca' Foscari University - Venice](https://www.unive.it) in the academic year 2018/19.

Made by Mauro Molin (857855) and Marco Marangoni (858450).

## Basics
The basic idea for this project is to share data between subsequent calls. For example, taking the transposed matrix doesn't require a copy of the data.

The behavior of the copy constructior deep copies the data into a new matrix.

The library is templated to the type of the data contained in the matrix.

## Usage

### Basics
The class to use is called `Matrix`. To instantiate a new matrix you can use the following code:
```c++
Matrix<int> m(10, 20); //Creates a 10x20 matrix of integers
```

Accessing and setting data:
```c++
m(1,3) = 10;
std::cout << m(1,3); //Prints 10
```

Deep copy:
```c++
Matrix<int> m2 = m; //Triggers copy constructor that does a deep copy
m2(1,3) = 20;
std::cout << m(1,3); //Prints 10
std::cout << m2(1,3); //Prints 20
```

Supports `const` version:
```c++
const Matrix<int> m3 = m;
std::cout << m3(1,3); //Prints 10
m3(1,3) = 30; //Compile error
```

### Shared data
Returned matrices share data (until copy constructor is called):
```c++
m.transpose()(3,1) = 40;
std::cout << m(1,3); //Prints 40


auto sm = m.submatrix(2, 2, 3, 3); //Does NOT trigger copy constructor

sm(0,0) = 50;
std::cout << m(2,2); //Prints 50

m(3,3) = 60;
std::cout << sm(1,1); //Prints 60
```

Chained calls also share data:
```c++
m.traspose().diagonal()(1,0) = 70;
std::cout << m(1,1); //Prints 70
```

### Iteration
There are two iterators, one for row-major order, one for column-major order:
```c++
//For each row, for each column
for (auto it = m.beginRowMajor(); it != m.endRowMajor(); ++it) {
    std::cout << *it << std::endl;
}

//For each column, for each row
for (auto it = m.beginColumnMajor(); it != m.endColumnMajor(); ++it) {
    std::cout << *it << std::endl;
}
```


## Implementation details
The library has been implemented using the [decorator pattern](https://en.wikipedia.org/wiki/Decorator_pattern). 

### MatrixData
The `MatrixData` class is an abstract class whose purpose is to expose the getter and setter for the data. In our implementation, the set can throw an exception if the operation is not supported.

The base implementation is `VectorMatrixData<T>`: it holds the data in a linearized `std::vector<T>`.
Other implementations such as `SubmatrixMD<T>` or `TransposedMD<T>` wrap another `MatrixData<T>` and change the behavior of the getter and the setter. 
 
The base `(int, int)` constructor of `Matrix<T>` creates a `VectorMatrixData<T>` by default.

### MatrixCell
The `(int, int)` operator of `Matrix`, used to access and set the cells, returns a `MatrixCell<T>`. This class exposes the operations required to use it as a `T`, and the `=` operator in order to change the value of the cell.
This is done because for some operations (such as returning the zeroes in `diagonalMatrix`) it's not possible to return a reference to the value.

A `const Matrix<T>` cannot be modified, so it needs to return a `const MatrixCell<T>`. If we allowed the copy constructor it would be possible to assign a `const MatrixCell<T>` to a `MatrixCell<T>`, thus allowing to modify a `const Matrix<T>`.
For this reason we decided to delete the copy constructor. We could have triggered a deep copy, but this behaviour would have been unexpected by the end user and has no practical use.

### Vectors and covectors
Vectors and covectors are not specially handled. They are simply a `nx1` and `1xn` matrices. There are the methods `isVector()` and `isCovector()`. We chose to do this because they are simply a property of a matrix, and are not a characterization (e.g. a `1x1` matrix is both a vector and a covector).

## Testing
In the `tests.cpp` file there is a main function that can be called to ensure that all tests are successful. Every major method is tested. 

The library has been complied and tested with [CMake](https://cmake.org) under Windows 10.