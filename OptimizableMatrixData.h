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
		mutable std::mutex optimizeMutex; //Mutex for the method optimize()
	private:
		mutable std::shared_future<std::unique_ptr<O>> optimized;
		//I'm saving the pointer to optimized matrix in order to skip accessing it through a future and a unique_ptr
		mutable O *optimizedPointer = NULL;

	public:

		OptimizableMatrixData(unsigned int rows, unsigned int columns) :
				MatrixData<T>(rows, columns) {
		}

		OptimizableMatrixData(const OptimizableMatrixData<T, O> &another) :
				MatrixData<T>(another.rows(), another.columns()) {
			//The cached data is not passed around, since it will be too difficult to copy
			if (another.optimizedPointer != NULL) {
				std::cout << "Warning: cached data is lost!\n";
			}
		}

		OptimizableMatrixData(OptimizableMatrixData<T, O> &&another) noexcept :
				MatrixData<T>(another.rows(), another.columns()) {
			//The cached data is not passed around, since it will be too difficult to move
		}

		virtual ~OptimizableMatrixData() {
			if (this->optimized.valid()) {
				this->optimized.wait();
			}
		}

		MATERIALIZE_IMPL

		void virtualOptimize() const override {
			this->optimize();
		}

		void virtualWaitOptimized() const override {
			MatrixData<T>::virtualWaitOptimized();
			auto future = this->optimized;
			if (future.valid()) {
				future.wait();
			}
			if (this->optimizedPointer != NULL) {
				this->optimizedPointer->virtualWaitOptimized();
			}
		}

	private:
		T doGet(unsigned row, unsigned col) const {
			if (this->optimizedPointer == NULL) {
				//I'm saving the pointer to optimized matrix in order to skip accessing it through a future and a unique_ptr
				this->optimizedPointer = optimized.get().get();
			}
			return this->optimizedPointer->get(row, col);
		}


	protected:

		void optimize() const {
			std::unique_lock<std::mutex> lock(this->optimizeMutex);
			if (!this->optimizeHasBeenCalled) {
				this->optimized = std::async(std::launch::async, [=] {
					auto ptr = this->virtualCreateOptimizedMatrix();
					ptr->virtualOptimize();
					return ptr;
				}).share();
				this->optimizeHasBeenCalled = true;
			}
		}

		/**
		 * This method optimizes the multiplication if the multiplication chain involves more than three matrix.
		 */
		virtual std::unique_ptr<O> virtualCreateOptimizedMatrix() const = 0;
};

#endif //MATRIX_OPERATIONNODEMATRIXDATA_H
