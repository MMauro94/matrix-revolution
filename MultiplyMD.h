//
// Created by MMarco on 25/02/2019.
//

#ifndef MATRIX_MULTIPLYMD_H
#define MATRIX_MULTIPLYMD_H

#include "MatrixData.h"
#include "OptimizableMD.h"
#include "MaterializerMD.h"
#include <deque>
#include <cmath>
#include <chrono>
#include <thread>

//Using long, blocks of 128k will be 128x128
unsigned OPTIMAL_BLOCK_SIZE = 128 * 1024;

template<typename T>
class OptimizedMultiplyMD;

template<typename T>
class BaseMultiplyMD;

/**
 * Implementation of <code>MatrixData</code> that exposes the multiplication of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class MultiplyMD : public OptimizableMD<T, OptimizedMultiplyMD<T>> {

	private:
		/**
		 * Needed to keep the pointers!
		 * Using a deque, since it allows members without copy/move constructors
		 */
		mutable std::deque<OptimizedMultiplyMD<T>> nodeReferences;
		MD1 left;
		MD2 right;

		template<typename U, class MD3, class MD4> friend
		class MultiplyMD;

	public:

		MultiplyMD(MD1 left, MD2 right) : OptimizableMD<T, OptimizedMultiplyMD<T>>(left.rows(), right.columns()), left(left), right(right) {
			if (left.columns() != right.rows()) {
				Utils::error("Multiplication should be performed on compatible matrices");
			}
		}

		MultiplyMD(const MultiplyMD<T, MD1, MD2> &another) : OptimizableMD<T, OptimizedMultiplyMD<T>>(another), left(another.left), right(another.right) {
		}

		MultiplyMD(MultiplyMD<T, MD1, MD2> &&another) noexcept : OptimizableMD<T, OptimizedMultiplyMD<T>>(another), left(another.left), right(another.right) {
		}

		virtual ~MultiplyMD() {
			//This is done in order to don't have threads that uses this object (or something inside nodeReferences or left or right), after I'm being destroying
			this->virtualWaitOptimized();
		}

		std::vector<const MatrixData<T> *> virtualGetChildren() const override {
			return {&this->left, &this->right};
		}

		MultiplyMD<T, MD1, MD2> copy() const {
			return MultiplyMD<T, MD1, MD2>(this->left.copy(), this->right.copy());
		}

	protected:

		/**
		 * Adds it child to the multiplication chain
		 */
		void addToMultiplicationChain(std::vector<const MatrixData<T> *> &multiplicationChain) const {
			this->left.addToMultiplicationChain(multiplicationChain);
			this->right.addToMultiplicationChain(multiplicationChain);
		}

		/**
		 * This method optimizes the multiplication tree, by doing first the multiplication that reduces the most
		 * the number of dimensions
		 */
		std::unique_ptr<OptimizedMultiplyMD<T>> virtualCreateOptimizedMatrix() const override {
			//Step 1: getting the chain of multiplications to perform
			std::vector<const MatrixData<T> *> multiplicationChain;
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
				const MatrixData<T> *leftMatrix = multiplicationChain[bestIndex];
				const MatrixData<T> *rightMatrix = multiplicationChain[bestIndex + 1];

				//Step 3b: replacing the two matrices in the chain with the computed product
				//Creating the multiplication inside nodeReferences
				nodeReferences.emplace_back(leftMatrix, rightMatrix);
				//Replacing the two matrices with the multiplication
				multiplicationChain.erase(multiplicationChain.begin() + bestIndex + 1);
				multiplicationChain[bestIndex] = &nodeReferences.back();
			}

			//Step 4: the last item in the chain is the multiplication result.
			// It is a OptimizedMultiplyMD, since it comes from nodeReferences.
			auto *optimized = static_cast<const OptimizedMultiplyMD<T> *>(multiplicationChain[0]);
			return std::make_unique<OptimizedMultiplyMD<T>>(*optimized);
		}
};

/**
 * This class is used only internally on MultiplyMD, to keep the optimal operation tree.
 */
template<typename T>
class OptimizedMultiplyMD : public OptimizableMD<T, ConcatenationMD<T, MultiSumMD<T, BaseMultiplyMD<T>>>> {
	private:
		const MatrixData<T> *left, *right;
	public:
		OptimizedMultiplyMD(const MatrixData<T> *left, const MatrixData<T> *right)
				: OptimizableMD<T, ConcatenationMD<T, MultiSumMD<T, BaseMultiplyMD<T>>>>(left->rows(), right->columns()),
				  left(left), right(right) {
		}

		OptimizedMultiplyMD(const OptimizedMultiplyMD<T> &another) :
				OptimizableMD<T, ConcatenationMD<T, MultiSumMD<T, BaseMultiplyMD<T>>>>(another),
				left(another.left), right(another.right) {
		}

		//No move constructor
		OptimizedMultiplyMD(OptimizedMultiplyMD<T> &&another) noexcept = delete;

