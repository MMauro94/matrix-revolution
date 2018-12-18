//
// Created by MMarco on 28/11/2018.
//
#include<iostream>

#include"Matrix.h"
#include <chrono>


int main() {
	using namespace std;
	auto start = std::chrono::system_clock::now();
	for (unsigned int iteration = 0; iteration < 10; ++iteration) {
		auto startInner = std::chrono::system_clock::now();
		Matrix<int> m1(4000, 5000);
		for (unsigned int i = 0; i < m1.rows(); ++i) {
			for (unsigned int j = 0; j < m1.columns(); ++j) {
				m1(i, j) = i * 10 + j;
			}
		}

		auto m2 = m1.transpose().submatrix(1, 1, 3000, 3000).transpose().submatrix(1, 1, 1000, 1000);

		for (unsigned int i = 0; i < m2.rows(); ++i) {
			for (unsigned int j = 0; j < m2.columns(); ++j) {
				m2(i, j) = i * 10 + j;
			}
		}


		auto m3 = m1.transpose();
		for (unsigned int i = 0; i < m3.rows(); ++i) {
			for (unsigned int j = 0; j < m3.columns(); ++j) {
				m3(i, j) = i * 10 + j;
			}
		}


		auto endInner = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = endInner - startInner;
		std::cout << iteration << ") " << diff.count() << " s\n";
	}

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << "Total time " << diff.count() << " s\n";
}

