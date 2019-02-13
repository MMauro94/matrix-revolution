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

template<typename T>
class BaseMultiplyMatrix;

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

		MultiplyMatrix(MD1 left, MD2 right) : OptimizableMatrixData<T, VirtualMultiplyMatrix<T>, MD1, MD2>("*", left, right, left.rows(),
																										   right.columns()) {
			if (left.columns() != right.rows()) {
				Utils::error("Multiplication should be performed on compatible matrices");
			}
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
 * This class is used only internally on MultiplyMatrix, to keep the optimal operation tree.
 */
template<typename T>
class VirtualMultiplyMatrix
		: public OptimizableMatrixData<T, MatrixResizer<T, MatrixConcatenation<T, MultiSumMatrix<T, BaseMultiplyMatrix<T>>>>, MatrixData<T> *, MatrixData<T> *> {
	public:
		VirtualMultiplyMatrix(MatrixData<T> *left, MatrixData<T> *right)
				: OptimizableMatrixData<T, MatrixResizer<T, MatrixConcatenation<T, MultiSumMatrix<T, BaseMultiplyMatrix<T>>>>, MatrixData<T> *, MatrixData<T> *>(
				"**",
				left,
				right,
				left->rows(),
				right->columns()) {
		}

		//No copy constructor
		//VirtualMultiplyMatrix(const VirtualMultiplyMatrix<T> &another) = delete;

		//No move constructor
		//VirtualMultiplyMatrix(VirtualMultiplyMatrix<T> &&another) noexcept = delete;


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
			threadPool->add([=] { multiply(threadPool); });
		}

	private:

		std::vector<MatrixResizer<T, VectorMatrixData<T>>>
		divideInBlocks(MatrixData<T> *matrix, unsigned numberOfGridRows, unsigned numberOfGridCols) {
			//e.g. matrix is 202x302;
			//numberOfGridRows = 3
			// numberOfGridCols = 4
			std::vector<MatrixResizer<T, VectorMatrixData<T>>> ret;
			unsigned rowsOfGrid = Utils::ceilDiv(matrix->rows(), numberOfGridRows);//e.g. 68
			unsigned colsOfGrid = Utils::ceilDiv(matrix->columns(), numberOfGridCols);//e.g. 76
			for (unsigned r = 0; r < numberOfGridRows; r++) {
				for (unsigned c = 0; c < numberOfGridCols; c++) {
					unsigned blockRowStart = r * rowsOfGrid;//0, 68, 136
					unsigned blockRowEnd = std::min(((r + 1) * rowsOfGrid), matrix->rows());//68, 136, 202
					unsigned blockColStart = c * colsOfGrid;//0, 76, 152, 228
					unsigned blockColEnd = std::min(((c + 1) * colsOfGrid), matrix->columns());//76, 152, 228, 302
					unsigned int blockRows = blockRowEnd - blockRowStart;
					unsigned int blockCols = blockColEnd - blockColStart;
					VectorMatrixData<T> block(blockRows, blockCols);
					const std::string debugName = matrix->getDebugName() + std::to_string(r) + "-" + std::to_string(c);
					block.setDebugName(debugName.c_str());
					for (unsigned rr = 0; rr < blockRows; rr++) {
						for (unsigned cc = 0; cc < blockCols; cc++) {
							block.set(rr, cc, matrix->virtualGet(blockRowStart + rr, blockColStart + cc));
						}
					}
					//I wrap the matrix in a MatrixResizer to make sure every block is of the sme size
					ret.emplace_back(block, rowsOfGrid, colsOfGrid);
				}
			}
			return ret;
		}

		void multiply(ThreadPool *threadPool) {
			std::cout << "Executing " + this->getDebugName() + "\n";

			//std::this_thread::sleep_for(std::chrono::milliseconds(10000));

			unsigned optimalMultiplicationSize = 100;
			//E.g. A Matrix 202x302 will be divided in 3x4 blocks, of size 68x76
			unsigned numberOfGridRowsA = Utils::ceilDiv(this->left->rows(), optimalMultiplicationSize);//e.g. 3
			unsigned rowsOfGridA = Utils::ceilDiv(this->left->rows(), numberOfGridRowsA);//e.g. 68
			unsigned numberOfGridColsA = Utils::ceilDiv(this->left->columns(), optimalMultiplicationSize);//e.g. 4
			unsigned colsOfGridA = Utils::ceilDiv(this->left->columns(), numberOfGridColsA);//e.g. 76
			//Now that I've decided the blocks of A, I can comute the blocks of B.
			//For example, if B is 302x404, it will be divided in 4x5 blocks of size 76x81
			unsigned numberOfGridRowsB = numberOfGridColsA;//4
			unsigned rowsOfGridB = colsOfGridA;//76
			unsigned numberOfGridColsB = Utils::ceilDiv(this->right->columns(), optimalMultiplicationSize);// e.g. 5
			unsigned colsOfGridB = Utils::ceilDiv(this->right->columns(), numberOfGridColsB);//e.g. 81
			//Now we divide the matrices in blocks
			auto blocksOfA = this->divideInBlocks(this->left, numberOfGridRowsA, numberOfGridColsA);
			auto blocksOfB = this->divideInBlocks(this->right, numberOfGridRowsB, numberOfGridColsB);
			//Now the result C is a matrix 202x404, and has 3x5 blocks of size 68x81

			//Cose da fare:
			//-Creare i blocchi di C
			//-Unire i blocchi di C
			std::deque<MultiSumMatrix<T, BaseMultiplyMatrix<T>>> resultingBlocks;
			for (unsigned r = 0; r < numberOfGridRowsA; r++) {
				for (unsigned c = 0; c < numberOfGridColsB; c++) {
					std::deque<BaseMultiplyMatrix<T>> toMultiply;
					for (unsigned k = 0; k < numberOfGridRowsB; k++) {
						toMultiply.emplace_back(blocksOfA[r * numberOfGridColsA + k], blocksOfB[k * numberOfGridColsB + c]);
					}
					resultingBlocks.emplace_back(toMultiply);
				}
			}

			std::cout << "Executed " + this->getDebugName() + "!!!\n";
			MatrixConcatenation<T, MultiSumMatrix<T, BaseMultiplyMatrix<T>>> multiplication(resultingBlocks, numberOfGridRowsA * rowsOfGridA,
																							numberOfGridColsB * colsOfGridB);
			this->setOptimized(
					std::make_shared<MatrixResizer<T, MatrixConcatenation<T, MultiSumMatrix<T, BaseMultiplyMatrix<T>>>>>(multiplication, this->rows(),
																														 this->columns()));
		}
};

