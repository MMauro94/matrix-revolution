#ifndef MATRIX_ARRAYMATRIXDATA_H
#define MATRIX_ARRAYMATRIXDATA_H

#include "MatrixData.h"
#include <iostream>

template<typename T>
class ArrayMatrixData : public MatrixData<T> {
	private:
		T **data;

	public:
		ArrayMatrixData(int rows, int columns) : MatrixData<T>(rows, columns) {
			data = new T *[rows];
			for (int i = 0; i < rows; ++i) {
				this->data[i] = new T[columns];
			}
		}

		~ArrayMatrixData() {
			for (int i = 0; i < getRows(); ++i) {
				delete[](data[i]);
			}
			delete[](data);
		}

		T get(int row, int column) override {
			return data[row][column];
		}

		void set(int row, int column, T obj) override {
			data[row][column] = obj;
		}
};

#endif //MATRIX_ARRAYMATRIXDATA_H
