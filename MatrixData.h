#ifndef MATRIX_MATRIXDATA_H
#define MATRIX_MATRIXDATA_H

#include <memory>
#include <vector>

template<typename T>
class VectorMatrixData;

/**
 * Abstract class that exposes the data of the matrix
 * @tparam T type of the data
 */
template<typename T>
class MatrixData {

    private:
        int _rows, _columns;

    public:
        MatrixData(int rows, int columns) : _rows(rows), _columns(columns) {}

        /**
         * @return number of columns
         */
        int columns() const {
            return this->_columns;
        }

        /**
         * @return number of rows
         */
        int rows() const {
            return this->_rows;
        }

        /**
         * @param row the row index
         * @param col the column index
         * @return the value at the given position
         */
        virtual T get(int row, int col) const = 0;

        /**
         * Sets the new value at the given position. Can throw exception if the operation is not supported.
         * @param row the row index
         * @param col the column index
         * @param t the new value
         */
        virtual void set(int row, int col, T t) {
            throw "Unsupported";
        }

        /**
         * @return a copy of this data
         */
        VectorMatrixData<T> copy();
};

/**
 * Implementation of <code>MatrixData</code> that actually holds the value in a <code>std::vector</code>
 * @tparam T type of the data
 */
template<typename T>
class VectorMatrixData : public MatrixData<T> {

    private:
        std::vector<T> vector;

    public:
        VectorMatrixData(int rows, int columns) : MatrixData(rows, columns), vector(rows * columns) {
        }

        T get(int row, int col) const override {
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



/**
 * Implementation of <code>MatrixData</code> that exposes a submatrix of another <code>MatrixData</code>
 * @tparam T type of the data
 */
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

        T get(int row, int col) const override {
            return wrapped->get(row + rowOffset, col + colOffset);
        }

        void set(int row, int col, T t) override {
            return wrapped->set(row + rowOffset, col + colOffset, t);
        }

};

/**
 * Implementation of <code>MatrixData</code> that exposes the transposed matrix of another <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T>
class TransposedMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;

    public:

        explicit TransposedMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(wrapped->columns(), wrapped->rows()), wrapped(wrapped) {
        }

        T get(int row, int col) const override {
            return wrapped->get(col, row);
        }

        void set(int row, int col, T t) override {
            return wrapped->set(col, row, t);
        }

};

/**
 * Implementation of <code>MatrixData</code> that exposes the diagonal vector of another squared <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T>
class DiagonalMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;

    public:

        explicit DiagonalMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(wrapped->rows(), 1), wrapped(wrapped) {
        }

        T get(int row, int col) const override {
            return wrapped->get(row, row);
        }

        void set(int row, int col, T t) override {
            return wrapped->set(row, row, t);
        }

};

/**
 * Implementation of <code>MatrixData</code> that exposes a square diagonal matrix of another vector <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T>
class DiagonalMatrixMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;

    public:

        explicit DiagonalMatrixMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(wrapped->rows(), wrapped->rows()),
                                                                                   wrapped(wrapped) {
        }

        T get(int row, int col) const override {
            if (row == col) {
                return wrapped->get(row, 0);
            } else {
                return 0;
            }
        }

};

#endif //MATRIX_MATRIXDATA_H
