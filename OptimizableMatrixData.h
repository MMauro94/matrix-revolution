//
// Created by MMarco on 12/02/2019.
//

#ifndef MATRIX_OPERATIONNODEMATRIXDATA_H
#define MATRIX_OPERATIONNODEMATRIXDATA_H

#include <deque>
#include <future>
#include "MatrixData.h"

template<typename T, class O>
class OptimizableMatrixData : public MatrixData<T> {
	protected:
		const std::string wrapName;
		mutable std::mutex optimizeMutex; //Mutex for the method optimize()
	private:
		mutable std::shared_future<std::unique_ptr<O>> optimized;
		mutable O *optimizedPointer = NULL;//Saving the pointer to optimized in order to skip accessing optimizzed through a future and a unique_ptr

	public:

		OptimizableMatrixData(const std::string wrapName, unsigned int rows, unsigned int columns) :
				MatrixData<T>(rows, columns), wrapName(wrapName) {
		}

		OptimizableMatrixData(const OptimizableMatrixData<T, O> &another) :
				MatrixData<T>(another.rows(), another.columns()), wrapName(another.wrapName) {
			//The cached data is not passed around, since it will be too difficult to copy
		}

		OptimizableMatrixData(OptimizableMatrixData<T, O> &&another) noexcept :
				MatrixData<T>(another.rows(), another.columns()), wrapName(another.wrapName) {
			//The cached data is not passed around, since it will be too difficult to move
		}

		void optimize() const {
			std::unique_lock<std::mutex> lock(this->optimizeMutex);
			if (!this->optimized.valid()) {
				this->optimized = std::async(std::launch::async, [=] {
					return this->doOptimization();
				}).share();
			}
		}

		T get(unsigned int row, unsigned int col) const {
			return getOptimized()->get(row, col);
		}

		MATERIALIZE_IMPL

		virtual const std::string getDebugName() const override {
			return this->wrapName;
		}

	protected:

		/**
		 * This method optimizes the multiplication if the multiplication chain involves more than three matrix.
		 */
		virtual std::unique_ptr<O> doOptimization() const = 0;

		O *getOptimized() const {
			if (!this->optimized.valid()) {//This "if" is outside to skip call if unnecessary
				this->optimize();
			}
			if (this->optimizedPointer == NULL) {
				//Waiting for optimized, it not already done
				optimizedPointer = optimized.get().get();
			}
			return optimizedPointer;
		}
};

#endif //MATRIX_OPERATIONNODEMATRIXDATA_H
