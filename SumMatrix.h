//
// Created by MMarco on 12/02/2019.
//

#ifndef MATRIX_SUMMATRIX_H
#define MATRIX_SUMMATRIX_H

#include "OptimizableMatrixData.h"

template<typename T, class MD1, class MD2>
class BaseSumMatrix;

/**
 * Implementation of <code>MatrixData</code> that exposes the sum of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class SumMatrix : public BiMatrixWrapper<T, MD1, MD2> {
	public:
		SumMatrix(MD1 left, MD2 right) : BiMatrixWrapper<T, MD1, MD2>("+", left, right, left.rows(), left.columns()) {
		}

		T get(unsigned row, unsigned col) const {
			return this->left.get(row, col) + this->right.get(row, col);
		}

		COMMON_MATRIX_DATA_METHODS

		SumMatrix<T, MD1, MD2> copy() const {
			return SumMatrix<T, MD1, MD2>(this->left.copy(), this->right.copy());
		}
};

/**
 * Implementation of <code>MatrixData</code> that exposes the sum of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD>
class MultiSumMatrix : public MultiMatrixWrapper<T, MD> {
	public:
		explicit MultiSumMatrix(std::deque<MD> wrapped) : MultiMatrixWrapper<T, MD>("+", wrapped, wrapped[0].rows(), wrapped[0].columns()) {
		}

		T get(unsigned row, unsigned col) const {
			T ret = 0;
			for (auto it = this->wrapped.begin(); it < this->wrapped.end(); it++) {
				ret += it->get(row, col);
			}
			return ret;
		}

		COMMON_MATRIX_DATA_METHODS

		MultiSumMatrix<T, MD> copy() const {
			return MultiSumMatrix<T, MD>(this->copyWrapped());
		}
};

#endif //MATRIX_SUMMATRIX_H
