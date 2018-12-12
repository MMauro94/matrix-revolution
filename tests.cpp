#include <iostream>
#include <vector>
#include <memory>
#include "Matrix.h"
#include "StaticSizeMatrix.h"


void initializeCells(Matrix<int> &m, int rowMultiplier, int colMultiplier) {
	for (unsigned int row = 0; row < m.rows(); ++row) {
		for (unsigned int col = 0; col < m.columns(); ++col) {
			m(row, col) = row * rowMultiplier + col * colMultiplier;
		}
	}
}

void assert(int expected, int actual) {
	if (expected != actual) {
		std::cout << "ERROR: expected " << expected << ", got " << actual << std::endl;
		exit(1);
	}
}

void assertAll(int expected, Matrix<int> m) {
	for (unsigned int r = 0; r < m.rows(); ++r) {
		for (unsigned int c = 0; c < m.columns(); ++c) {
			assert(expected, m(r, c));
		}
	}
}

void assertEquals(Matrix<int> m1, Matrix<int> m2) {
	assert(m1.columns(), m2.columns());
	assert(m1.rows(), m2.rows());

	for (unsigned int r = 0; r < m1.rows(); ++r) {
		for (unsigned int c = 0; c < m1.columns(); ++c) {
			assert(m1(r, c), m2(r, c));
		}
	}
}

template<class IT>
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
		for (unsigned int r = 0; r < m.rows(); ++r) {
			for (unsigned int c = 0; c < m.columns(); ++c) {
				m(r, c) = k;
				assert(k, m(r, c));
			}
		}
	}

	//ASSIGN VALUES FROM [1..columns*rows] in row major order
	int k = 0;
	for (unsigned int r = 0; r < m.rows(); ++r) {
		for (unsigned int c = 0; c < m.columns(); ++c) {
			k++;
			m(r, c) = k;
			assert(k, m(r, c));
		}
	}

	//TEST ROW-MAJOR ITERATOR
	testIterator(m.beginRowMajor(), m.endRowMajor(), m.size());


	//ASSIGN VALUES FROM [1..columns*rows] in column major order
	k = 0;
	for (unsigned int c = 0; c < m.columns(); ++c) {
		for (unsigned int r = 0; r < m.rows(); ++r) {
			k++;
			m(r, c) = k;
			assert(k, m(r, c));
		}
	}

	//TEST COLUMN-MAJOR ITERATOR
	testIterator(m.beginColumnMajor(), m.endColumnMajor(), m.size());


	//TEST TRANSPOSE
	if (m.rows() >= 4 && m.columns() >= 3) {
		m.transpose()(2, 3) = 76;
		assert(76, m(3, 2));
	}
	assertEquals(m, m.transpose().transpose());

	//TESTI DIAGONAL
	if (m.isSquared()) {
		m.diagonal()(2, 0) = 54;
		assert(54, m(2, 2));

		assertEquals(m.diagonal(), m.diagonal().diagonalMatrix().diagonal());
	}

	//TEST DIAGONAL MATRIX
	if (m.isVector()) {
		auto d = m.diagonalMatrix();
		assert(m.rows(), d.columns());
		assert(m.rows(), d.rows());

		for (unsigned int r = 0; r < d.rows(); ++r) {
			for (unsigned int c = 0; c < d.columns(); ++c) {
				if (r == c) {
					assert(m(r, 0), d(r, c));
				} else {
					assert(0, d(r, c));
				}
			}
		}
	}

	//TEST SUBMATRIX
	if (m.rows() >= 4 && m.columns() >= 4) {
		auto sm = m.submatrix(2, 3, 2, 1);
		assert(m(2, 3), sm(0, 0));
		assert(m(3, 3), sm(1, 0));

		sm(0, 0) = 123;
		assert(123, m(2, 3));
	}

	assertEquals(m, m.submatrix(0, 0, m.rows(), m.columns()));
}


void testMultiplicationAndAddition() {
	StaticSizeMatrix<4, 3, int> m1;
	StaticSizeMatrix<3, 5, int> m2;
	initializeCells(m1, 12, 5);
	initializeCells(m2, 7, 13);
	const StaticSizeMatrix<4, 3, int> &sum1 = m1 + m1;
	const StaticSizeMatrix<4, 5, int> &multiplication1 = m1 * m2;

	m1.print("%02d");
	m2.print("%02d");
	multiplication1.print("%04d");
	assert(92, sum1.get<3, 2>());
	assert(7327, multiplication1.get<3, 4>());


	StaticSizeMatrix<42, 100, int> m3;
	StaticSizeMatrix<100, 28, int> m4;
	StaticSizeMatrix<28, 14, int> m5;
	initializeCells(m3, 12, 5);
	initializeCells(m4, 7, 13);
	initializeCells(m5, 3, 5);
	const StaticSizeMatrix<42, 14, int> &multiplication2 = m3 * m4 * m5;
}

int main() {
	/*
	 * MAIN THAT PERFORMS SOME TESTS
	 * IN CASE OF FAILURE, THE PROGRAM EXITS
	 */

	std::cout << "START TEST" << std::endl;

	Matrix<int> sq(10, 10);
	StaticSizeMatrix<15, 20, int> static1;
	StaticSizeMatrix<20, 3, int> static2;
	Matrix<int> rect(5, 10);
	Matrix<int> vector(10, 1);

	const StaticSizeMatrix<20, 15, int> &transposed = static1.transpose();
	const StaticSizeMatrix<4, 5, int> &submatrix = static1.submatrix<6, 7, 4, 5>();

	int read = static1.get<7, 0>();
	static1.get<1, 0>() = read;

	assertAll(0, sq);
	assertAll(0, static1);
	assertAll(0, transposed);
	assertAll(0, submatrix);
	assertAll(0, rect);
	assertAll(0, vector);

	test(sq);
	test(static1);
	test(transposed);
	test(submatrix);
	test(rect);
	test(vector);

	assertAll(0, sq);
	assertAll(0, static1);
	assertAll(0, transposed);
	assertAll(0, submatrix);
	assertAll(0, rect);
	assertAll(0, vector);

	std::cout << "Testing multiplication" << std::endl;

	testMultiplicationAndAddition();

	std::cout << "ALL TESTS PASSED" << std::endl;
	return 0;
}
