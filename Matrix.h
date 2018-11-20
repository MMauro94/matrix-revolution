//
// Created by molin on 20/11/2018.
//

#ifndef MATRIX_MATRIX_H
#define MATRIX_MATRIX_H

#include <memory>
#include "MatrixData.h"
#include "MatrixIterator.h"

template<typename T>
class Matrix {
    protected:
        std::shared_ptr<MatrixData<T>> data;

    public:
        explicit Matrix(const std::shared_ptr<MatrixData<T>> &data) : data(data) {}

        explicit Matrix(int rows, int columns) : data(std::make_shared<VectorMatrixData<T>>(rows, columns)) {

        }


        Matrix(const Matrix<T> &other) : data(std::make_shared<VectorMatrixData<T>>(other.data->copy())) {

        }

    public:

        T get(int row, int col) {
            return data->get(row, col);
        }

        void set(int row, int col, T t) {
            data->set(row, col, t);
        }

        int columns() {
            return data->columns();
        }

        int rows() {
            return data->rows();
        }

        int size() {
            return rows() * columns();
        }

        Matrix<T> submatrix(int rowOffset, int colOffset, int rows, int columns) {
            return Matrix<T>(std::make_shared<SubmatrixMD<T>>(rowOffset, colOffset, rows, columns, data));
        }

        Matrix<T> transpose() {
            return Matrix<T>(std::make_shared<TransposedMD<T>>(data));
        }

        bool isSquared() {
            return rows() == columns();
        }

        Matrix<T> diagonal() {
            if (!isSquared()) {
                throw "diagonal() can only be called on squared matrices";
            }
            return Matrix<T>(std::make_shared<DiagonalMD<T>>(data));
        }

        bool isVector() {
            return columns() == 1;
        }

        bool isCovector() {
            return rows() == 1;
        }

        Matrix<T> diagonalMatrix() {
            if (!isVector()) {
                throw "diagonalMatrix() can only be called on vectors (nx1 matrices)";
            }
            return Matrix<T>(std::make_shared<DiagonalMatrixMD<T>>(data));
        }

        MatrixRowMajorIterator<T> beginRowMajor() {
            return MatrixRowMajorIterator<T>(data, 0, 0);
        }

        MatrixRowMajorIterator<T> endRowMajor() {
            return MatrixRowMajorIterator<T>(data, rows(), 0);
        }

        MatrixColumnMajorIterator<T> beginColumnMajor() {
            return MatrixColumnMajorIterator<T>(data, 0, 0);
        }

        MatrixColumnMajorIterator<T> endColumnMajor() {
            return MatrixColumnMajorIterator<T>(data, 0, columns());
        }

        void print() {
            for (int i = 0; i < this->rows(); ++i) {
                for (int j = 0; j < this->columns(); ++j) {
                    std::cout << this->get(i, j) << ",";
                }
                std::cout << std::endl;
            }
        }
};

#endif //MATRIX_MATRIX_H
