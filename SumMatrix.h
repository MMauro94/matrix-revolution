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
class SumMatrix : public OptimizableMatrixData<T, BaseSumMatrix<T, MD1, MD2>, MD1, MD2> {

	public:

		explicit SumMatrix(MD1 left, MD2 right) :
				OptimizableMatrixData<T, BaseSumMatrix<T, MD1, MD2>, MD1, MD2>(left, right, left.rows(), left.columns(), "+") {
		}

		SumMatrix(const SumMatrix<T, MD1, MD2> &another) : OptimizableMatrixData<T, BaseSumMatrix<T, MD1, MD2>, MD1, MD2>(another) {
		}

		SumMatrix(SumMatrix<T, MD1, MD2> &&another) noexcept : OptimizableMatrixData<T, BaseSumMatrix<T, MD1, MD2>, MD1, MD2>(another) {
		}

		void doOptimization(ThreadPool *threadPool) override {
			//Optimizing is O(1), no need to use the threadPool
			this->setOptimized(std::make_shared<BaseSumMatrix<T, MD1, MD2>>(this->left, this->right));
		}

		virtual const MatrixData<T> *getLeft() const {
			return &(this->left);
		}

		virtual const MatrixData<T> *getRight() const {
			return &(this->right);
		}
};

template<typename T, class MD1, class MD2>
class BaseSumMatrix : public MatrixData<T> {
	private:
		MD1 left;
		MD2 right;
	public:
		BaseSumMatrix(MD1 left, MD2 right) : MatrixData<T>(left.rows(), left.columns()), left(left), right(right) {
		}

		T get(unsigned row, unsigned col) const {
			return this->left.get(row, col) + this->right.get(row, col);
		}

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

		BaseSumMatrix<T, MD1, MD2> copy() const {
			return BaseSumMatrix<T, MD1, MD2>(this->left.copy(), this->right.copy());
		}

		void optimize(ThreadPool *threadPool) const override {
			this->left.optimize(threadPool);
			this->right.optimize(threadPool);
		}
};

#endif //MATRIX_SUMMATRIX_H
