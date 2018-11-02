#ifndef MATRIX_MATRIX_H
#define MATRIX_MATRIX_H

#include <ostream>
#include "MatrixData.h"
#include "ArrayMatrixData.h"
#include "matrix_exceptions.h"
#include "MatrixDataWrapper.h"

template<typename T>
class Matrix {

    private:
        MatrixData<T> *data;
        bool mustDeleteData;

        void checkSquare() const {
            if (!isSquare()) {
                throw new SquareMatrixRequiredException;
            }
        }

        void checkVector() const {
            if (!isVector()) {
                throw new VectorRequiredException;
            }
        }

        Matrix<T> _diagonal() {
            checkSquare();

            class DiagonalData : public MatrixDataWrapper<T> {
                public:

                    DiagonalData(MatrixData<T> *data) : MatrixDataWrapper(data, data->getColumns(), 1) {
                    }

                    T &get(int row, int column) {
                        return data->get(row, row);
                    }

            };

            return Matrix<T>(new DiagonalData(data));
        }

        Matrix<T> _transposed() const {
            class TransposedData : public MatrixDataWrapper<T> {
                public:

                    TransposedData(MatrixData<T> *data) : MatrixDataWrapper(data, data->getColumns(), data->getRows()) {
                    }

                    T &get(int row, int column) {
                        return data->get(column, row);
                    }

            };

            return Matrix<T>(new TransposedData(data));
        }

        Matrix<T> _submatrix(int rowOffset, int rowCount, int columnOffset, int columnCount) const {
            class SubmatrixData : public MatrixDataWrapper<T> {
                private:
                    int rowOffset;
                    int columnOffset;
                public:

                    SubmatrixData(MatrixData<T> *data, int rowOffset, int rowCount, int columnOffset, int columnCount)
                            : MatrixDataWrapper(data, rowCount, columnCount), rowOffset(rowOffset), columnOffset(columnOffset) {
                    }

                    T &get(int row, int column) {
                        return data->get(row + rowOffset, column + columnOffset);
                    }

            };

            return Matrix<T>(new SubmatrixData(data, rowOffset, rowCount, columnOffset, columnCount));
        }

        const Matrix<T> _toDiagonalMatrix() {
            checkVector();

            class DiagonalData : public MatrixDataWrapper<T> {
                private:
                    T zero = 0;
                public:

                    DiagonalData(MatrixData<T> *data) : MatrixDataWrapper(data, data->getRows(), data->getRows()) {
                    }

                    T &get(int row, int column) override {
                        if (row == column) {
                            return data->get(row, row);
                        } else {
                            return zero;
                        }
                    }
            };

            return Matrix<T>(new DiagonalData(data));
        }

        explicit Matrix(MatrixData<T> *data) : data(data) {
            data->incrementRefCount();
        }

    public:

        Matrix(const Matrix<T> &other) {
            data = other.data;
            data->incrementRefCount();
        }

        Matrix(const Matrix<T> &&other) noexcept {
            data = other.data;
            data->incrementRefCount();
        }

        class BaseMatrixRow {
            protected:
                MatrixData<T> *data;
                int row;
            public:

                BaseMatrixRow(MatrixData<T> *data, int row) {
                    this->data = data;
                    this->row = row;
                }

        };

        class ReadOnlyMatrixRow : BaseMatrixRow {
            public:
                ReadOnlyMatrixRow(MatrixData<T> *data, int row) : BaseMatrixRow(data, row) {
                }

                const T &operator[](int index) const {
                    return data->get(row, index);
                }
        };

        class MatrixRow : BaseMatrixRow {
            public:
                MatrixRow(MatrixData<T> *data, int row) : BaseMatrixRow(data, row) {
                }

                T &operator[](int index) const {
                    return data->get(row, index);
                }
        };

        Matrix(int rows, int columns) : Matrix(new ArrayMatrixData<T>(rows, columns)) {

        }

        ~Matrix() {
            if (data->decrementRefCount()) {
                std::cout << "Deleting data from matrix";
                delete (data);
            }
        }

        void clear() {
            initialize(0);
        }

        int rows() const {
            return data->getRows();
        }

