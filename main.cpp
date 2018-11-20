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
    for (int i = 0; i < m.height(); ++i) {
        for (int j = 0; j < m.width(); ++j) {
            assert(expected, m.get(i, j));
        }
    }
}

void assertEquals(Matrix<int> m1, Matrix<int> m2) {
    assert(m1.width(), m2.width());
    assert(m1.height(), m2.height());

    for (int i = 0; i < m1.height(); ++i) {
        for (int j = 0; j < m1.width(); ++j) {
            assert(m1.get(i, j), m2.get(i, j));
        }
    }
}


void test(Matrix<int> m) {
    //TEST ASSIGNMENTS
    for (int k = 0; k < 10; ++k) {
        for (int i = 0; i < m.height(); ++i) {
            for (int j = 0; j < m.width(); ++j) {
                m.set(i, j, k);
                assert(k, m.get(i, j));
            }
        }
    }

    //ASSIGN VALUES FROM [1..width*height] in row major order
    int k = 0;
    for (int i = 0; i < m.height(); ++i) {
        for (int j = 0; j < m.width(); ++j) {
            k++;
            m.set(i, j, k);
            assert(k, m.get(i, j));
        }
    }

    //TEST ROW-MAJOR ITERATOR
    k = 0;
    for (auto it = m.beginRowMajor(); it != m.endRowMajor(); ++it) {
        assert(++k, *it);
    }
    assert(m.width() * m.height(), k);

    //TEST TRANSPOSE
    if (m.height() >= 4 && m.width() >= 3) {
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
        assert(m.height(), d.width());
        assert(m.height(), d.height());

        for (int i = 0; i < d.height(); ++i) {
            for (int j = 0; j < d.width(); ++j) {
                if (i == j) {
                    assert(m.get(i, 0), d.get(i, j));
                } else {
                    assert(0, d.get(i, j));
                }
            }
        }
    }


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