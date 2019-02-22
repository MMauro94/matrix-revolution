# matrix-revolution - Third assignment

The third assignment of [Advanced Algorithms and Programming Methods](https://www.unive.it/data/insegnamento/274858) class, made by Marco Marangoni (858450).

## Assignment
Extend the matrix library so that the operations can be performed concurrently.

There are two forms of concurrency to be developed:

1. Concurrent operations:  In multiple matrix operations like (A+B) * (C+D) the addition A+B and C+D can be performed asynchronously in any order (or in parallel) before the final multiplication. Same goes for sequence of multiplications like `A*B*C*D` if the optimized multiplication order happens to be `(A*B)*(C*D)`.

2. Parallel matrix multiplication: With large matrix multiplication the access times through the polymorphic wrapper can induce an sizable overhead. one way around this is to access big fixed-size submatrices with each call. 
With this optimization matrix A is composed of several submatrices (A<sub>ij</sub>), each fetched with a single polymorphic call. Matrix multiplication can be expressed easily in this form, so `C=A*B` becomes C<sub>ij</sub> = ∑<sub>k</sub> A<sub>ik</sub>*B<sub>kj</sub>, where now `*` denotes the usual matrix multiplication for the submatrices. 
Now, each Cij can be computed independently from the other and in parallel.

## Changes between v2
//TODO