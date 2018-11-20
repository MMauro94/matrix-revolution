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

        explicit Matrix(int height, int width) : data(std::make_shared<VectorMatrixData<T>>(height, width)) {

        }


        Matrix(const Matrix<T> &other) : data(std::make_shared<VectorMatrixData<T>>(other.data->copy())) {

        }

    public:

        T get(int x, int y) {
            return data->get(x, y);
        }

        void set(int x, int y, T t) {
            data->set(x, y, t);
        }

        int width() {
            return data->width();
        }

        int height() {
            return data->height();
        }

        Matrix<T> submatrix(int yOffset, int xOffset, int height, int width) {
            return Matrix<T>(std::make_shared<SubmatrixMD<T>>(yOffset, xOffset, height, width, data));
        }

        Matrix<T> transpose() {
            return Matrix<T>(std::make_shared<TransposedMD<T>>(data));
        }

        bool isSquared() {
            return height() == width();
        }

        Matrix<T> diagonal() {
            if (!isSquared()) {
                throw "diagonal() can only be called on squared matrices";
            }
            return Matrix<T>(std::make_shared<DiagonalMD<T>>(data));
        }

        bool isVector() {
            return width() == 1;
        }

        bool isCovector() {
            return height() == 1;
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
            return MatrixRowMajorIterator<T>(data, height(), 0);
        }

        void print() {
            for (int i = 0; i < this->height(); ++i) {
                for (int j = 0; j < this->width(); ++j) {
                    std::cout << this->get(i, j) << ",";
                }
                std::cout << std::endl;
            }
        }
};

#endif //MATRIX_MATRIX_H
