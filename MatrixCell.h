#ifndef MATRIX_MATRIXCELL_H
#define MATRIX_MATRIXCELL_H

#include <memory>
#include "MatrixData.h"

template<typename T>
class MatrixCell {
    private:

        std::shared_ptr<MatrixData<T>> data;
        int row, col;


    public:

        MatrixCell(const std::shared_ptr<MatrixData<T>> &data, int row, int col) : row(row), col(col), data(data) {
        }

        /** Deleted because it would have allowed to make a <code>const MatrixCell&lt;T&gt;</code> non constant */
        MatrixCell(const MatrixCell<T> &) = delete; //Copy constructor

        MatrixCell(MatrixCell<T> &&) noexcept = default; //Move constructor



        MatrixCell<T> &operator=(T const &obj) {
            this->data->set(this->row, this->col, obj);
            return *this;
        }

        operator const T() const {
            return this->data->get(this->row, this->col);
        }

};

#endif //MATRIX_MATRIXCELL_H
