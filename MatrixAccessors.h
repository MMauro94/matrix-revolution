//
// Created by MMarco on 03/11/2018.
//

#ifndef MATRIX_MATRIXACCESSORS_H
#define MATRIX_MATRIXACCESSORS_H


#include "MatrixData.h"

template<typename T>
class MatrixCell {
	private :
		MatrixData<T> *data;
		int row, column;
	public:
		MatrixCell(MatrixData<T> *data, int row, int column) : data(data), row(row), column(column) {}

		MatrixCell &operator=(T const &obj) {
			data->set(row, column, obj);
			return *this;
		}

		operator const T &() const {
			return data->get(row, column);
		}
};

template<typename T>
class BaseMatrixRow {
	protected:
		MatrixData<T> *data;
		int row;
	public:
		BaseMatrixRow(MatrixData<T> *data, int row) : data(data), row(row) {}

};

template<typename T>
class ReadOnlyMatrixRow : BaseMatrixRow<T> {
	public:
		ReadOnlyMatrixRow(MatrixData<T> *data, int row) : BaseMatrixRow(data, row) {}

		const MatrixCell<T> operator[](int index) const {
			return MatrixCell<T>(data, row, index);
		}
};

template<typename T>
class MatrixRow : BaseMatrixRow<T> {
	public:
		MatrixRow(MatrixData<T> *data, int row) : BaseMatrixRow(data, row) {
		}

		MatrixCell<T> operator[](int index) const {
			return MatrixCell<T>(data, row, index);
		}
};

#endif //MATRIX_MATRIXACCESSORS_H
