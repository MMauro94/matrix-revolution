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
				: OptimizableMatrixData<T, VectorMatrixData<T>>("Materializer", rows, columns, true), rowOffset(rowOffset), colOffset(colOffset), wrapped(wrapped) {
		}

	protected:
		void doOptimization(ThreadPool *threadPool) override {
			threadPool->add([=] {
				auto materialized = this->wrapped->materialize(rowOffset, colOffset, this->rows(), this->columns());
				this->setOptimized(std::make_unique<VectorMatrixData<T>>(materialized));
			});
		}

		virtual std::vector<const MatrixData<T> *> getChildren() const {
			return {this->wrapped};
		}
};

#endif //MATRIX_MATRIXMATERIALIZER_H
