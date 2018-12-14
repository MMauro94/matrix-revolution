#include <iostream>
#include <vector>
#include <memory>
#include "Matrix.h"
#include "StaticSizeMatrix.h"

template<typename T>
void initializeCells(Matrix<T> &m, T rowMultiplier, T colMultiplier) {
	for (unsigned int row = 0; row < m.rows(); ++row) {
		for (unsigned int col = 0; col < m.columns(); ++col) {
			m(row, col) = row * rowMultiplier + col * colMultiplier;
		}
	}
}

template<typename T>
void assert(T expected, T actual) {
	if (expected != actual) {
		std::cout << "ERROR: expected " << expected << ", got " << actual << std::endl;
		exit(1);
	}
}

template<typename T>
void assertAll(T expected, Matrix<T> m) {
	for (unsigned int r = 0; r < m.rows(); ++r) {
		for (unsigned int c = 0; c < m.columns(); ++c) {
			assert<T>(expected, m(r, c));
		}
	}
}

template<typename T>
void assertEquals(Matrix<T> m1, Matrix<T> m2) {
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

template<typename T>
void test(Matrix<int> &m) {
	//TEST ASSIGNMENTS
	for (int k = 0; k < 10; ++k) {
		for (unsigned int r = 0; r < m.rows(); ++r) {
			for (unsigned int c = 0; c < m.columns(); ++c) {
				m(r, c) = k;
				assert<T>(k, m(r, c));
			}
		}
	}

	//ASSIGN VALUES FROM [1..columns*rows] in row major order
	int k = 0;
	for (unsigned int r = 0; r < m.rows(); ++r) {
		for (unsigned int c = 0; c < m.columns(); ++c) {
			k++;
			m(r, c) = k;
			assert<T>(k, m(r, c));
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
			assert<T>(k, m(r, c));
		}
	}

	//TEST COLUMN-MAJOR ITERATOR
	testIterator(m.beginColumnMajor(), m.endColumnMajor(), m.size());


	//TEST TRANSPOSE
	if (m.rows() >= 4 && m.columns() >= 3) {
		m.transpose()(2, 3) = 76;
		assert<T>(76, m(3, 2));
	}
	assertEquals(m, m.transpose().transpose());

	//TESTI DIAGONAL
	if (m.isSquared()) {
		m.diagonal()(2, 0) = 54;
		assert<T>(54, m(2, 2));

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
					assert<T>(0, d(r, c));
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
		assert<T>(123, m(2, 3));
	}

	assertEquals(m, m.submatrix(0, 0, m.rows(), m.columns()));
}


void testMultiplicationAndAddition() {
	StaticSizeMatrix<4, 3, int> m1;
	const StaticSizeMatrix<4, 3, int> m1Plus10 = m1 + 10;
	Matrix<double> m2(4, 3);
	Matrix<int> m3(3, 5);
	StaticSizeMatrix<5, 6, int> m4;
	initializeCells(m1, 12, 5);
	initializeCells<double>(m2, 11, 7);
	initializeCells(m3, 7, 13);
	initializeCells(m4, 8, 2);
	const Matrix<double> sum1a = m1 + m2;
	const Matrix<double> sum1b = m2 + m1;
	const StaticSizeMatrix<4, 3, int> sum1c = m1 + m1;
	const Matrix<int> multiplication1a = m1 * m3;
	const Matrix<double> multiplication1b = m2 * m3.readOnlyCast<double>();
	const Matrix<int> multiplication1c = m3 * m4;

	//m1.print("%02d");
	//m2.print("%02d");
	//multiplication1a.print("%04d");
	//m3.print("%02d");
	//m4.print("%02d");
	//multiplication1c.print("%04d");
	assert(10, m1Plus10(3, 2));
	assert(92, sum1c.get<3, 2>());
	assert<double>(93, sum1a(3, 2));
	assert<double>(93, sum1b(3, 2));
	assertEquals(sum1a, sum1b);
	assert(7327, multiplication1a(3, 4));
	assert<double>(7178, multiplication1b(3, 4));
	assert(5040, multiplication1c(2, 2));


	StaticSizeMatrix<42, 7, int> m5;
	StaticSizeMatrix<7, 28, int> m6;
	StaticSizeMatrix<28, 14, int> m7;
	initializeCells(m5, 12, 5);
	initializeCells(m6, 7, 13);
	initializeCells(m7, 3, 5);
	const StaticSizeMatrix<42, 14, int> multiplication2 = m5 * m6 * m7;
	assert(35015890, multiplication2.get<0, 1>());
	assert(110059096, multiplication2.get<1, 10>());
	assert(145932388, multiplication2.get<3, 4>());
	//multiplication2.print("%04d");
}


void testBasicStuff() {
	Matrix<int> sq(10, 10);
	StaticSizeMatrix<15, 20, int> static1;
	StaticSizeMatrix<20, 3, int> static2;
	Matrix<int> rect(5, 10);
	Matrix<int> vector(10, 1);

	StaticSizeMatrix<20, 15, int> transposed = static1.transpose();
	StaticSizeMatrix<4, 5, int> submatrix = static1.submatrix<6, 7, 4, 5>();

	int read = static1.get<7, 0>();
	static1.get<1, 0>() = read;

	assertAll(0, sq);
	assertAll(0, static1);
	assertAll(0, transposed);
	assertAll(0, submatrix);
	assertAll(0, rect);
	assertAll(0, vector);

	test<int>(sq);
	test<int>(static1);
	test<int>(transposed);
	test<int>(submatrix);
	test<int>(rect);
	test<int>(vector);
}

int main() {
	/*
	 * MAIN THAT PERFORMS SOME TESTS
	 * IN CASE OF FAILURE, THE PROGRAM EXITS
	 */

	std::cout << "START TEST" << std::endl;

	std::cout << "Testing basic stuff" << std::endl;
	testBasicStuff();

	std::cout << "Testing multiplication" << std::endl;
	testMultiplicationAndAddition();

	std::cout << "ALL TESTS PASSED" << std::endl;
	return 0;
}
