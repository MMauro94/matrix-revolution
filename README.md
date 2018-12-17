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

To read and write data, you can use the `()` operator:
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

###Static matrix
If you know the size of the matrix at static time, you could use the class `StaticSizeMatrix`, which extends `Matrix`.
```c++
StaticSizeMatrix<10, 20, int> m; //Creates a 10x20 matrix of integers
```
You can use all the methods of `Matrix` on a `StaticSizeMatrix`, however all the methods add additional static information about the matrix dimensions.

For example
```c++
StaticSizeMatrix<10, 20, int> m; //Creates a 10x20 matrix of integers
//As in Matrix<int>
m(1,3) = 10;
std::cout << m(1,3); //Prints 10
//Unique for StaticSizeMatrix
m.get<1, 3>() = 10;
std::cout << m.get<1, 3>(); //Prints 10
m.get<40, 60>() = 10; //Compiler error! Index out of bounds
```

When using a `StaticSizeMatrix`, some methods will be enabled only for some specific matrices. For example, the diagonal can be obtained only for square matrices.

### Additions and multiplications
Both `Matrix` and `StaticSizeMatrix` support addition and multiplication by a constat or by another `Matrix` or `StaticSizeMatrix`.

When the size of the resulting matrix can be inferred at compile time, a `StaticSizeMatrix` is returned, otherwise a `Matrix` is returned.

The addition is allowed only for matrices of the same size and the multiplication is allowed only when the number of columns of the first matrix is the same as the number of rows of the second. When using `StaticSizeMatrix`, this check is performed at compile time, otherwise it is performed at runtime.
```c++
StaticSizeMatrix<2, 3, int> mA;
StaticSizeMatrix<3, 5, int> mB;
StaticSizeMatrix<5, 2, int> mC;
StaticSizeMatrix<2, 3, int> mD;

//...

(mA * mB * mC).print("%02d"); //Prints the product of mA, mB and mC
(2 * mA + mD + mD).print("%02d"); //Prints 2mA + 2mD
auto err = mA + mB;//Compiler error: the matrix must be of the same size
auto err2 = mA * mC;//Compiler error: incompatible sizes
```
 
 When performing a chain of multiplications, like `mA * mB * mC` in the example above, some optimization on the order of operations is made, in order to minimize the total number of calculations to perform.

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

### Static matrices specialization
When the size of a matrix is known at compile time, it is better to use `StaticSieMatrix`. This will enable additional checks at compile time, in order to reduce as much as possible the number of errors that can be raised at runtime.

To accomplish this, the library uses //TODOenable_if. This is a more flexible alternative to template specialization, since it allows to add some methods (e.g. adding the `diagonal` method for square matrices), as well as doing some parameter check (e.g. checking the bounds of the `submatrix`). 

Using //TODOenable_if also allows the IDE to understand the checks, which doesn't happen when using `static_assert` or other similar alternatives.

### Multiplication optimization
When performing a chain of multiplications, the order in which the products are performed may be optimized on order to reduce the totl number of calculations. In particular, the product which shrinks the matrices the most will be executed first.

For example, then multiplying a `2x3`, `3x5` and `5x2` matrices, the `(3x5)*(5x2)` multiplication will be executed first, since it will reduce by `5` the number of dimensions.

This is accomplished by the class `MultiplyMatrix`, which optimizes the order of operations when the first access is made. In particular the optimizations is done in these steps:
1. Create a list containing the chain of matrix to multiply
2. Until the list has more than one element
     1. Find the pair of consecutive matrix `a` and `b` which, when multiplied, will reduce the dimension the most
     2. Perform the multiplication `c = axb`
     3. Substitute `a` and `b` with `c`. This will reduce the size of the chain by one
3. The remaining matrix in the chain is the total product

##Sum and TODOmultiplication between matrices of different types
This library support adding matrices of different types. For example, a matrix of double is returned when adding a matrix of int and a matrix of doubles.
```c++
StaticSizeMatrix<4, 3, int> m1;
Matrix<double> m2(4, 3);
const Matrix<double> sum = m1 + m2;
```
This is accomplished using the //TODOkeyword `decltype`, which allows to know the type of the result of the sum of two types. For example, the signature of the addition is the following:
```c++
template<typename U>
const Matrix<decltype(T() + U())> operator+(const Matrix<U> &another) const {
    ...
}
```
As long as an empty constructor is provided for the types used in the matrices, and the addition between the two types is supported, this library will allow the sum of the two matrices.

### Vectors and covectors
Vectors and covectors are not specially handled. They are simply a `nx1` and `1xn` matrices. There are the methods `isVector()` and `isCovector()`. We chose to do this because they are simply a property of a matrix, and are not a characterization (e.g. a `1x1` matrix is both a vector and a covector).

## Testing
In the `tests.cpp` file there is a main function that can be called to ensure that all tests are successful. Every major method is tested. 

The library has been complied and tested with [CMake](https://cmake.org) under Windows 10.