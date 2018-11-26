#ifndef MATRIX_MATRIX_H
#define MATRIX_MATRIX_H

#include <memory>
#include <string>
#include "MatrixData.h"
#include "MatrixIterator.h"
#include "MatrixCell.h"

/**
 * The only class exposed to the user of the library. It provides every method needed to use the matrix.
 * @tparam T the type of the data contained in each matrix cell
 */
template<typename T>
class Matrix {
    protected:
        std::shared_ptr<MatrixData<T>> data; //Pointer to the class holding the data

    private:
        /** Private constructor that accepts a pointer to the data */
        explicit Matrix(const std::shared_ptr<MatrixData<T>> &data) : data(data) {}

    public:

        /**
         * Base constructor that creates a new matrix of the given size
         * @param rows number of rows
         * @param columns number of columns
         */
        explicit Matrix(int rows, int columns) : data(std::make_shared<VectorMatrixData<T>>(rows, columns)) {
        }


        /**
         * Copy constructor that triggers a deep copy of the matrix
         * @param other the other matrix
         */
        Matrix(const Matrix<T> &other) : data(std::make_shared<VectorMatrixData<T>>(other.data->copy())) {
        }

        /**
         * Move constructor. Default behaviour.
         * @param other the other matrix
         */
        Matrix(Matrix<T> &&other) noexcept = default;


        const MatrixCell<T> operator()(int row, int col) const {
            return MatrixCell<T>(data, row, col);
        }

        MatrixCell<T> operator()(int row, int col) {
            return MatrixCell<T>(data, row, col);
        }

        /**
         * @return the number of columns
         */
        int columns() const {
            return data->columns();
        }

        /**
         * @return the number of rows
         */
        int rows() const {
            return data->rows();
        }

        /**
         * @return the total number of cells (rows*columns)
         */
        int size() const {
            return rows() * columns();
        }

        Matrix<T> submatrix(int rowOffset, int colOffset, int rows, int columns) {
            return Matrix<T>(std::make_shared<SubmatrixMD<T>>(rowOffset, colOffset, rows, columns, data));
        }

        const Matrix<T> submatrix(int rowOffset, int colOffset, int rows, int columns) const {
            return Matrix<T>(std::make_shared<SubmatrixMD<T>>(rowOffset, colOffset, rows, columns, data));
        }

        Matrix<T> transpose() {
            return Matrix<T>(std::make_shared<TransposedMD<T>>(data));
        }

        const Matrix<T> transpose() const {
            return Matrix<T>(std::make_shared<TransposedMD<T>>(data));
        }

        /**
         * @return true if this matrix is a square (has the same number of rows and columns)
         */
        bool isSquared() const {
            return rows() == columns();
        }

        Matrix<T> diagonal() {
            if (!isSquared()) {
                throw "diagonal() can only be called on squared matrices";
            }
            return Matrix<T>(std::make_shared<DiagonalMD<T>>(data));
        }

        const Matrix<T> diagonal() const {
            if (!isSquared()) {
                throw "diagonal() can only be called on squared matrices";
            }
            return Matrix<T>(std::make_shared<DiagonalMD<T>>(data));
        }

        /**
         * @return true if this matrix is a vector (has only one column)
         */
        bool isVector() const {
            return columns() == 1;
        }

        /**
         * @return true if this matrix is a covector (has only one row)
         */
        bool isCovector() const {
            return rows() == 1;
        }

        /**
         * Can only be called on a vector.
         * @return an immutable diagonal square matrix that has this vector as diagonal and <code>0</code> (zero) in all other positions.
         */
        const Matrix<T> diagonalMatrix() const {
            if (!isVector()) {
                throw "diagonalMatrix() can only be called on vectors (nx1 matrices)";
            }
            return Matrix<T>(std::make_shared<DiagonalMatrixMD<T>>(data));
        }

        /**
         * @return an iterator on the first position. This iterator moves from left to right, and then top to bottom.
         */
        MatrixRowMajorIterator<T> beginRowMajor() const {
            return MatrixRowMajorIterator<T>(data, 0, 0);
        }

        /**
         * @return an iterator on the last position. This iterator moves from left to right, and then top to bottom.
         */
        MatrixRowMajorIterator<T> endRowMajor() const {
            return MatrixRowMajorIterator<T>(data, rows(), 0);
        }

        /**
         * @return an iterator on the first position. This iterator moves from top to bottom, and then left to right.
         */
        MatrixColumnMajorIterator<T> beginColumnMajor() const {
            return MatrixColumnMajorIterator<T>(data, 0, 0);
        }

        /**
        * @return an iterator on the last position. This iterator moves from top to bottom, and then left to right.
        */
        MatrixColumnMajorIterator<T> endColumnMajor() const {
            return MatrixColumnMajorIterator<T>(data, 0, columns());
        }

        /**
         * Prints the content of this matrix to the standard output
         * @param format the format string to use when printing values
         * @param separator the separator between each column
         */
        void print(const char *format, const char *separator = "  ") const {
            for (int row = 0; row < rows(); ++row) {
                for (int col = 0; col < columns(); ++col) {
                    if (col > 0) {
                        std::cout << separator;
                    }
                    printf(format, (T) (*this)(row, col));
                }
                std::cout << std::endl;
            }
        }
};

#endif //MATRIX_MATRIX_H
