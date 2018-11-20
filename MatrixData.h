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
        int _height, _width;

    public:
        MatrixData(int height, int width) : _height(height), _width(width) {}

        int width() {
            return this->_width;
        }

        int height() {
            return this->_height;
        }

        virtual T get(int x, int y) = 0;

        virtual void set(int x, int y, T t) {
            throw "Unsupported";
        }

        VectorMatrixData<T> copy();
};


template<typename T>
class VectorMatrixData : public MatrixData<T> {

    private:
        std::vector<T> vector;

    public:
        VectorMatrixData(int height, int width) : MatrixData(height, width), vector(height * width) {
        }

        T get(int x, int y) override {
            return vector[x * this->width() + y];
        }

        void set(int x, int y, T t) override {
            vector[x * this->width() + y] = t;
        }

};


template<typename T>
VectorMatrixData<T> MatrixData<T>::copy() {
    VectorMatrixData<T> ret(height(), width());
    for (int i = 0; i < height(); ++i) {
        for (int j = 0; j < width(); ++j) {
            ret.set(i, j, get(i, j));
        }
    }
    return ret;
}

template<typename T>
class SubmatrixMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;
        int xOffset, yOffset;


    public:

        SubmatrixMD(int yOffset, int xOffset, int height, int width, const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(height, width),
                                                                                                                      xOffset(xOffset),
                                                                                                                      yOffset(yOffset),
                                                                                                                      wrapped(wrapped) {
        }

        T get(int x, int y) override {
            return wrapped->get(x + xOffset, y + yOffset);
        }

        void set(int x, int y, T t) override {
            return wrapped->set(x + xOffset, y + yOffset, t);
        }

};


template<typename T>
class TransposedMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;

    public:

        explicit TransposedMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(wrapped->width(), wrapped->height()), wrapped(wrapped) {
        }

        T get(int x, int y) override {
            return wrapped->get(y, x);
        }

        void set(int x, int y, T t) override {
            return wrapped->set(y, x, t);
        }

};


template<typename T>
class DiagonalMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;

    public:

        explicit DiagonalMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(wrapped->height(), 1), wrapped(wrapped) {
        }

        T get(int x, int y) override {
            return wrapped->get(x, x);
        }

        void set(int x, int y, T t) override {
            return wrapped->set(x, x, t);
        }

};


template<typename T>
class DiagonalMatrixMD : public MatrixData<T> {

    private:

        std::shared_ptr<MatrixData<T>> wrapped;

    public:

        explicit DiagonalMatrixMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData(wrapped->height(), wrapped->height()),
                                                                                   wrapped(wrapped) {
        }

        T get(int x, int y) override {
            if (x == y) {
                return wrapped->get(x, 0);
            } else {
                return 0;
            }
        }

};

#endif //MATRIX_MATRIXDATA_H
