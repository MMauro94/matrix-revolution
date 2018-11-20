//
// Created by molin on 20/11/2018.
//

#ifndef MATRIX_MATRIXDATA_H
#define MATRIX_MATRIXDATA_H

#include <memory>
#include <vector>

template<typename T>
class VectorMatrixData;

template<typename T>
class MatrixData {

    private:
        int _rows, _columns;

    public:
        MatrixData(int rows, int columns) : _rows(rows), _columns(columns) {}

        int columns() {
            return this->_columns;
        }

        int rows() {
            return this->_rows;
        }

        virtual T get(int row, int col) = 0;

        virtual void set(int row, int col, T t) {
            throw "Unsupported";
        }

        VectorMatrixData<T> copy();
};


template<typename T>
class VectorMatrixData : public MatrixData<T> {

    private:
        std::vector<T> vector;

    public:
        VectorMatrixData(int rows, int columns) : MatrixData(rows, columns), vector(rows * columns) {
        }

        T get(int row, int col) override {
            return vector[row * this->columns() + col];
        }

        void set(int row, int col, T t) override {
            vector[row * this->columns() + col] = t;
        }

};


template<typename T>
VectorMatrixData<T> MatrixData<T>::copy() {
    VectorMatrixData<T> ret(rows(), columns());
    for (int i = 0; i < rows(); ++i) {
        for (int j = 0; j < columns(); ++j) {
            ret.set(i, j, get(i, j));
        }
    }
    return ret;
}

template<typename T>
class SubmatrixMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;
        int rowOffset, colOffset;


    public:

        SubmatrixMD(int rowOffset, int colOffset, int rows, int columns, const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(rows, columns),
                                                                                                                      rowOffset(rowOffset),
                                                                                                                      colOffset(colOffset),
                                                                                                                      wrapped(wrapped) {
        }

        T get(int row, int col) override {
            return wrapped->get(row + rowOffset, col + colOffset);
        }

        void set(int row, int col, T t) override {
            return wrapped->set(row + rowOffset, col + colOffset, t);
        }

};


template<typename T>
class TransposedMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;

    public:

        explicit TransposedMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(wrapped->columns(), wrapped->rows()), wrapped(wrapped) {
        }

        T get(int row, int col) override {
            return wrapped->get(col, row);
        }

        void set(int row, int col, T t) override {
            return wrapped->set(col, row, t);
        }

};


template<typename T>
class DiagonalMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;

    public:

        explicit DiagonalMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(wrapped->rows(), 1), wrapped(wrapped) {
        }

        T get(int row, int col) override {
            return wrapped->get(row, row);
        }

        void set(int row, int col, T t) override {
            return wrapped->set(row, row, t);
        }

};


template<typename T>
class DiagonalMatrixMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;

    public:

        explicit DiagonalMatrixMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(wrapped->rows(), wrapped->rows()),
                                                                                   wrapped(wrapped) {
        }

        T get(int row, int col) override {
            if (row == col) {
                return wrapped->get(row, 0);
            } else {
                return 0;
            }
        }

};

#endif //MATRIX_MATRIXDATA_H
