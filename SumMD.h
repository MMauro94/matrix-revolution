//
// Created by MMarco on 22/02/2019.
//

#ifndef MATRIX_SUMMD_H
#define MATRIX_SUMMD_H

#include "OptimizableMD.h"

/**
 * Implementation of <code>MatrixData</code> that exposes the sum of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class SumMDa : public BiMatrixWrapper<T, MD1, MD2> {
	public:
		SumMDa(MD1 left, MD2 right) : BiMatrixWrapper<T, MD1, MD2>(left, right, left.rows(), left.columns()) {
			if (left.rows() != right.rows() || left.columns() != right.columns()) {
				Utils::error("Sum between incompatible sizes");
			}
		}

		MATERIALIZE_IMPL

		SumMDa<T, MD1, MD2> copy() const {
			return SumMDa<T, MD1, MD2>(this->left.copy(), this->right.copy());
		}

	private:

		T doGet(unsigned row, unsigned col) const {
			return this->left.get(row, col) + this->right.get(row, col);
		}
};

/**
 * Implementation of <code>MatrixData</code> that exposes the sum of a list given matrices
 * @tparam T type of the data
 */
template<typename T, class MD>
class MultiSumMD : public MultiMatrixWrapper<T, MD> {
	public:
		explicit MultiSumMD(std::deque<MD> wrapped) : MultiMatrixWrapper<T, MD>(wrapped, wrapped[0].rows(), wrapped[0].columns()) {
			for (auto &m:wrapped) {
				if (m.rows() != this->rows() || m.columns() != this->columns()) {
					Utils::error("Sum between incompatible sizes");
				}
			}
		}

		MATERIALIZE_IMPL

		MultiSumMD<T, MD> copy() const {
			return MultiSumMD<T, MD>(this->copyWrapped());
		}

	private:
		T doGet(unsigned row, unsigned col) const {
			T ret = 0;
			for (auto it = this->wrapped.begin(); it < this->wrapped.end(); it++) {
				ret += it->get(row, col);
			}
			return ret;
		}
};
#endif //MATRIX_SUMMD_H