template<typename T>
class BaseMultiplyMatrix
		: public OptimizableMatrixData<T, VectorMatrixData<T>, MatrixResizer<T, VectorMatrixData<T>>, MatrixResizer<T, VectorMatrixData<T>>> {
	public:
		BaseMultiplyMatrix(MatrixResizer<T, VectorMatrixData<T>> left, MatrixResizer<T, VectorMatrixData<T>> right)
				: OptimizableMatrixData<T, VectorMatrixData<T>, MatrixResizer<T, VectorMatrixData<T>>, MatrixResizer<T, VectorMatrixData<T>>>("***",
																																			  left,
																																			  right,
																																			  left.rows(),
																																			  right.columns()) {
		}

		//No copy constructor
		//BaseMultiplyMatrix(const BaseMultiplyMatrix<T> &another) = delete;

		//No move constructor
		//BaseMultiplyMatrix(BaseMultiplyMatrix<T> &&another) noexcept = delete;


	protected:

		virtual const MatrixData<T> *getLeft() const {
			return &(this->left);
		}

		virtual const MatrixData<T> *getRight() const {
			return &(this->right);
		}

		void doOptimization(ThreadPool *threadPool) override {
			threadPool->add([=] { doSerialOptimization(); });
		}

	private:

		void doSerialOptimization() {
			std::cout << "Executing " + this->getDebugName() + "\n";

			//std::this_thread::sleep_for(std::chrono::milliseconds(10000));

			std::shared_ptr<VectorMatrixData<T>> ret = std::make_shared<VectorMatrixData<T>>(this->left.rows(), this->right.columns());
			for (unsigned int r = 0; r < ret->rows(); r++) {
				for (unsigned int c = 0; c < ret->columns(); c++) {
					T cell = 0;
					for (unsigned j = 0; j < this->left.columns(); j++) {
						cell += this->left.get(r, j) * this->right.get(j, c);
					}
					ret->set(r, c, cell);
				}
			}

			std::cout << "Executed " + this->getDebugName() + "!!!\n";
			this->setOptimized(ret);
		}
};

#endif //MATRIX_MULTIPLYMATRIX_H
