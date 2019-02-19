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
		const MatrixData<T> *wrapped;
		unsigned rowOffset, colOffset;

	public:
		MatrixMaterializer(const MatrixData<T> *wrapped, unsigned rowOffset, unsigned colOffset, unsigned rows, unsigned columns)
				: OptimizableMatrixData<T, VectorMatrixData<T>>(rows, columns), rowOffset(rowOffset), colOffset(colOffset), wrapped(wrapped) {
		}

	protected:
		std::unique_ptr<VectorMatrixData<T>> virtualCreateOptimizedMatrix() const override {
			auto materialized = this->wrapped->virtualMaterialize(rowOffset, colOffset, this->rows(), this->columns());
			return std::make_unique<VectorMatrixData<T>>(materialized);
		}

		std::vector<const MatrixData<T> *> virtualGetChildren() const override {
			return {this->wrapped};
		}
};

#endif //MATRIX_MATRIXMATERIALIZER_H
