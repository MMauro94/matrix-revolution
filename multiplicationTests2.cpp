#include <iostream>
#include <vector>
#include <ctime>
#include <memory>
#include "Matrix.h"
#include "StaticSizeMatrix.h"

template<class MD>
void initializeCells(Matrix<long, MD> &m, long rowMultiplier, long colMultiplier) {
	for (unsigned row = 0; row < m.rows(); ++row) {
		for (unsigned col = 0; col < m.columns(); ++col) {
			m(row, col) = row * rowMultiplier + col * colMultiplier;
		}
	}
}

int main() {
	StaticSizeMatrix<400, 5000, long> mA;
	initializeCells(mA, 12, 5);
	StaticSizeMatrix<400, 5000, long> mB;
	initializeCells(mB, 7, 13);

	StaticSizeMatrix<5000, 2000, long> mC;
	initializeCells(mC, 3, 8);
	StaticSizeMatrix<5000, 2000, long> mD;
	initializeCells(mD, 4, 7);

	StaticSizeMatrix<2000, 400, long> mE;
	initializeCells(mE, 8, 9);
	StaticSizeMatrix<2000, 400, long> mF;
	initializeCells(mF, 5, 11);

	StaticSizeMatrix<400, 1001, long> mG;
	initializeCells(mG, 54, 11);
	StaticSizeMatrix<1001, 400, long> mH;
	initializeCells(mH, 66, 11);

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


	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	long first = multiplication(0, 0);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "The first element is " << first << ", and it took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.0 << " seconds" << std::endl;

	std::cout << "((A + B) * (C + D) * (E + F)) + (G * H)" << std::endl;
	std::cout << std::endl << std::endl;
	//multiplication.getData().printTree();
	std::cout << std::endl << std::endl;
	std::cout << "Oks" << std::endl;

	return 0;
}
