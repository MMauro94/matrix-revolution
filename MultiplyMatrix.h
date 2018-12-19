//
// Created by MMarco on 19/12/2018.
//

#ifndef MATRIX_MULTIPLYMATRIX_H
#define MATRIX_MULTIPLYMATRIX_H

#include "MatrixData.h"
#include <deque>

template<typename T>
class VirtualMultiplyMatrix;

/**
 * Implementation of <code>MatrixData</code> that exposes the multiplication of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class MultiplyMatrix : public MatrixData<T> {

	private:
		MD1 left;
		MD2 right;
		mutable VirtualMultiplyMatrix<T> *optimizedMatrix = NULL;
		mutable bool optimized = false;
		/**
		 * Needed to keep the pointers!
		 * Using a deque, since it allows members without copy/move constructors
		 */
		mutable std::deque<VirtualMultiplyMatrix<T>> computedMultiplications;

		template<typename U, class MD3, class MD4> friend
		class MultiplyMatrix;

	public:

		MultiplyMatrix(MD1 left, MD2 right) :
				MatrixData<T>(left.rows(), right.columns()), left(left), right(right) {
		}

		MultiplyMatrix(const MultiplyMatrix<T, MD1, MD2> &another) :
				MatrixData<T>(another.rows(), another.columns()), left(another.left), right(another.right) {
			//The cached data is not passed around, since it will be too difficult to copy
		}

		MultiplyMatrix(MultiplyMatrix<T, MD1, MD2> &&another) noexcept :
				MatrixData<T>(another.rows(), another.columns()), left(another.left), right(another.right) {
			//The cached data is not passed around, since it will be too difficult to copy
		}

		T get(unsigned int row, unsigned int col) const {
			this->optimizeIfNecessary();
			if (this->optimizedMatrix == NULL) {
				T ret = 0;
				for (unsigned j = 0; j < this->left.columns(); j++) {
					ret += this->left.get(row, j) * this->right.virtualGet(j, col);
				}
				return ret;
			} else {
				return this->optimizedMatrix->get(row, col);
			}
		}

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

		MultiplyMatrix<T, MD1, MD2> copy() const {
			return MultiplyMatrix<T, MD1, MD2>(this->left.copy(), this->right.copy());
		}

	private:

		/**
		 * This method optimizes the multiplication if the multiplication chain involves more than three matrix.
		 */
		void optimizeIfNecessary() const {
			if (!this->optimized) {
				//Step 1: getting the chain of multiplications to perform
				std::vector<MatrixData<T> *> multiplicationChain;
				const_cast<MultiplyMatrix<T, MD1, MD2> *>
				(this)->addToMultiplicationChain(multiplicationChain);

				//Step 2: If I multiply only two matrices, no optimization is performed, since it will be faster to just access the data
				if (multiplicationChain.size() > 2) {
					//Step 3: execute the multiplications in an efficient order, until a single matrix is left
					while (multiplicationChain.size() > 1) {
						//Step 3a: find the multiplication that reduces the multiplication the most
						unsigned bestIndex = 0;
						for (unsigned i = 0; i < multiplicationChain.size() - 1; i++) {
							if (multiplicationChain[i]->columns() > multiplicationChain[bestIndex]->columns()) {
								bestIndex = i;
							}
						}
						MatrixData<T> *leftMatrix = multiplicationChain[bestIndex];
						MatrixData<T> *rightMatrix = multiplicationChain[bestIndex + 1];

						//Step 3b: replacing the two matrices in the chain with the computed product
						//Creating the multiplication inside computedMultiplications
						computedMultiplications.emplace_back(leftMatrix, rightMatrix);
						//Replacing the two matrices with the multiplication
						multiplicationChain.erase(multiplicationChain.begin() + bestIndex + 1);
						multiplicationChain[bestIndex] = &computedMultiplications.back();
					}

					//Step 4: the last item in the chain is the multiplication result.
					// It is a VirtualMultiplyMatrix, since it comes from computeMultiplication().
					this->optimizedMatrix = static_cast<VirtualMultiplyMatrix<T> *>(multiplicationChain[0]);
				}
				this->optimized = true;
			}
		}

	protected:

		/**
		 * Adds it child to the multiplication chain
		 */
		void addToMultiplicationChain(std::vector<MatrixData<T> *> &multiplicationChain) {
			if (this->optimizedMatrix != NULL) {
				multiplicationChain.push_back(this->optimizedMatrix);
			} else {
				this->left.addToMultiplicationChain(multiplicationChain);
				this->right.addToMultiplicationChain(multiplicationChain);
			};
		}
};

/**
 * This class is used only internally on MultiplyMatrix, to keet the optimal operation tree.
 */
template<typename T>
class VirtualMultiplyMatrix : public MatrixData<T> {

	private:
		MatrixData<T> *left;
		MatrixData<T> *right;
		mutable std::unique_ptr<VectorMatrixData<T>> optimizedMatrix = NULL;
		mutable bool optimized = false;

	public:

		VirtualMultiplyMatrix(MatrixData<T> *left, MatrixData<T> *right) :
				MatrixData<T>(left->rows(), right->columns()), left(left), right(right) {
		}

		//No copy constructor
		VirtualMultiplyMatrix(const VirtualMultiplyMatrix<T> &another) = delete;

		//No move constructor
		VirtualMultiplyMatrix(VirtualMultiplyMatrix<T> &&another) noexcept = delete;

		/**
		 * The first time this matrix is accessed, the date is kept in a VectorMatrixData
		 */
		T get(unsigned int row, unsigned int col) const {
			this->optimizeIfNecessary();
			return this->optimizedMatrix->get(row, col);
		}

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

	private:

		template<typename U>
		static std::unique_ptr<VectorMatrixData<U>> computeMultiplication(MatrixData<U> *left, MatrixData<U> *right) {
			std::unique_ptr<VectorMatrixData<U>> ret = std::make_unique<VectorMatrixData<U>>(left->rows(), right->columns());
			for (unsigned int r = 0; r < ret->rows(); r++) {
				for (unsigned int c = 0; c < ret->columns(); c++) {
					U cell = 0;
					for (unsigned j = 0; j < left->columns(); j++) {
						cell += left->virtualGet(r, j) * right->virtualGet(j, c);
					}
					ret->set(r, c, cell);
				}
			}
			return ret;
		}

		void optimizeIfNecessary() const {
			if (!this->optimized) {
				this->optimizedMatrix = computeMultiplication(this->left, this->right);
				this->optimized = true;
			}
		}
};

#endif //MATRIX_MULTIPLYMATRIX_H
