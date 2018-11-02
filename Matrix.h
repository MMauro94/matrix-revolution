//
// Created by molin on 01/11/2018.
//

#ifndef MATRIX_MATRIX_H
#define MATRIX_MATRIX_H

#include <ostream>

/*
 * Cose da fare:
 * -Determinante
 * -Inversa
 * -Rank
 * -triangolare
 * -identità
 * -vettori
 *   -linearmente indipendenti?
 * -covettori
 * -submatrix
 * -trasposta
 * -diagonale
 */


template<unsigned int ROWS, unsigned int COLUMNS, class T>
class Matrix;

template<unsigned int ROWS, unsigned int COLUMNS, class T>
class BaseMatrix {

private:
    T data[ROWS][COLUMNS];

protected:
    virtual Matrix<ROWS, COLUMNS, T> cast() = 0;

public:

    BaseMatrix() {
    }

    void clear() {
        initialize(0);
    }

    void initialize(T val) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLUMNS; ++j) {
                data[i][j] = val;
            }
        }
    }

    const T *operator[](int index) const {
        return data[index];
    }

    T *operator[](int index) {
        return data[index];
    }

    template<unsigned int C>
    Matrix<ROWS, C, T> operator*(Matrix<COLUMNS, C, T> other) const {
        Matrix<ROWS, C, T> ret;
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < C; ++j) {
                ret[i][j] = 0;
                for (int k = 0; k < COLUMNS; ++k) {
                    ret[i][j] += data[i][k] * other.data[k][j];
                }
            }
        }
        return ret;
    }

    void multiplyBy(T factor) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLUMNS; ++j) {
                data[i][j] *= factor;
            }
        }
    }

    Matrix<ROWS, COLUMNS, T> operator*(T factor) {
        Matrix<ROWS, COLUMNS, T> ret = cast();
        ret.multiplyBy(factor);
        return ret;
    }

    Matrix<ROWS, COLUMNS, T> operator/(T factor) {
        Matrix<ROWS, COLUMNS, T> ret = cast();
        ret.multiplyBy(1 / factor);
        return ret;
    }


    void sumBy(T addend) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLUMNS; ++j) {
                data[i][j] += addend;
            }
        }
    }

    Matrix<ROWS, COLUMNS, T> operator+(T addend) {
        Matrix<ROWS, COLUMNS, T> ret = cast();
        ret.sumBy(addend);
        return ret;
    }

    Matrix<ROWS, COLUMNS, T> operator-(T addend) {
        Matrix<ROWS, COLUMNS, T> ret = cast();
        ret.sumBy(-addend);
        return ret;
    }

    template<unsigned int R, unsigned int C, class T>
    friend std::ostream &operator<<(std::ostream &os, const Matrix<R, C, T> &matrix) {
        for (int i = 0; i < R; ++i) {
            for (int j = 0; j < C; ++j) {
                os << matrix.data[i][j] << "\t";
            }
            os << std::endl;
        }
        return os;
    }
};

template<unsigned int ROWS, unsigned int COLUMNS, class T>
class Matrix : public BaseMatrix<ROWS, COLUMNS, T> {
protected:
    Matrix<ROWS, COLUMNS, T> cast() {
        return *this;
    }
};


//SQUARE MATRIX
template<unsigned int N, class T>
class Matrix<N, N, T> : public BaseMatrix<N, N, T> {
protected:
    Matrix<N, N, T> cast() {
        return *this;
    }

public:

    std::pair<Matrix<N, N, T>, Matrix<N, N, T>> luDecomposition() {
        Matrix<N, N, T> l, u;
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                if (j < i) {
                    l[j][i] = 0;
                } else {
                    l[j][i] = (*this)[j][i];
                    for (int k = 0; k < i; ++k) {
                        l[j][i] -= l[j][k] * u[k][i];
                    }
                }
            }

            for (int j = 0; j < N; ++j) {
                if (j < i) {
                    u[i][j] = 0;
                } else if (j == i) {
                    u[i][j] = 1;
                } else {
                    u[i][j] = (*this)[i][j] / l[i][i];
                    for (int k = 0; k < i; ++k) {
                        u[i][j] -= (l[i][k] * u[k][j]) / l[i][i];
                    }
                }
            }
        }
        return std::make_pair(l, u);
    }

    bool isUpperTriangular() {
        for (int i = 0; i < N; ++i) {
            for (int j = i + 1; j < N; ++j) {
                if ((*this)[i][j] != 0) {
                    return false;
                }
            }
        }
        return true;
    }

    bool isLowerTriangular() {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < i; ++j) {
                if ((*this)[i][j] != 0) {
                    return false;
                }
            }
        }
        return true;
    }

    bool isTriangular() {
        return isLowerTriangular() || isUpperTriangular();
    }

    bool isDiagonal() {
        return isLowerTriangular() && isUpperTriangular();
    }

    T determinant() {
        if (isTriangular()) {
            T ret = 1;
            for (int i = 0; i < N; ++i) {
                ret *= (*this)[i][i];
            }
            return ret;
        } else {
            auto lu = luDecomposition();
            return lu.first.determinant() * lu.second.determinant();
        }
    }

};

//VECTOR
template<unsigned int N, class T>
class Matrix<N, 1, T> : public BaseMatrix<N, 1, T> {

protected:
    Matrix<N, 1, T> cast() {
        return *this;
    }

};

#endif //MATRIX_MATRIX_H
