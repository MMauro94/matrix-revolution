#include <iostream>
#include <vector>
#include <memory>
#include "Matrix.h"
#include "StaticSizeMatrix.h"

template<typename T, class MD>
void initializeCells(Matrix<T, MD> &m, T rowMultiplier, T colMultiplier) {
	for (unsigned row = 0; row < m.rows(); ++row) {
		for (unsigned col = 0; col < m.columns(); ++col) {
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

template<typename T, class MD>
void assertAll(T expected, const Matrix<T, MD> &m) {
	for (unsigned r = 0; r < m.rows(); ++r) {
		for (unsigned c = 0; c < m.columns(); ++c) {
			assert<T>(expected, m(r, c));
		}
	}
}

template<typename T, class MD1, class MD2>
void assertEquals(const Matrix<T, MD1> &m1, const Matrix<T, MD2> &m2) {
	assert(m1.columns(), m2.columns());
	assert(m1.rows(), m2.rows());

	for (unsigned r = 0; r < m1.rows(); ++r) {
		for (unsigned c = 0; c < m1.columns(); ++c) {
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

template<typename T, class MD>
void dirtify(Matrix<T, MD> m) {
	for (unsigned r = 0; r < m.rows(); r++) {
		for (unsigned c = 0; c < m.columns(); c++) {
			m(r, c) = 18;
		}
	}
}

template<typename T, class MD>
void dirtify2(Matrix<T, MD> &m) {
	for (unsigned r = 0; r < m.rows(); r++) {
		for (unsigned c = 0; c < m.columns(); c++) {
			m(r, c) = 18;
		}
	}
}

template<typename T, class MD>
void test(Matrix<T, MD> &m) {
	assertAll(0, m);
	dirtify<T, MD>(m);//Testing deep copy
	assertAll(0, m);
	dirtify2<T, MD>(m);//Testing move
	assertAll(18, m);
	//TEST ASSIGNMENTS
	for (int k = 0; k < 10; ++k) {
		for (unsigned r = 0; r < m.rows(); ++r) {
			for (unsigned c = 0; c < m.columns(); ++c) {
				m(r, c) = k;
				assert<T>(k, m(r, c));
			}
		}
	}

	//ASSIGN VALUES FROM [1..columns*rows] in row major order
	int k = 0;
	for (unsigned r = 0; r < m.rows(); ++r) {
		for (unsigned c = 0; c < m.columns(); ++c) {
			k++;
			m(r, c) = k;
			assert<T>(k, m(r, c));
		}
	}

	//TEST ROW-MAJOR ITERATOR
	testIterator(m.beginRowMajor(), m.endRowMajor(), m.size());


	//ASSIGN VALUES FROM [1..columns*rows] in column major order
	k = 0;
	for (unsigned c = 0; c < m.columns(); ++c) {
		for (unsigned r = 0; r < m.rows(); ++r) {
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

	//TEST DIAGONAL
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

		for (unsigned r = 0; r < d.rows(); ++r) {
			for (unsigned c = 0; c < d.columns(); ++c) {
				if (r == c) {
					assert<T>(m(r, 0), d(r, c));
				} else {
					assert<T>(0, d(r, c));
				}
			}
		}
	}

	//TEST SUBMATRIX
	if (m.rows() >= 4 && m.columns() >= 4) {
		auto sm = m.submatrix(2, 3, 2, 1);
		assert<T>(m(2, 3), sm(0, 0));
		assert<T>(m(3, 3), sm(1, 0));

		sm(0, 0) = 123;
		assert<T>(123, m(2, 3));
	}

	assertEquals(m, m.submatrix(0, 0, m.rows(), m.columns()));

	//Cleaning the matrix
	for (unsigned r = 0; r < m.rows(); r++) {
		for (unsigned c = 0; c < m.columns(); c++) {
			m(r, c) = 0;
		}
	}
}


void testMultiplicationAndAddition() {
	StaticSizeMatrix<4, 3, int> m1;
	Matrix<int> m2(4, 3);
	Matrix<int> m3(3, 5);
	StaticSizeMatrix<5, 6, int> m4;
	initializeCells(m1, 12, 5);
	initializeCells<int>(m2, 11, 7);
	initializeCells(m3, 7, 13);
	initializeCells(m4, 8, 2);
	auto sum1a = m1 + m2;
	auto sum1b = m2 + m1;
	auto sum1c = m1 + m1;
	auto multiplication1c = m3 * m4;
	const auto multiplication1a = m1 * m3;

	//m1.print("%02d");
	//m2.print("%02d");
	//multiplication1a.print("%04d");
	//m3.print("%02d");
	//m4.print("%02d");
	//multiplication1c.print("%04d");
	assert<int>(92, sum1c.get<3, 2>());
	assert<int>(93, sum1a(3, 2));
	assert<int>(93, sum1b(3, 2));
	//assertEquals(sum1a, sum1b);
	assert<int>(7327, multiplication1a(3, 4));
	assert<int>(5040, multiplication1c(2, 2));


	StaticSizeMatrix<42, 7, int> m5;
	StaticSizeMatrix<7, 28, int> m6;
	StaticSizeMatrix<28, 14, int> m7;
	initializeCells(m5, 12, 5);
	initializeCells(m6, 7, 13);
	initializeCells(m7, 3, 5);
	auto multiplication2 = m5 * m6 * m7;
	assert<int>(35015890, multiplication2.get<0, 1>());
	assert<int>(110059096, multiplication2.get<1, 10>());
	assert<int>(145932388, multiplication2.get<3, 4>());
	//multiplication2.print("%04d");
}


void testBasicStuff() {
	Matrix<int> sq(10, 10);
	StaticSizeMatrix<10, 10, int> sqStatic;
	StaticSizeMatrix<15, 20, int> static1;
	StaticSizeMatrix<20, 3, int> static2;
	Matrix<int> rect(5, 10);
	Matrix<int> vector(10, 1);
	auto multiplied1 = (sq * sq.transpose()).copy();
	StaticSizeMatrix<5, 5, int> multiplied2 = (sqStatic * sqStatic.transpose()).diagonal().diagonalMatrix().submatrix<5, 5>(1, 1).copy();

	auto transposed = static1.transpose();
	auto submatrix = static1.submatrix<6, 7, 4, 5>();
	auto submatrix2 = rect.submatrix<4, 5>(1, 4);

	int read = static1.get<7, 0>();
	static1.get<1, 0>() = read;

	test<int>(submatrix);
	test<int>(submatrix2);
	test<int>(sq);
	test<int>(static1);
	test<int>(transposed);
	test<int>(multiplied1);
	test<int>(multiplied2);
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