        int columns() const {
            return data->getColumns();
        }

        const ReadOnlyMatrixRow operator[](int index) const {
            return ReadOnlyMatrixRow(data, index);
        }

        const MatrixRow operator[](int index) {
            return MatrixRow(data, index);
        }

        void initialize(T val) {
            for (int i = 0; i < rows(); ++i) {
                for (int j = 0; j < columns(); ++j) {
                    (*this)[i][j] = val;
                }
            }
        }

        Matrix<T> operator*(Matrix<T> other) {
            if (columns() != other.rows()) {
                throw new InvalidMatrixSizeException("Multiplication can only be performed between matrices NxM and MxO");
            }
            Matrix<T> ret(rows(), other.columns());
            for (int i = 0; i < rows(); ++i) {
                for (int j = 0; j < other.columns(); ++j) {
                    ret[i][j] = row(i).transposed().scalarProduct(other.column(j));
                }
            }
            return ret;
        }

        void multiplyBy(T factor) {
            for (int i = 0; i < rows(); ++i) {
                for (int j = 0; j < columns(); ++j) {
                    (*this)[i][j] *= factor;
                }
            }
        }

        /*Matrix<T> operator*(T factor) {
            Matrix<T> ret = *this;
            ret.multiplyBy(factor);
            return ret;
        }*/

        Matrix<T> operator/(T factor) {
            Matrix<T> ret = *this;
            ret.multiplyBy(1 / factor);
            return ret;
        }

        void sumBy(T addend) {
            for (int i = 0; i < rows(); ++i) {
                for (int j = 0; j < columns(); ++j) {
                    (*this)[i][j] += addend;
                }
            }
        }

        Matrix<T> operator+(T addend) {
            Matrix<T> ret = *this;
            ret.sumBy(addend);
            return ret;
        }

        Matrix<T> operator-(T addend) {
            Matrix<T> ret = *this;
            ret.sumBy(-addend);
            return ret;
        }

        template<typename T>
        friend std::ostream &operator<<(std::ostream &os, const Matrix<T> &matrix) {
            for (int i = 0; i < matrix.rows(); ++i) {
                for (int j = 0; j < matrix.columns(); ++j) {
                    os << matrix[i][j] << "\t";
                }
                os << std::endl;
            }
            return os;
        }

        bool isSquare() const {
            return data->getColumns() == data->getRows();
        }

        const Matrix<T> diagonal() const {
            return _diagonal();
        }

        Matrix<T> diagonal() {
            return _diagonal();
        }

        const Matrix<T> transposed() const {
            return _transposed();
        }

        Matrix<T> transposed() {
            return _transposed();
        }

        const Matrix<T> submatrix(int rowOffset, int rowCount, int columnOffset, int columnCount) const {
            return _submatrix(rowOffset, rowCount, columnOffset, columnCount);
        }

        Matrix<T> submatrix(int rowOffset, int rowCount, int columnOffset, int columnCount) {
            return _submatrix(rowOffset, rowCount, columnOffset, columnCount);
        }

        bool isVector() const {
            return columns() == 1;
        }

        bool isCovector() const {
            return rows() == 1;
        }

        const Matrix<T> toDiagonalMatrix() {
            return _toDiagonalMatrix();
        }

        const Matrix<T> column(int columnIndex) const {
            return submatrix(0, rows(), columnIndex, 1);
        }

        Matrix<T> column(int columnIndex) {
            return submatrix(0, rows(), columnIndex, 1);
        }

        const Matrix<T> row(int rowIndex) const {
            return submatrix(rowIndex, 1, 0, columns());
        }

        Matrix<T> row(int rowIndex) {
            return submatrix(rowIndex, 1, 0, columns());
        }

        T scalarProduct(const Matrix<T> other) const {
            checkVector();
            other.checkVector();

            if (rows() != other.rows()) {
                throw new InvalidMatrixSizeException("Must have the same number of rows");
            }

            T ret = 0;
            for (int i = 0; i < rows(); ++i) {
                ret += (*this)[i][0] * other[i][0];
            }
            return ret;
        }
};


#endif //MATRIX_MATRIX_H
