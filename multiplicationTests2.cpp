#include <iostream>
#include <vector>
#include <memory>
#include "Matrix.h"
#include "StaticSizeMatrix.h"

template<class MD>
void initializeCells(Matrix<int, MD> &m, int rowMultiplier, int colMultiplier) {
	for (unsigned row = 0; row < m.rows(); ++row) {
		for (unsigned col = 0; col < m.columns(); ++col) {
			m(row, col) = row * rowMultiplier + col * colMultiplier;
		}
	}
}

int main() {
	StaticSizeMatrix<2, 3, int> mA;
	StaticSizeMatrix<3, 5, int> mB;
	StaticSizeMatrix<5, 2, int> mC;
	initializeCells(mA, 12, 5);
	initializeCells(mB, 7, 13);
	initializeCells(mC, 3, 8);

	std::cout << "Matrix A" << std::endl;
	mA.print("%2d");

	std::cout << "Matrix B" << std::endl;
	mB.print("%2d");

	std::cout << "Matrix C" << std::endl;
	mC.print("%2d");

	auto multiplication = mA * mB * mC;

	multiplication.printForMultiplicationDebug();

	std::cout << "Matrix AxBxC" << std::endl;
	multiplication.print("%2d");
	return 0;
}
