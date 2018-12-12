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

	const StaticSizeMatrix<2, 2, int> &multiplication = mA * mB * mC;

	std::cout << "Matrix AxBxC" << std::endl;
	multiplication.print("%2d");
	return 0;
}
