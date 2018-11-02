//
// Created by molin on 02/11/2018.
//

#ifndef MATRIX_MATRIXDATAWRAPPER_H
#define MATRIX_MATRIXDATAWRAPPER_H

#include "MatrixData.h"

template<typename T>
class MatrixDataWrapper : public MatrixData<T> {
    protected:
        MatrixData<T> *data;

        MatrixDataWrapper(MatrixData<T> *data, int rows, int columns) : MatrixData(rows, columns), data(data) {
            data->incrementRefCount();
        }

    public:
        MatrixDataWrapper(MatrixDataWrapper<T> &other) {
            data = other.data;
            data->incrementRefCount();
        }

        MatrixDataWrapper(MatrixDataWrapper<T> &&other) noexcept {
            data = other.data;
            data->incrementRefCount();
        }

        virtual ~MatrixDataWrapper() {
            if (data->decrementRefCount()) {
                std::cout << "Deleting data from wrapper";
                delete data;
            }
        }

};

#endif //MATRIX_MATRIXDATAWRAPPER_H
