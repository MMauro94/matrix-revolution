//
// Created by MMarco on 19/12/2018.
//

#ifndef MATRIX_MULTIPLYMATRIX_H
#define MATRIX_MULTIPLYMATRIX_H

#include "MatrixData.h"
#include "OptimizableMatrixData.h"
#include "MatrixMaterializer.h"
#include <deque>
#include <chrono>
#include <thread>

unsigned OPTIMAL_MULTIPLICATION_SIZE = 150;

template<typename T>
class OptimizedMultiplyMatrix;

template<typename T>
class BaseMultiplyMatrix;

/**
 * Implementation of <code>MatrixData</code> that exposes the multiplication of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class MultiplyMatrix : public OptimizableMatrixData<T, OptimizedMultiplyMatrix<T>> {

	private:
		/**
		 * Needed to keep the pointers!
		 * Using a deque, since it allows members without copy/move constructors
		 */
		mutable std::deque<OptimizedMultiplyMatrix<T>> nodeReferences;
		MD1 left;
		MD2 right;

		template<typename U, class MD3, class MD4> friend
		class MultiplyMatrix;

	public:

		MultiplyMatrix(MD1 left, MD2 right) : OptimizableMatrixData<T, OptimizedMultiplyMatrix<T>>("Unoptimized multiplication", left.rows(), right.columns()),
											  left(left),
											  right(right) {
			if (left.columns() != right.rows()) {
				Utils::error("Multiplication should be performed on compatible matrices");
			}
		}

		MultiplyMatrix(const MultiplyMatrix<T, MD1, MD2> &another) : OptimizableMatrixData<T, OptimizedMultiplyMatrix<T>>(another), left(another.left), right(another.right) {
		}

		MultiplyMatrix(MultiplyMatrix<T, MD1, MD2> &&another) noexcept : OptimizableMatrixData<T, OptimizedMultiplyMatrix<T>>(another), left(another.left), right(another.right) {
		}

		/*void printDebugTree(const std::string &prefix, bool isLeft) const override {
			//I print the optimized tree, not the optimized one
			this->doPrintDebugTree(prefix, isLeft, {this->getOptimized()});
		};*/

		virtual std::vector<const MatrixData<T> *> getChildren() const {
			return {&this->left, &this->right};
		}


		MultiplyMatrix<T, MD1, MD2> copy() const {
			return MultiplyMatrix<T, MD1, MD2>(this->left.copy(), this->right.copy());
		}

	protected:

		/**
		 * Adds it child to the multiplication chain
		 */
		void addToMultiplicationChain(std::vector<MatrixData<T> *> &multiplicationChain) {
			this->left.addToMultiplicationChain(multiplicationChain);
			this->right.addToMultiplicationChain(multiplicationChain);
		}

		/**
		 * This method optimizes the multiplication tree, by doing first the multiplication that reduces the most
		 * the number of dimensions
		 */
		std::unique_ptr<OptimizedMultiplyMatrix<T>> doOptimization() override {
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
			// It is a OptimizedMultiplyMatrix, since it comes from nodeReferences.
			auto *optimized = static_cast<const OptimizedMultiplyMatrix<T> *>(multiplicationChain[0]);
			//TODO: make better
			return std::make_unique<OptimizedMultiplyMatrix<T>>(*optimized);
		}
};

/**
 * This class is used only internally on MultiplyMatrix, to keep the optimal operation tree.
 */
template<typename T>
class OptimizedMultiplyMatrix : public OptimizableMatrixData<T, MatrixConcatenation<T, MultiSumMatrix<T, BaseMultiplyMatrix<T>>>> {
	private:
		MatrixData<T> *left, *right;
	public:
		OptimizedMultiplyMatrix(MatrixData<T> *left, MatrixData<T> *right)
				: OptimizableMatrixData<T, MatrixConcatenation<T, MultiSumMatrix<T, BaseMultiplyMatrix<T>>>>("Optimized multiplication", left->rows(), right->columns()),
				  left(left),
				  right(right) {
		}

		OptimizedMultiplyMatrix(const OptimizedMultiplyMatrix<T> &another) :
				OptimizableMatrixData<T, MatrixConcatenation<T, MultiSumMatrix<T, BaseMultiplyMatrix<T>>>>(another), left(another.left), right(another.right) {
		}

		//No move constructor
		OptimizedMultiplyMatrix(OptimizedMultiplyMatrix<T> &&another) noexcept = delete;

		virtual std::vector<const MatrixData<T> *> getChildren() const {
			return {this->left, this->right};
		}

	protected:

