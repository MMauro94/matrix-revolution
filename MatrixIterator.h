//
// Created by molin on 20/11/2018.
//

#ifndef MATRIX_MATRIXITERATOR_H
#define MATRIX_MATRIXITERATOR_H

#include <memory>
#include "MatrixData.h"

template<typename T>
class MatrixRowMajorIterator {
    private:
        std::shared_ptr<MatrixData<T>> data;
        int x, y;

    public:
        MatrixRowMajorIterator(const std::shared_ptr<MatrixData<T>> &data, int x, int y) : data(data), x(x), y(y) {}

        T operator*() {
            return data->get(x, y);
        }

        void operator++() {
            if (y+1 >= data->width()) {
                x++;
                y=0;
            } else {
                y++;
            }
        }

        bool operator==(const MatrixRowMajorIterator<T> &other) const {
            return x == other.x && y == other.y && data == other.data;
        }

        bool operator!=(const MatrixRowMajorIterator<T> &other) const {
            return !(*this == other);
        }

        MatrixRowMajorIterator<T>& operator=(T t) {
            data->set(x, y, t);
            return this;
        }
};

#endif //MATRIX_MATRIXITERATOR_H