		std::vector<const MatrixData<T> *> virtualGetChildren() const override {
			return {this->left, this->right};
		}

	protected:

		std::unique_ptr<ConcatenationMD<T, MultiSumMD<T, BaseMultiplyMD<T>>>> virtualCreateOptimizedMatrix() const override {
			auto optimalMultiplicationSize = (unsigned) sqrt(OPTIMAL_BLOCK_SIZE / (double) sizeof(T));

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
			std::deque<MultiSumMD<T, BaseMultiplyMD<T>>> resultingBlocks;
			for (unsigned r = 0; r < numberOfGridRowsA; r++) {
				for (unsigned c = 0; c < numberOfGridColsB; c++) {
					std::deque<BaseMultiplyMD<T>> toMultiply;
					for (unsigned k = 0; k < numberOfGridRowsB; k++) {
						toMultiply.emplace_back(blocksOfA[r * numberOfGridColsA + k], blocksOfB[k * numberOfGridColsB + c]);
					}
					resultingBlocks.emplace_back(toMultiply);
				}
			}
			//optimized is LARGER or equal to this matrix, but that's not a problem
			return std::make_unique<ConcatenationMD<T, MultiSumMD<T, BaseMultiplyMD<T>>>>(
					resultingBlocks, numberOfGridRowsA * rowsOfGridA, numberOfGridColsB * colsOfGridB
			);
		}

	private:

		std::vector<std::shared_ptr<ResizerMD<T, MaterializerMD<T>>>>
		divideInBlocks(const MatrixData<T> *matrix, unsigned numberOfGridRows, unsigned numberOfGridCols) const {
			//e.g. matrix is 202x302;
			//numberOfGridRows = 3
			// numberOfGridCols = 4
			unsigned rowsOfGrid = Utils::ceilDiv(matrix->rows(), numberOfGridRows);//e.g. 68
			unsigned colsOfGrid = Utils::ceilDiv(matrix->columns(), numberOfGridCols);//e.g. 76
			std::vector<std::shared_ptr<ResizerMD<T, MaterializerMD<T>>>> ret;
			for (unsigned r = 0; r < numberOfGridRows; r++) {
				for (unsigned c = 0; c < numberOfGridCols; c++) {
					unsigned blockRowStart = r * rowsOfGrid;//0, 68, 136
					unsigned blockRowEnd = std::min(((r + 1) * rowsOfGrid), matrix->rows());//68, 136, 202
					unsigned blockColStart = c * colsOfGrid;//0, 76, 152, 228
					unsigned blockColEnd = std::min(((c + 1) * colsOfGrid), matrix->columns());//76, 152, 228, 302
					unsigned int blockRows = blockRowEnd - blockRowStart;
					unsigned int blockCols = blockColEnd - blockColStart;

					MaterializerMD<T> block(matrix, blockRowStart, blockColStart, blockRows, blockCols);
					//I wrap the matrix in a ResizerMD to make sure every block is of the same size
					ret.push_back(std::make_shared<ResizerMD<T, MaterializerMD<T>>>(block, rowsOfGrid, colsOfGrid));
				}
			}
			return ret;
		}

};

template<typename T>
class BaseMultiplyMD : public OptimizableMD<T, VectorMatrixData<T>> {
	private:
		mutable std::shared_ptr<ResizerMD<T, MaterializerMD<T>>> left, right;
	public:
		BaseMultiplyMD(std::shared_ptr<ResizerMD<T, MaterializerMD<T>>> left, std::shared_ptr<ResizerMD<T, MaterializerMD<T>>> right)
				: OptimizableMD<T, VectorMatrixData<T>>(left->rows(), right->columns()), left(left), right(right) {
		}

		//I cannot return left or right, since I could leak an object that will be deleted in the future
		std::vector<const MatrixData<T> *> virtualGetChildren() const override {
			//return {this->left.get(), this->right.get()};
			return std::vector<const MatrixData<T> *>();
		}

	protected:

		std::unique_ptr<VectorMatrixData<T>> virtualCreateOptimizedMatrix() const override {
			//Since OptimizableMD doesn't call optimize on children automatically, I do it here
			this->left->optimize();
			this->right->optimize();

			//Keeping the references to left and right, to save some time when calling get()
			ResizerMD<T, MaterializerMD<T>> *ll = this->left.get();
			ResizerMD<T, MaterializerMD<T>> *rr = this->right.get();
			std::unique_ptr<VectorMatrixData<T>> ret = std::make_unique<VectorMatrixData<T>>(ll->rows(), rr->columns());
			for (unsigned int r = 0; r < ret->rows(); r++) {
				for (unsigned int c = 0; c < ret->columns(); c++) {
					T sum = 0;
					for (unsigned j = 0; j < ll->columns(); j++) {
						sum += ll->get(r, j) * rr->get(j, c);
					}
					ret->set(r, c, sum);
				}
			}

			//Freeing memory
			this->left.reset();
			this->right.reset();
			return ret;
		}
};

#endif //MATRIX_MULTIPLYMD_H
