//
// Created by MMarco on 12/02/2019.
//

#ifndef MATRIX_OPERATIONNODEMATRIXDATA_H
#define MATRIX_OPERATIONNODEMATRIXDATA_H

#include <deque>
#include "MatrixData.h"

ThreadPool *GLOBAL_THREAD_POOL = (new ThreadPool(100))->start();

template<typename T, class O>
class OptimizableMatrixData : public MatrixData<T> {
	protected:
		const std::string wrapName;
		mutable std::mutex optimization_mutex;
		mutable std::condition_variable condition;
	private:
		const bool callOptimizeOnChildren;
		mutable std::shared_ptr<O> optimized = NULL;/*std::shared_ptr<O>(NULL, [=](O *opt) {
			//This is needed in order to donì't call delete on NULL
			if (opt != NULL) {
				delete opt;
			}
		});*/
		mutable bool isOptimizing = false;
		mutable bool alreadyWaitedOptimization = false;

	public:

		OptimizableMatrixData(const std::string wrapName, unsigned int rows, unsigned int columns, bool callOptimizeOnChildren) :
				MatrixData<T>(rows, columns), wrapName(wrapName), callOptimizeOnChildren(callOptimizeOnChildren) {
		}

		OptimizableMatrixData(const OptimizableMatrixData<T, O> &another) :
				MatrixData<T>(another.rows(), another.columns()), wrapName(another.wrapName), callOptimizeOnChildren(another.callOptimizeOnChildren) {
			//The cached data is not passed around, since it will be too difficult to copy
			if (this->optimized != NULL) {
				std::cout << "Warning: losing optimized matrix!\n";
			}
		}

		OptimizableMatrixData(OptimizableMatrixData<T, O> &&another) noexcept :
				MatrixData<T>(another.rows(), another.columns()), wrapName(another.wrapName), callOptimizeOnChildren(another.callOptimizeOnChildren) {
			//The cached data is not passed around, since it will be too difficult to move
			if (this->optimized != NULL) {
				std::cout << "Warning: losing optimized matrix!\n";
			}
		}

		void optimize(ThreadPool *threadPool) const override {
			if (this->optimized == NULL) {
				std::unique_lock<std::mutex> lock(this->optimization_mutex);
				if (!this->isOptimizing) {
					this->isOptimizing = true;
					if (this->callOptimizeOnChildren) {
						MatrixData<T>::optimize(threadPool);
					}
					const_cast<OptimizableMatrixData<T, O> *>(this)->doOptimization(threadPool);
				}
			}
		}

		void setOptimized(std::shared_ptr<O> optimized) const {
			if (this->optimized != NULL) {
				Utils::error("Matrix already optimized!");
			} else if (optimized == NULL) {
				Utils::error("Optimized cannot be NULL!");
			} else if (optimized->rows() < this->rows() || optimized->columns() < this->columns()) {
				Utils::error("The optimized matrix should be bigger!");
			}
			this->optimized = optimized;
			this->condition.notify_all();
			this->optimized->optimize(GLOBAL_THREAD_POOL);
		}

		T get(unsigned int row, unsigned int col) const {
			if (this->optimized == NULL) {//This if is outside to skip virtual call if unnecessary
				this->optimize(GLOBAL_THREAD_POOL);
			}
			if (!this->alreadyWaitedOptimization) {//This if is outside to skip virtual call if unnecessary
				this->waitOptimized();
			}
			return this->optimized->get(row, col);
		}

		MATERIALIZE_IMPL

		virtual const std::string getDebugName() const override {
			return this->wrapName;
		}

		void waitOptimized() const override {
			//Waiting for optimized
			if (!this->alreadyWaitedOptimization) {
				if (this->optimized == NULL) {
					std::unique_lock<std::mutex> lock(this->optimization_mutex);
					this->condition.wait(lock, [=] { return optimized != NULL; });
					if (this->optimized == NULL) {
						Utils::error("Not yet optimized");
					}
				}
				this->optimized->waitOptimized();
				if (this->callOptimizeOnChildren) {
					MatrixData<T>::waitOptimized();
				}
				this->alreadyWaitedOptimization = true;
			}
		}

	protected:

		/**
		 * This method optimizes the multiplication if the multiplication chain involves more than three matrix.
		 */
		virtual void doOptimization(ThreadPool *threadPool) = 0;

		O *optOptimized() const {
			return this->optimized.get();
		}

		O *getOptimized() const {
			this->optimize(GLOBAL_THREAD_POOL);
			this->waitOptimized();
			return this->optOptimized();
		}
};

#endif //MATRIX_OPERATIONNODEMATRIXDATA_H
