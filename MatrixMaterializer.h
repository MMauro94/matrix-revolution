//
// Created by MMarco on 14/02/2019.
//

#ifndef MATRIX_MATRIXMATERIALIZER_H
#define MATRIX_MATRIXMATERIALIZER_H

#include "MatrixData.h"
#include "OptimizableMatrixData.h"

template<typename T>
class MatrixMaterializer : public OptimizableMatrixData<T, VectorMatrixData<T>> {
	private:
		MatrixData<T> *wrapped;
		unsigned rowOffset, colOffset;

	public:
		MatrixMaterializer(MatrixData<T> *wrapped, unsigned rowOffset, unsigned colOffset, unsigned rows, unsigned columns)
				: OptimizableMatrixData<T, VectorMatrixData<T>>("Materializer", rows, columns), rowOffset(rowOffset), colOffset(colOffset), wrapped(wrapped) {
		}

		//TODO: copy optimized version
		//No copy constructor
		//MatrixMaterializer(const MatrixMaterializer<T> &another) = delete;

		//No move constructor
		//MatrixMaterializer(MatrixMaterializer<T> &&another) noexcept = delete;


	protected:
		void doOptimization(ThreadPool *threadPool) override {
			this->wrapped->optimize(threadPool);
			threadPool->add([=] {
				//std::cout << "Materializing...\n";
				setOptimized(std::make_shared<VectorMatrixData<T>>(wrapped->materialize(rowOffset, colOffset, rows(), columns())));
			});
		}

		virtual void printDebugTree(const std::string &prefix, bool isLeft, bool hasVirtualBarrier) const override {
			//I print the un-materialized matrix, since the materialized one is boring
			MatrixData::printDebugTree(prefix, isLeft, hasVirtualBarrier);
			MatrixData::printDebugChildrenTree(prefix, isLeft, {this->wrapped}, false);
		};
};

#endif //MATRIX_MATRIXMATERIALIZER_H
