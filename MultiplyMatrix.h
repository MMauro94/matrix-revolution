//
// Created by MMarco on 19/12/2018.
//

#ifndef MATRIX_MULTIPLYMATRIX_H
#define MATRIX_MULTIPLYMATRIX_H

#include "MatrixData.h"
#include "OptimizableMatrixData.h"
#include <deque>
#include <chrono>
#include <thread>

template<typename T>
class VirtualMultiplyMatrix;

/**
 * Implementation of <code>MatrixData</code> that exposes the multiplication of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class MultiplyMatrix : public OptimizableMatrixData<T, VirtualMultiplyMatrix<T>, MD1, MD2> {

	private:
		/**
		 * Needed to keep the pointers!
		 * Using a deque, since it allows members without copy/move constructors
		 */
		mutable std::deque<VirtualMultiplyMatrix<T>> nodeReferences;

		template<typename U, class MD3, class MD4> friend
		class MultiplyMatrix;

	public:

		MultiplyMatrix(MD1 left, MD2 right) : OptimizableMatrixData<T, VirtualMultiplyMatrix<T>, MD1, MD2>(left, right, left.rows(),
																										   right.columns(), "**") {
		}

		MultiplyMatrix(const MultiplyMatrix<T, MD1, MD2> &another) : OptimizableMatrixData<T, VirtualMultiplyMatrix<T>, MD1, MD2>(another) {
		}

		MultiplyMatrix(MultiplyMatrix<T, MD1, MD2> &&another) noexcept : OptimizableMatrixData<T, VirtualMultiplyMatrix<T>, MD1, MD2>(another) {
		}

		virtual const MatrixData<T> *getLeft() const {
			return &(this->left);
		}

		virtual const MatrixData<T> *getRight() const {
			return &(this->right);
		}

		virtual void printDebugTree() const {
			OptimizableMatrixData::printDebugTree();
		}

		virtual void printDebugTree(const std::string &prefix, bool isLeft) const {
			this->waitOptimized();
			return this->optOptimized()->printDebugTree(prefix, isLeft);
		}

	protected:

		/**
		 * This method optimizes the multiplication if the multiplication chain involves more than three matrix.
		 */
		void doOptimization(ThreadPool *threadPool) override {
			threadPool->add([=] {
				doSerialOptimization();
			});
		}

		/**
		 * Adds it child to the multiplication chain
		 */
		void addToMultiplicationChain(std::vector<MatrixData<T> *> &multiplicationChain) {
			if (this->optOptimized() != NULL) {
				multiplicationChain.push_back(this->optOptimized());
			} else {
				this->left.addToMultiplicationChain(multiplicationChain);
				this->right.addToMultiplicationChain(multiplicationChain);
			};
		}

	private:

		void doSerialOptimization() {
			//std::cout << "Executing " + this->getDebugName(true) + "\n";


			//Step 1: getting the chain of multiplications to perform
			std::vector<MatrixData<T> *> multiplicationChain;
			addToMultiplicationChain(multiplicationChain);
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
				//Creating the multiplication inside nodeReferences
				nodeReferences.emplace_back(leftMatrix, rightMatrix);
				//Replacing the two matrices with the multiplication
				multiplicationChain.erase(multiplicationChain.begin() + bestIndex + 1);
				multiplicationChain[bestIndex] = &nodeReferences.back();
			}

			//Step 4: the last item in the chain is the multiplication result.
			// It is a VirtualMultiplyMatrix, since it comes from nodeReferences.
			VirtualMultiplyMatrix<T> *optimized = static_cast<VirtualMultiplyMatrix<T> *>(multiplicationChain[0]);

			//std::cout << "Executed " + this->getDebugName(true) + "!!!\n";
			setOptimized(std::shared_ptr<VirtualMultiplyMatrix<T>>(optimized));
		}
};

/**
 * This class is used only internally on MultiplyMatrix, to keet the optimal operation tree.
 */
template<typename T>
class VirtualMultiplyMatrix : public OptimizableMatrixData<T, VectorMatrixData<T>, MatrixData<T> *, MatrixData<T> *> {
	public:
		VirtualMultiplyMatrix(MatrixData<T> *left, MatrixData<T> *right)
				: OptimizableMatrixData<T, VectorMatrixData<T>, MatrixData<T> *, MatrixData<T> *>(left, right, left->rows(),
																								  right->columns(), "*") {
		}

		//No copy constructor
		VirtualMultiplyMatrix(const VirtualMultiplyMatrix<T> &another) = delete;

		//No move constructor
		VirtualMultiplyMatrix(VirtualMultiplyMatrix<T> &&another) noexcept = delete;


	protected:
		virtual const MatrixData<T> *getLeft() const {
			return this->left;
		}

		virtual const MatrixData<T> *getRight() const {
			return this->right;
		}

		void doOptimization(ThreadPool *threadPool) override {
			this->left->optimize(threadPool);
			this->right->optimize(threadPool);
			threadPool->add([=] { doSerialOptimization(); });
		}

	private:
		void doSerialOptimization() {
			std::cout << "Executing " + this->getDebugName(true) + "\n";

			std::this_thread::sleep_for(std::chrono::milliseconds(10000));

			std::shared_ptr<VectorMatrixData<T>> ret = std::make_shared<VectorMatrixData<T>>(this->left->rows(), this->right->columns());
			for (unsigned int r = 0; r < ret->rows(); r++) {
				for (unsigned int c = 0; c < ret->columns(); c++) {
					T cell = 0;
					for (unsigned j = 0; j < this->left->columns(); j++) {
						cell += this->left->virtualGet(r, j) * this->right->virtualGet(j, c);
					}
					ret->set(r, c, cell);
				}
			}

			std::cout << "Executed " + this->getDebugName(true) + "!!!\n";
			this->setOptimized(ret);
		}
};

#endif //MATRIX_MULTIPLYMATRIX_H
