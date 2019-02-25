//
// Created by MMarco on 22/02/2019.
//

#ifndef MATRIX_MATERIALIZERMD_H
#define MATRIX_MATERIALIZERMD_H

#include "MatrixData.h"
#include "OptimizableMD.h"

template<typename T>
class MaterializerMD : public OptimizableMD<T, VectorMatrixData<T>> {
	private:
		const MatrixData<T> *wrapped;
		unsigned rowOffset, colOffset;

	public:
		MaterializerMD(const MatrixData<T> *wrapped, unsigned rowOffset, unsigned colOffset, unsigned rows, unsigned columns)
				: OptimizableMD<T, VectorMatrixData<T>>(rows, columns), rowOffset(rowOffset), colOffset(colOffset), wrapped(wrapped) {
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

#endif //MATRIX_MATERIALIZERMD_H
