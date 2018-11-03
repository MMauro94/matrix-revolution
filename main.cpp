#include <iostream>
#include "Matrix.h"
#include <vector>

/*
void memtest() {
    std::vector<Matrix<double>> vec;
    vec.reserve(10000);
    for (int i = 0; i < 10000; ++i) {
        vec.emplace_back(100, 100);
    }
    system("pause");
}*/

int main() {
	Matrix<double> m(4, 4);
	for (int i = 0; i < m.rows(); ++i) {
		for (int j = 0; j < m.columns(); ++j) {
			m[i][j] = i + j * 10;
			std::cout << m[i][j] << " ";
		}
		std::cout << std::endl;
	}

	m.diagonal()[2][0] = 987;
	m.column(1).row(2)[0][0] = 123;

	std::cout << "MATRIX:" << std::endl << m << std::endl << std::endl;

	std::cout << "DIAGONAL:" << std::endl << m.diagonal() << std::endl << std::endl;

	std::cout << "TRANSPOSED:" << std::endl << m.transposed() << std::endl << std::endl;

	std::cout << "SUBMATRIX:" << std::endl << m.submatrix(1, 2, 2, 2) << std::endl << std::endl;

	std::cout << "DIAGONAL MATRIX:" << std::endl << m.diagonal().toDiagonalMatrix() << std::endl << std::endl;

	std::cout << "COLUMN 2:" << std::endl << m.column(2) << std::endl << std::endl;

	std::cout << "ROW 1:" << std::endl << m.row(1) << std::endl << std::endl;

	std::cout << "MATRIX COPIED:" << std::endl << m.copy() << std::endl << std::endl;
	std::cout << "MATRIX WITH DIFFERENT FIRST:" << std::endl << m.with(0, 0, 12345) << std::endl << std::endl;
	std::cout << "MATRIX + 12:" << std::endl << m + 12 << std::endl << std::endl;
	std::cout << "MATRIX - 9:" << std::endl << m - 9 << std::endl << std::endl;
	std::cout << "MATRIX * 5:" << std::endl << m * 5 << std::endl << std::endl;
	std::cout << "MATRIX / 3:" << std::endl << m / 3 << std::endl << std::endl;


	std::cout << "M*M:" << std::endl << m * m << std::endl << std::endl;
	std::cout << "M+M:" << std::endl << m + m << std::endl << std::endl;
	std::cout << "M-M:" << std::endl << m - m << std::endl << std::endl;


/*
    //PROPERTIES
    std::cout << "IS TRIANGULAR:" << m.isTriangular() << std::endl;
    std::cout << "IS UPPER TRIANGULAR:" << m.isUpperTriangular() << std::endl;
    std::cout << "IS LOWER TRIANGULAR:" << m.isLowerTriangular() << std::endl;
    std::cout << "IS DIAGONAL:" << m.isDiagonal() << std::endl << std::endl;

    auto lu = m.luDecomposition();
    std::cout << "LU DECOMPOSITION:" << std::endl
              << "L:" << std::endl << lu.first << std::endl
              << "U:" << std::endl << lu.second << std::endl
              << "LU:" << std::endl << lu.first * lu.second << std::endl << std::endl;

    double determinant = m.determinant();
    std::cout << "DETERMINANT: " << determinant << std::endl;

*/
	/*
	const auto m2 = (m + 100) - 10;
	std::cout << m;
	std::cout << m2;*/

	return 0;
}