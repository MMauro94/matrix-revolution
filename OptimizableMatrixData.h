//
// Created by MMarco on 12/02/2019.
//

#ifndef MATRIX_OPERATIONNODEMATRIXDATA_H
#define MATRIX_OPERATIONNODEMATRIXDATA_H

#include <deque>
#include "MatrixData.h"

template<typename T>
class OperationNodeMatrixData;

ThreadPool *GLOBAL_THREAD_POOL = (new ThreadPool(10))->start();

template<typename T, class O, class MD1, class MD2>
class OptimizableMatrixData : public MatrixData<T> {
	protected:
		MD1 left;
		MD2 right;
		const char *operandName;
		mutable std::mutex optimization_mutex;
		mutable std::condition_variable condition;
	private:
		mutable std::shared_ptr<O> optimized = NULL;
		mutable bool isOptimizing = false;

	public:

		OptimizableMatrixData(MD1 left, MD2 right, unsigned int rows, unsigned int columns, const char *operandName) :
				MatrixData<T>(rows, columns), left(left), right(right), operandName(operandName) {
		}

		OptimizableMatrixData(const OptimizableMatrixData<T, O, MD1, MD2> &another) :
				MatrixData<T>(another.rows(), another.columns()), left(another.left), right(another.right), operandName(another.operandName) {
			//The cached data is not passed around, since it will be too difficult to copy
		}

		OptimizableMatrixData(OptimizableMatrixData<T, O, MD1, MD2> &&another) noexcept :
				MatrixData<T>(another.rows(), another.columns()), left(another.left), right(another.right), operandName(another.operandName) {
			//The cached data is not passed around, since it will be too difficult to move
		}

		void optimize(ThreadPool *threadPool) const {
			if (this->optimized == NULL) {
				std::unique_lock<std::mutex> lock(this->optimization_mutex);
				if (!this->isOptimizing) {
					this->isOptimizing = true;
					const_cast<OptimizableMatrixData<T, O, MD1, MD2> *>(this)->doOptimization(threadPool);
				}
			}
		}

		void setOptimized(std::shared_ptr<O> optimized) const {
			if (this->optimized != NULL) {
				Utils::error("Matrix already optimized!");
			}
			this->optimized = optimized;
			this->optimized->optimize(GLOBAL_THREAD_POOL);
			this->condition.notify_all();
		}

		virtual void waitOptimized() const {
			//Making sure I'm optimizing...
			this->optimize(GLOBAL_THREAD_POOL);
			//Waiting for optimized
			std::unique_lock<std::mutex> lock(this->optimization_mutex);
			this->condition.wait(lock, [=] { return optimized != NULL; });
			if (this->optimized == NULL) {
				Utils::error("Not yet optimized");
			}
		}

		O *optOptimized() const {
			return this->optimized.get();
		}

		T get(unsigned int row, unsigned int col) const {
			this->waitOptimized();
			return this->optimized->get(row, col);
		}

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

		OptimizableMatrixData<T, O, MD1, MD2> copy() const {
			return OptimizableMatrixData<T, O, MD1, MD2>(this->left.copy(), this->right.copy());
		}

		virtual std::string getDebugName(bool reversePolishNotation) const {
			std::string l = this->getLeft()->getDebugName(false);
			std::string r = this->getRight()->getDebugName(false);
			std::string on = this->operandName;
			if (reversePolishNotation) {
				return "(" + on + " (" + l + " " + r + ")";
			} else {
				return "(" + l + " " + on + " " + r + ")";
			}
		}

		virtual void printDebugTree() const {
			this->printDebugTree("", false);
		}

		virtual void printDebugTree(const std::string &prefix, bool isLeft) const {
			MatrixData::printDebugTree(prefix, isLeft);
			// enter the next tree level - left and right branch
			this->getLeft()->printDebugTree(prefix + (isLeft ? "|   " : "    "), true);
			this->getRight()->printDebugTree(prefix + (isLeft ? "|   " : "    "), false);
		}

	protected:

		/**
		 * This method optimizes the multiplication if the multiplication chain involves more than three matrix.
		 */
		virtual void doOptimization(ThreadPool *threadPool) = 0;

		virtual const MatrixData<T> *getLeft() const = 0;

		virtual const MatrixData<T> *getRight() const = 0;
};

#endif //MATRIX_OPERATIONNODEMATRIXDATA_H
