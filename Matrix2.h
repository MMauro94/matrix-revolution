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


/*
//SQUARE MATRIX
template<unsigned int N, class T>
class Matrix2<N, N, T> : public BaseMatrix<N, N, T> {
protected:
    Matrix2<N, N, T> cast() {
        return *this;
    }

public:

    std::pair<Matrix2<N, N, T>, Matrix2<N, N, T>> luDecomposition() {
        Matrix2<N, N, T> l, u;
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
*/
#endif //MATRIX_MATRIX_H
