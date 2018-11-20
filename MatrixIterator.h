//
// Created by molin on 20/11/2018.
//

#ifndef MATRIX_MATRIXITERATOR_H
#define MATRIX_MATRIXITERATOR_H

#include <memory>
#include "MatrixData.h"

template<typename T>
class BaseMatrixIterator {
    protected:
        std::shared_ptr<MatrixData<T>> data;
        int row, col;

    public:
        BaseMatrixIterator(const std::shared_ptr<MatrixData<T>> &data, int row, int col) : data(data), row(row), col(col) {}

        T operator*() {
            return data->get(row, col);
        }

        bool operator==(const BaseMatrixIterator<T> &other) const {
            return row == other.row && col == other.col && data == other.data;
        }

        bool operator!=(const BaseMatrixIterator<T> &other) const {
            return !(*this == other);
        }

        BaseMatrixIterator<T> &operator=(T t) {
            data->set(row, col, t);
            return this;
        }
};

template<typename T>
class MatrixRowMajorIterator : public BaseMatrixIterator<T> {
    public:
        MatrixRowMajorIterator(const std::shared_ptr<MatrixData<T>> &data, int row, int col) : BaseMatrixIterator(data, row, col) {}

        void operator++() {
            if (col + 1 >= data->columns()) {
                row++;
                col = 0;
            } else {
                col++;
            }
        }

        void operator--() {
            if (col == 0) {
                row--;
                col = data->columns() - 1;
            } else {
                col--;
            }
        }

};

template<typename T>
class MatrixColumnMajorIterator : public BaseMatrixIterator<T> {
    public:
        MatrixColumnMajorIterator(const std::shared_ptr<MatrixData<T>> &data, int row, int col) : BaseMatrixIterator(data, row, col) {}

        void operator++() {
            if (row + 1 >= data->rows()) {
                col++;
                row = 0;
            } else {
                row++;
            }
        }

        void operator--() {
            if (row == 0) {
                col--;
                row = data->rows() - 1;
            } else {
                row--;
            }
        }

};


#endif //MATRIX_MATRIXITERATOR_H
