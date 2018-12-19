# matrix-revolution - Second assignment

The second assignment of [Advanced Algorithms and Programming Methods](https://www.unive.it/data/insegnamento/274858) class, made by Marco Marangoni (858450).

## Assignment
Extend the matrix library (either the one presented or, if you feel confident and lucky, your own) to include the following features:

1.  Compile-time matrix dimensions: provide a variant of the matrix that maintains compile-time information about height and width. allow accessors to be bounds-checked at compile time and maintain compile-time size information as much as possible through matrix operators
2.  Matrix operations: provide matrix addition and (row-by-column) multiplication. Operations should interact with compile-time dimensions correctly (dimensions checked at compile-time).

When dealing with multiple multiplications, the library must perform the sequence of multiplications in the most computationally efficient order, i.e., by multiplying matrices contracting the largest dimension first. For example, let A, B, and C be 2x3, 3x5, and 5x2 matrices respectively, then the expression A*B*C should be computed as if it where A*(B*C), performing first the multiplication B*C since it is contracting the dimension 5, and then multiplying A by the result, contracting the smaller dimension 3.

## Changes between v1
Beside the changes made to solve the second assignment, the major change has been the migration from dynamic decorator to static decorator.

This has been done by adding on the template of `Matrix` and `MatrixData` the information about the call chain.

This change allowed to put the class `MatrixData` completely inside `Matrix`, without needing smart pointers. To keep the library still able to share data, the smart pointer has been moved into the class `VectorMatrixData` (the `MatrixData` implementation which actually holds the vector with the data).

It may seem like a small change, but it allowed to remove pointers in most of the classes, like `MatrixCell`, `MatrixIterator` and every other `MatrixData`implementation.

## Matrix of static dimension
The first task of the second assignment was to add a new matrix class that has static information about the matrix size.

I've chosen to call this class `StaticSizeMatrix`, subclass of `Matrix`. 

The main features of `StaticSizeMatrix` are:
1.  On the constructor, the sizes are checked
2.  An accessor named `get<int, int>()` has been added to read and write data to the matrix. The difference with the operator `(int, int)` is that bounds are checked at compile time
3.  Two variant of the function `submatrix` which return a `StaticSizeMatrix` have been added:
    1.  `submatrix<int, int, int, int>()`, in which all bounds are checked at compile time
    2.  `submatrix<int, int>(int, int)` in which the size of the submatrix is in the template, and it is checked at compile time
4.  The methods `transpose`, `diagonal`, `diagonalMatrix` now return a `StaticSizeMatrix` and are enabled only when the matrix is of the correct size (e.g. `diagonal` can be called only on a square matrix).

### Disable methods
To disable certain methods when the size was incompatible with the operation, I've used `std::enable_if`, which allows to check at compile time for a condition on the template parameters.

This is a more flexible alternative to template specialization, since it allows to add some methods (e.g. adding the `diagonal` method for square matrices), as well as doing some parameter check (e.g. checking the bounds of the `submatrix`). 

Using `std::enable_if` also allows the IDE to understand the checks, which doesn't happen when using `static_assert` or other similar alternatives.

### Hierarchy
Since `StaticMatrixData` extends `Matrix`, most methods of `Matrix` are still available directly (e.g. `submatrix` when the window size is not constant, and the accessor when the coordinates are not constant).

The methods that are not directly available, are shadowed by methods which return a more specific type. E.g. the method `Matrix::transpose` is shadowed by `StaticSizeMatrix::transpose`.
This is not a problem, since there aren't any drawbacks in using the overloaded methods.

Moreover, since `StaticSizeMatrix` doesn't add any field to the class, it is completely possible to upcast it to `Matrix` without any slicing or other side effects.

## Additions and multiplications
To solve the second task, both `Matrix` and `StaticSizeMatrix` now support addition and multiplication by another `Matrix` or `StaticSizeMatrix`.

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
 
### Multiplication optimizations
When performing a multiplication between three or more matrices, like `m1 * m2 * m3`, the order of operations will be rearranged in order to reduce the total number of calculations needed.

For example, then multiplying a `2x3`, `3x5` and `5x2` matrices, the `(3x5)*(5x2)` multiplication will be executed first, since it will reduce by `5` the number of dimensions.

This is done inside the decorator class `MultiplyMatrix`. At the first access to the data, the following operations are performed:
1.  The chain of multiplications is saved inside a vector
2.  If the chain is only two matrices long, I can stop the optimization in order to keep accessing the data without using virtual operators
3.  While the chain has more than one element:
    1.  Find the pair of matrices in the chain that will be the most efficient to multiply
    2.  Replacing the two matrices with a `MatrixData` that represents their multiplication
4.  The last matrix in the chain is the result of the multiplication

In the end, inside `MultiplyMatrix` there will be the optimized operation tree, which can be accessed in an optimal order.

### Sum between matrices of different types
This library support adding matrices of different types. For example, a matrix of double is returned when adding a matrix of int and a matrix of doubles.
```c++
StaticSizeMatrix<4, 3, int> m1;
Matrix<double> m2(4, 3);
const auto sum = m1 + m2; //sum is a Matrix of double
```
This is accomplished using the `decltype` specifier, which allows to know the type of the result of the sum of two types. For example, the signature of the addition is the following:
```c++
template<typename U, class MD2>
const Matrix<decltype(T() + U()), SumMatrix<decltype(T() + U()), MD, MD2>> operator+(const Matrix<U, MD2> &another) const {
    ...
}
```
As long as an empty constructor is provided for the types used in the matrices, and the addition between the two types is supported, this library will allow the sum of the two matrices.