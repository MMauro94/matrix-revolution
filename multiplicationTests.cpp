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
void assertEqual(const Matrix<T> &m1, const Matrix<T> &m2) {
	if (m1.rows() != m2.rows() || m1.columns() != m2.columns()) {
		std::cout << "ERROR: expected matrix of the same size" << std::endl;
		exit(1);
	}
	for (unsigned int r = 0; r < m1.rows(); ++r) {
		for (unsigned int c = 0; c < m2.columns(); ++c) {
			assert(m1(r, c), m2(r, c));
		}
	}
}

int main() {
	StaticSizeMatrix<4, 9, double> mAd;
	StaticSizeMatrix<4, 9, int> mA;
	StaticSizeMatrix<9, 7, int> mB;
	StaticSizeMatrix<7, 8, int> mC;
	StaticSizeMatrix<8, 2, int> mD;
	initializeCells(mA, 12, 5);
	initializeCells<double>(mAd, 12, 5);

	assertEqual(mA.readOnlyCast<double>(), mAd);

	initializeCells(mB, 7, 13);
	initializeCells(mC, 3, 8);
	initializeCells(mD, 2, 4);

	std::cout << "Matrix A" << std::endl;
	mA.print("%2d");
	std::cout << "-------------------------" << std::endl << std::endl;

	std::cout << "Matrix B" << std::endl;
	mB.print("%2d");
	std::cout << "-------------------------" << std::endl << std::endl;

	std::cout << "Matrix C" << std::endl;
	mC.print("%2d");
	std::cout << "-------------------------" << std::endl << std::endl;

	std::cout << "Matrix D" << std::endl;
	mD.print("%2d");
	std::cout << "-------------------------" << std::endl << std::endl;

	const StaticSizeMatrix<4, 7, int> &multiplicationAB = mA * mB;
	const StaticSizeMatrix<9, 8, int> &multiplicationBC = mB * mC;
	const StaticSizeMatrix<7, 2, int> &multiplicationCD = mC * mD;
	const StaticSizeMatrix<4, 8, int> &multiplicationABC = multiplicationAB * mC;
	const StaticSizeMatrix<4, 2, int> &multiplicationABCD = multiplicationAB * multiplicationCD;
	const StaticSizeMatrix<4, 2, int> &multiplicationABCD2 = mA * mB * mC * mD;

	const StaticSizeMatrix<4, 9, double> &sum = mA + mAd;

	std::cout << "Matrix AxBxC" << std::endl;
	multiplicationABC.print("%2d"); // This should perform AxB and (AxB)xC
	std::cout << "-------------------------" << std::endl << std::endl;

	std::cout << "Matrix AxB" << std::endl;
	multiplicationAB.print("%2d"); //This shouldn't perform any multiplication, since AxB is already computed
	std::cout << "-------------------------" << std::endl << std::endl;

	std::cout << "Matrix AxBxCxD" << std::endl;
	multiplicationABCD.print("%4d");//This should compute the multiplication (AxB)xC and (AxBxC)xD
	std::cout << "-------------------------" << std::endl << std::endl;

	std::cout << "Matrix AxBxCxD computed from scratch" << std::endl;
	multiplicationABCD2.print("%4d");//This should perform AxB, CxD and (AxB)x(CxD)
	std::cout << "-------------------------" << std::endl << std::endl;

	assert(954342816, multiplicationABCD.get<3, 1>());
	assertEqual(multiplicationABCD, multiplicationABCD2);
	std::cout << "ALL TESTS PASSED" << std::endl;
	return 0;
}
