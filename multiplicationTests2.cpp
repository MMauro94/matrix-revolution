#include <iostream>
#include <vector>
#include <ctime>
#include <memory>
#include "Matrix.h"
#include "StaticSizeMatrix.h"

template<class MD>
void initializeCells(Matrix<int, MD> &m, const char *debugName, int rowMultiplier, int colMultiplier) {
	m.setDebugName(debugName);
	for (unsigned row = 0; row < m.rows(); ++row) {
		for (unsigned col = 0; col < m.columns(); ++col) {
			m(row, col) = row * rowMultiplier + col * colMultiplier;
		}
	}
}

int main() {
	StaticSizeMatrix<2, 5, int> mA;
	initializeCells(mA, "A", 12, 5);
	StaticSizeMatrix<2, 5, int> mB;
	initializeCells(mB, "B", 7, 13);
	StaticSizeMatrix<5, 20, int> mC;
	initializeCells(mC, "C", 3, 8);
	StaticSizeMatrix<5, 20, int> mD;
	initializeCells(mD, "D", 4, 7);
	StaticSizeMatrix<20, 2, int> mE;
	initializeCells(mE, "E", 8, 9);
	StaticSizeMatrix<20, 2, int> mF;
	initializeCells(mF, "F", 5, 11);
	StaticSizeMatrix<2, 10, int> mG;
	initializeCells(mG, "G", 54, 11);
	StaticSizeMatrix<10, 2, int> mH;
	initializeCells(mH, "H", 66, 11);

	/*std::cout << "Matrix A" << std::endl;
	mA.print("%2d");

	std::cout << "Matrix B" << std::endl;
	mB.print("%2d");

	std::cout << "Matrix C" << std::endl;
	mC.print("%2d");

	std::cout << "Matrix D" << std::endl;
	mD.print("%2d");

	std::cout << "Matrix E" << std::endl;
	mE.print("%2d");

	std::cout << "Matrix F" << std::endl;
	mF.print("%2d");*/

	auto multiplication = ((mA + mB) * (mC + mD) * (mE + mF)) + (mG * mH);

	clock_t begin = clock();
	int first = multiplication.getData().get(0, 0);
	clock_t end = clock();
	std::cout << "The first element is " << first << ", and it took " << double(end - begin) / CLOCKS_PER_SEC << " seconds" << std::endl;

	multiplication.getData().printDebugTree();

	std::cout << std::endl << std::endl;
	std::cout << "((A + B) * (C + D) * (E + F)) + (G * H)" << std::endl;
	multiplication.print("%2d");

	return 0;
}
