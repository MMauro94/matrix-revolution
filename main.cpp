#include <iostream>
#include "Matrix.h"

int main() {
    auto *m2 = new Matrix<2, 2, double>;

    Matrix<2, 2, double> m;
    m.clear();
    m[0][0] = 2;
    m[0][1] = 4;

    m[1][0] = 1;
    m[1][1] = 2;


    std::cout << "MATRIX:" << std::endl << m << std::endl << std::endl;

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


    /*
    const auto m2 = (m + 100) - 10;
    std::cout << m;
    std::cout << m2;*/

    return 0;
}