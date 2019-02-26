# matrix-revolution - Third assignment

The third assignment of [Advanced Algorithms and Programming Methods](https://www.unive.it/data/insegnamento/274858) class, made by Marco Marangoni (858450).

## Assignment
Extend the matrix library so that the operations can be performed concurrently.

There are two forms of concurrency to be developed:

1. Concurrent operations:  In multiple matrix operations like (A+B) * (C+D) the addition A+B and C+D can be performed asynchronously in any order (or in parallel) before the final multiplication. Same goes for sequence of multiplications like `A*B*C*D` if the optimized multiplication order happens to be `(A*B)*(C*D)`.

2. Parallel matrix multiplication: With large matrix multiplication the access times through the polymorphic wrapper can induce an sizable overhead. one way around this is to access big fixed-size submatrices with each call. 
With this optimization matrix A is composed of several submatrices (A<sub>ij</sub>), each fetched with a single polymorphic call. Matrix multiplication can be expressed easily in this form, so `C=A*B` becomes C<sub>ij</sub> = ∑<sub>k</sub> A<sub>ik</sub>*B<sub>kj</sub>, where now `*` denotes the usual matrix multiplication for the submatrices. 
Now, each Cij can be computed independently from the other and in parallel.

## Breaking changes between v2
### Casting
To simplify the library, the sum between matrices of different types is no longer automatic.
To sum or multiply matrices of different types, you have to call the new method `cast` on one of the two matrices.

For example:
```
StaticSizeMatrix<4, 9, double> A;
StaticSizeMatrix<4, 9, int> B;

...

auto sum = A + B.cast<double>();
```

### Virtual get on MatrixData
A new method has been introduced to `MatrixData`, which allows to copy a submatrix to a `VectorMatrixData`.
This allows to pay for the virtual call only once in the case you don't know the exact type of the `MatrixData`.

Since doing a virtual call for a single cell has a huge impact on performances, the method `virtualGet` has been completely removed.

## Other changes
To implement concurrency and parallelization, some changes have been made to allow more powerful manipulations of the expression tree. 

### Expression tree
Like before, when performing operations on a matrix, the `MatrixData` is being wrapped inside another `MatrixData`.
When adding or multiplying `MatrixData`, the matrices involved are wrapped inside a `MatrixData` as well.

In this version of the library, the concept of children is introduced: for example, the child of `TransposedMD` is the `MatrixData` to transpose; the children of a `SumMD` are the `MatrixData` to sum.
To get the children of a `MatrixData` you can call `virtualGetChildren()`.

By accessing the children of a matrix, it's possible to reconstruct the full tree of operations. The leafs of the tree are `MatrixData` without children, i.e. a `VectorMatrixData`.

### ConcatenationMD
`ConcatenationMD` is a new `MatrixData`, which takes a list of matrices and exposes a matrix composed by the concatenation of the given matrices.
Let's suppose `A`, `B`, `C` and `D` are 2x2 matrices.
If I crate a `ConcatenationMD((A,B,C,D), 4, 4)`, I will obtain a 4x4 matrix, whose blocks are:

```
A|B
C|D
```

This class will be used to parallelize the multiplication by computing it on different blocks, and then merging the result.

### ResizerMD
`ResizerMD` is a new `MatrixData` which wraps a given matrix and exposes a new matrix of a different size. When trying to access cells outside the bounds of the original matrix, `0` is returned.

The purpose of this matrix will be clear later. 

### MatrixData optimization
Now each `MatrixData` expose a method called `optimize` (or `virtualOptimize` if the exact instance of `MatrixData` is unknown). By default, this method does nothing but calling `optimize` on the children.

This method is called the first time the matrix is accessed. For performance reasons, in multi-threaded environments, it's possible that this method is called more than once. 
When implementing this method, it's important to use locks if the `MatrixData` needs to be optimized only once.

Moreover, a method called `virtualWaitOptimized` must be implement on the `MatrixData` which override `optimize`. This method must wait until the optimization is complete.

#### OptimizableMD
The library provides a default abstract implementation of `MatrixData`, which overrides the `optimize` method. 

The first time `optimize` is called, an abstract method is called on another thread using `std::async`. The abstract method must return a new, faster, `MatrixData`, which later on is used to accessing the matrix.

#### MaterializerMD
The simpler class that overrides `OptimizableMD` is `MaterializerMD`. 

Let's suppose you have a pointer to `MatrixData`, and you want to access a large number of cells. Normally, you would have to call `virtualGet` for each cell, but it will result on a lot of virtual calls.

In this scenario, you can use `MaterializerMD` to wrap your matrix. By using the properties of `OptimizableMD`, the first time the matrix is accessed the original matrix is copied to a faster `VectorMatrixData` on another thread. This reduces the total number of virtual calls to one.

#### MultiplyMD
In the last assignment, `MultiplyMD` was introduced: a `MatrixData` that, the first time it was accessed, optimized the multiplication tree in order to compute first the multiplications that reduce the number of dimensions the most.

This class has been changed to extend `OptimizableMD`. When optimizing, it returns a `OptimizedMultiplyMD`, which represents the optimized multiplication tree. 


#### BaseMultiplyMD
Another implementation of `OptimizableMD` is `BaseMultiplyMD`. This class performs the normal multiplication between the two given matrices.
When optimizing, a `VectorMatrixData` with the result of the multiplication is constructed. 

It will be clear later how `OptimizedMultiplyMD` uses `BaseMultiplyMD` to parallelize the multiplication.

To speed up the multiplication as much as possible, the matrices passed to `BaseMultiplyMD` must be wrapped around a `MaterializerMD`, in order to minimize the number of virtual calls. 

## Concurrency
Let's suppose you want to perform `(AxB) + (CxD)`, where `A`, `B`,`C` and `D` are matrices. The goal of concurrency is to perform `AxB` and `CxD` at the same time, on different threads.

Since the sum is a `MatrixData`, on the first access `optimize` is called on the two children, which are `MultiplyMD` in this example. Those extend `OptimizableMD`, so they perform the multiplications on a separate thread, and at the same time.

Surprisingly, the library achieves parallelization on arbitrary expression trees without further modifications. 

## Parallelization
Let's suppose you want to perform `C=AxB`, where `A` and `B` are two big matrices. The goal of parallelization is to perform the multiplication on multiple CPU cores at the same time, to make it as fast as possible.

To do this, I used the following property: `A` and `B` can be divided into blocks, then `C` can be seen as a concatenation of blocks, where `C`<sub>`ij`</sub>` = ∑`<sub>`k`</sub>`A`<sub>`ik`</sub>` * B`<sub>`kj`</sub>, where `A`<sub>`ik`</sub> and `B`<sub>`kj`</sub> are blocks of `A` and `B` respectively, and `*` is the normal matrix multiplication.

To implement this, `OptimizedMultiplyMD` (the optimized version of `MultiplyMD`) extends `OptimizableMD`, and when optimizing returns a `ConcatenationMD`, where each block is the sum of a multiplications of the blocks of `A` and `B`, like in the formula above.

A simple multiplication will be transformed on a tree like this:
- MultiplyMD
  - OptimizedMultiplyMD
    - ConcatenateMD
      - Sum
        - BaseMultiplyMD
        - BaseMultiplyMD
        - ...
      - Sum
        - BaseMultiplyMD
        - BaseMultiplyMD
        - ...
      - ...


Since the library computes concurrently a tree of operations, by transforming a single multiplication on a large tree, parallelization is automatically achieved.

### Dividing the matrices into blocks
Dividing the matrices into blocks of the same size is not always possible, for example when the number of rows or the number columns are prime numbers.

Fortunately, you can extend the rows and columns with zeros, perform the multiplication and then shrink the resulting matrix without changing the result of the multiplication.

This property makes possible to divide every matrix in blocks, independently on the original size. 

The library tries to divide the matrix in blocks of 128KB, i.e. 128x128 if using longs. This was the size with the optimal balance between memory consumption and performance. 

At the end, the subtree of `BaseMultiplyMD` looks like this:
- BaseMultiplyMD
  - ResizerMD
    - MaterializerMD
      - Matrix A
  - ResizerMD
    - MaterializerMD
      - Matrix B

