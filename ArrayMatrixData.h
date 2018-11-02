#ifndef MATRIX_ARRAYMATRIXDATA_H
#define MATRIX_ARRAYMATRIXDATA_H

#include "MatrixData.h"
#include <iostream>

template<typename T>
class ArrayMatrixData : public MatrixData<T> {
    private:
        T **data;

    public:
        ArrayMatrixData(int rows, int columns) : MatrixData<T>(rows, columns) {
            data = new T *[rows];
            for (int i = 0; i < rows; ++i) {
                this->data[i] = new T[columns];
            }
        }

        ~ArrayMatrixData() {
            std::cout << "Deleting data";
            for (int i = 0; i < getRows(); ++i) {
                delete[](data[i]);
            }
            delete[](data);
        }

        T &get(int row, int column) override {
            return data[row][column];
        }
};

#endif //MATRIX_ARRAYMATRIXDATA_H
