#include <iostream>
#include "Matrix.h"
#include <vector>
#include <memory>

void assert(int expected, int actual) {
    if (expected != actual) {
        std::cout << "ERROR: expected " << expected << ", got " << actual << std::endl;
        exit(1);
    }
}

void assertAll(int expected, Matrix<int> m) {
    for (int r = 0; r < m.rows(); ++r) {
        for (int c = 0; c < m.columns(); ++c) {
            assert(expected, m.get(r, c));
        }
    }
}

void assertEquals(Matrix<int> m1, Matrix<int> m2) {
    assert(m1.columns(), m2.columns());
    assert(m1.rows(), m2.rows());

    for (int r = 0; r < m1.rows(); ++r) {
        for (int c = 0; c < m1.columns(); ++c) {
            assert(m1.get(r, c), m2.get(r, c));
        }
    }
}

template <class IT>
void testIterator(IT begin, IT end, int size) {
    int k = 0;
    for (auto it = begin; it != end; ++it) {
        assert(++k, *it);
    }
    assert(size, k);

    for (auto it = end; it != begin;) {
        --it;
        assert(k--, *it);
    }
    assert(0, k);
}

void test(Matrix<int> m) {
    //TEST ASSIGNMENTS
    for (int k = 0; k < 10; ++k) {
        for (int r = 0; r < m.rows(); ++r) {
            for (int c = 0; c < m.columns(); ++c) {
                m.set(r, c, k);
                assert(k, m.get(r, c));
            }
        }
    }

    //ASSIGN VALUES FROM [1..columns*rows] in row major order
    int k = 0;
    for (int r = 0; r < m.rows(); ++r) {
        for (int c = 0; c < m.columns(); ++c) {
            k++;
            m.set(r, c, k);
            assert(k, m.get(r, c));
        }
    }

    //TEST ROW-MAJOR ITERATOR
    testIterator(m.beginRowMajor(), m.endRowMajor(), m.size());


    //ASSIGN VALUES FROM [1..columns*rows] in column major order
    k = 0;
    for (int c = 0; c < m.columns(); ++c) {
        for (int r = 0; r < m.rows(); ++r) {
            k++;
            m.set(r, c, k);
            assert(k, m.get(r, c));
        }
    }

    //TEST COLUMN-MAJOR ITERATOR
    testIterator(m.beginColumnMajor(), m.endColumnMajor(), m.size());


    //TEST TRANSPOSE
    if (m.rows() >= 4 && m.columns() >= 3) {
        m.transpose().set(2, 3, 76);
        assert(76, m.get(3, 2));
    }
    assertEquals(m, m.transpose().transpose());

    //TESTI DIAGONAL
    if (m.isSquared()) {
        m.diagonal().set(2, 0, 54);
        assert(54, m.get(2, 2));

        assertEquals(m.diagonal(), m.diagonal().diagonalMatrix().diagonal());
    }

    //TEST DIAGONAL MATRIX
    if (m.isVector()) {
        auto d = m.diagonalMatrix();
        assert(m.rows(), d.columns());
        assert(m.rows(), d.rows());

        for (int r = 0; r < d.rows(); ++r) {
            for (int c = 0; c < d.columns(); ++c) {
                if (r == c) {
                    assert(m.get(r, 0), d.get(r, c));
                } else {
                    assert(0, d.get(r, c));
                }
            }
        }
    }

    //TEST SUBMATRIX
    if (m.rows() >= 4 && m.columns() >= 4) {
        auto sm = m.submatrix(2, 3, 2, 1);
        assert(m.get(2, 3), sm.get(0, 0));
        assert(m.get(3, 3), sm.get(1, 0));

        sm.set(0, 0, 123);
        assert(123, m.get(2, 3));
    }

    assertEquals(m, m.submatrix(0, 0, m.rows(), m.columns()));
}


int main() {
    std::cout << "START TEST" << std::endl;

    Matrix<int> sq(10, 10);
    Matrix<int> rect(5, 10);
    Matrix<int> vector(10, 1);


    assertAll(0, sq);
    assertAll(0, rect);
    assertAll(0, vector);


    test(sq);
    test(rect);
    test(vector);

    assertAll(0, sq);
    assertAll(0, rect);
    assertAll(0, vector);

    std::cout << "ALL TESTS PASSED";

    return 0;
}