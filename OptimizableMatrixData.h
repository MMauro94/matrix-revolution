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
			if (another.optimizedPointer != NULL) {
				std::cout << "Warning: cached data is lost!\n";
			}
		}

		virtual ~OptimizableMatrixData() {
			if (this->optimized.valid()) {
				this->optimized.wait();
			}
		}

		T get(unsigned int row, unsigned int col) const {
			return getOptimized()->get(row, col);
		}

		MATERIALIZE_IMPL

		void virtualOptimize() const override {
			this->optimize();
		}

		void optimize() const {
			//This "if" is also outside to skip creating the lock if unnecessary
			if (this->optimizedPointer == NULL && !this->optimized.valid()) {
				std::unique_lock<std::mutex> lock(this->optimizeMutex);
				if (!this->optimized.valid()) {
					this->optimized = std::async(std::launch::async, [=] {
						auto ptr = this->virtualCreateOptimizedMatrix();
						ptr->virtualOptimize();
						return ptr;
					}).share();
				}
			}
		}

		void virtualWaitOptimized() const override {
			MatrixData<T>::virtualWaitOptimized();
			auto future = this->optimized;
			if (future.valid()) {
				future.wait();
			}
			if(this->optimizedPointer != NULL){
				this->optimizedPointer->virtualWaitOptimized();
			}
		}

	protected:

		/**
		 * This method optimizes the multiplication if the multiplication chain involves more than three matrix.
		 */
		virtual std::unique_ptr<O> virtualCreateOptimizedMatrix() const = 0;

		O *getOptimized() const {
			this->optimize();//Calling the concrete implementation
			if (this->optimizedPointer == NULL) {
				//Waiting for optimized, it not already done.
				//I'm saving the pointer to optimized matrix in order to skip accessing it through a future and a unique_ptr
				optimizedPointer = optimized.get().get();
			}
			return optimizedPointer;
		}
};

#endif //MATRIX_OPERATIONNODEMATRIXDATA_H