		std::unique_ptr<MatrixConcatenation<T, MultiSumMatrix<T, BaseMultiplyMatrix<T>>>> doOptimization() {
			//E.g. A Matrix 202x302 will be divided in 3x4 blocks, of size 68x76
			unsigned numberOfGridRowsA = Utils::ceilDiv(this->left->rows(), OPTIMAL_MULTIPLICATION_SIZE);//e.g. 3
			unsigned rowsOfGridA = Utils::ceilDiv(this->left->rows(), numberOfGridRowsA);//e.g. 68
			unsigned numberOfGridColsA = Utils::ceilDiv(this->left->columns(), OPTIMAL_MULTIPLICATION_SIZE);//e.g. 4
			unsigned colsOfGridA = Utils::ceilDiv(this->left->columns(), numberOfGridColsA);//e.g. 76
			//Now that I've decided the blocks of A, I can comute the blocks of B.
			//For example, if B is 302x404, it will be divided in 4x5 blocks of size 76x81
			unsigned numberOfGridRowsB = numberOfGridColsA;//4
			unsigned rowsOfGridB = colsOfGridA;//76
			unsigned numberOfGridColsB = Utils::ceilDiv(this->right->columns(), OPTIMAL_MULTIPLICATION_SIZE);// e.g. 5
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
			//optimized is LARGER or equal to this matrix, but that's not a problem
			return std::make_unique<MatrixConcatenation<T, MultiSumMatrix<T, BaseMultiplyMatrix<T>>>>(resultingBlocks, numberOfGridRowsA * rowsOfGridA,
																									  numberOfGridColsB * colsOfGridB);
		}

	private:

		std::vector<std::shared_ptr<MatrixResizer<T, MatrixMaterializer<T>>>>
		divideInBlocks(MatrixData<T> *matrix, unsigned numberOfGridRows, unsigned numberOfGridCols) {
			//e.g. matrix is 202x302;
			//numberOfGridRows = 3
			// numberOfGridCols = 4
			unsigned rowsOfGrid = Utils::ceilDiv(matrix->rows(), numberOfGridRows);//e.g. 68
			unsigned colsOfGrid = Utils::ceilDiv(matrix->columns(), numberOfGridCols);//e.g. 76
			std::vector<std::shared_ptr<MatrixResizer<T, MatrixMaterializer<T>>>> ret;
			for (unsigned r = 0; r < numberOfGridRows; r++) {
				for (unsigned c = 0; c < numberOfGridCols; c++) {
					unsigned blockRowStart = r * rowsOfGrid;//0, 68, 136
					unsigned blockRowEnd = std::min(((r + 1) * rowsOfGrid), matrix->rows());//68, 136, 202
					unsigned blockColStart = c * colsOfGrid;//0, 76, 152, 228
					unsigned blockColEnd = std::min(((c + 1) * colsOfGrid), matrix->columns());//76, 152, 228, 302
					unsigned int blockRows = blockRowEnd - blockRowStart;
					unsigned int blockCols = blockColEnd - blockColStart;

					MatrixMaterializer<T> block(matrix, blockRowStart, blockColStart, blockRows, blockCols);
					//I wrap the matrix in a MatrixResizer to make sure every block is of the same size
					ret.push_back(std::make_shared<MatrixResizer<T, MatrixMaterializer<T>>>(block, rowsOfGrid, colsOfGrid));
				}
			}
			return ret;
		}

};

template<typename T>
class BaseMultiplyMatrix : public OptimizableMatrixData<T, VectorMatrixData<T>> {
	private:
		std::shared_ptr<MatrixResizer<T, MatrixMaterializer<T>>> left, right;
	public:
		BaseMultiplyMatrix(std::shared_ptr<MatrixResizer<T, MatrixMaterializer<T>>> left, std::shared_ptr<MatrixResizer<T, MatrixMaterializer<T>>> right)
				: OptimizableMatrixData<T, VectorMatrixData<T>>("Multiplication", left->rows(), right->columns()), left(left), right(right) {
		}

		virtual std::vector<const MatrixData<T> *> getChildren() const {
			return {this->left.get(), this->right.get()};
		}

	protected:

		std::unique_ptr<VectorMatrixData<T>> doOptimization() override {
			std::cout << "Multiplying...\n";
			std::unique_ptr<VectorMatrixData<T>> ret = std::make_unique<VectorMatrixData<T>>(this->left->rows(), this->right->columns());
			//ret->setDebugName("Multiplication result");
			for (unsigned int r = 0; r < ret->rows(); r++) {
				for (unsigned int c = 0; c < ret->columns(); c++) {
					T sum = 0;
					for (unsigned j = 0; j < this->left->columns(); j++) {
						sum += this->left->get(r, j) * this->right->get(j, c);
					}
					ret->set(r, c, sum);
				}
			}
			return ret;
		}
};

#endif //MATRIX_MULTIPLYMATRIX_H
