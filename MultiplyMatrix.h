//
// Created by MMarco on 19/12/2018.
//

#ifndef MATRIX_MULTIPLYMATRIX_H
#define MATRIX_MULTIPLYMATRIX_H

#include "MatrixData.h"

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
		mutable std::shared_ptr<VirtualMultiplyMatrix<T>> optimizedMatrix;
		mutable bool optimized = false;
		mutable std::vector<VirtualMultiplyMatrix<T>> computedMultiplications;//Needed to keep the pointers!

		template<typename U, class MD3, class MD4> friend
		class MultiplyMatrix;

	public:

		MultiplyMatrix(MD1 left, MD2 right) :
				MatrixData<T>(left.rows(), right.columns()), left(left), right(right) {
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
		void optimizeIfNecessary() const {
			if (!this->optimized) {
				//Step 1: getting the chain of multiplications to perform
				std::vector<MatrixData<T> *> multiplicationChain;
				const_cast<MultiplyMatrix<T, MD1, MD2> *>
				(this)->addToMultiplicationChain(multiplicationChain);
				if (multiplicationChain.size() > 2) {
					//Step 2: If I multiply only two matrices, no optimization is performed, since it will be faster to just access the data
					computedMultiplications.reserve(multiplicationChain.size() - 1);

					//Step 3: execute the multiplications in an efficient order, until a single matrix data is left
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
						//Creating the multiplication
						const VirtualMultiplyMatrix<T> &multiplication = VirtualMultiplyMatrix<T>(leftMatrix, rightMatrix);
						computedMultiplications.push_back(multiplication);
						//Replacing the two matrices with the multiplication
						multiplicationChain.erase(multiplicationChain.begin() + bestIndex + 1);
						multiplicationChain[bestIndex] = &computedMultiplications.back();
					}

					//Step 4: the last item in the chain is the multiplication result.
					// It is a VirtualMultiplyMatrix, since it comes from computeMultiplication().
					VirtualMultiplyMatrix<T> optimizedVector = *dynamic_cast<VirtualMultiplyMatrix<T> *>(multiplicationChain[0]);
					this->optimizedMatrix = std::make_shared<VirtualMultiplyMatrix<T >>(optimizedVector);
				}
				this->optimized = true;
			}
		}

	protected:

		void addToMultiplicationChain(std::vector<MatrixData<T> *> &multiplicationChain) {
			if (this->optimizedMatrix.get() != NULL) {
				multiplicationChain.push_back(this->optimizedMatrix.get());
			} else {
				this->left.addToMultiplicationChain(multiplicationChain);
				this->right.addToMultiplicationChain(multiplicationChain);
			};
		}
};


template<typename T>
class VirtualMultiplyMatrix : public MatrixData<T> {

	private:
		MatrixData<T> *left;
		MatrixData<T> *right;
		mutable std::shared_ptr<VectorMatrixData<T>> optimizedMatrix = NULL;
		mutable bool optimized = false;

	public:

		VirtualMultiplyMatrix(MatrixData<T> *left, MatrixData<T> *right) :
				MatrixData<T>(left->rows(), right->columns()), left(left), right(right) {
		}

		T get(unsigned int row, unsigned int col) const {
			this->optimizeIfNecessary();
			if (this->optimizedMatrix == NULL) {
				throw "Illegal state";
			}
			return this->optimizedMatrix->get(row, col);
		}

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

	private:

		template<typename T>
		static std::shared_ptr<VectorMatrixData<T>> computeMultiplication(MatrixData<T> *left, MatrixData<T> *right) {
			std::shared_ptr<VectorMatrixData<T>> ret = std::make_shared<VectorMatrixData<T>>(left->rows(), right->columns());
			for (unsigned int r = 0; r < ret->rows(); r++) {
				for (unsigned int c = 0; c < ret->columns(); c++) {
					T cell = 0;
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
