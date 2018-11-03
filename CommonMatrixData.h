//
// Created by MMarco on 03/11/2018.
//

#ifndef MATRIX_COMMONMATRIXDATA_H
#define MATRIX_COMMONMATRIXDATA_H

#include "MatrixDataWrapper.h"

/**
 * Exposes the diagonal elements of a matrix as a vector
 * @tparam T the type of the matrix
 */
template<typename T>
class DiagonalData : public MatrixDataWrapper<T> {
	public:

		explicit DiagonalData(MatrixData<T> *data) : MatrixDataWrapper(data, data->getColumns(), 1) {
		}

		T get(int row, int column) {
			return data->get(row, row);
		}

		void set(int row, int column, T obj) override {
			data->set(row, row, obj);
		}

};


/**
 * Exposed the element of the matrix inverting rows with columns
 * @tparam T the type of the matrix
 */
template<typename T>
class TransposedData : public MatrixDataWrapper<T> {
	public:

		explicit TransposedData(MatrixData<T> *data) : MatrixDataWrapper(data, data->getColumns(), data->getRows()) {
		}

		T get(int row, int column) {
			return data->get(column, row);
		}

		void set(int row, int column, T obj) override {
			data->set(column, row, obj);
		}

};

/**
 * Exposes a sub-matrix of a matrix.
 * @tparam T the type of the matrix
 */
template<typename T>
class SubmatrixData : public MatrixDataWrapper<T> {
	private:
		int rowOffset;
		int columnOffset;
	public:

		SubmatrixData(MatrixData<T> *data, int rowOffset, int rowCount, int columnOffset, int columnCount)
				: MatrixDataWrapper(data, rowCount, columnCount), rowOffset(rowOffset), columnOffset(columnOffset) {
		}

		T get(int row, int column) {
			return data->get(row + rowOffset, column + columnOffset);
		}

		void set(int row, int column, T obj) override {
			data->set(row + rowOffset, column + columnOffset, obj);
		}

};

/**
 * From the given vector of size N, creates a NxN diagonal matrix
 * @tparam T the type of the matrix
 */
template<typename T>
class ToDiagonalMatrixData : public MatrixDataWrapper<T> {
	public:

		explicit ToDiagonalMatrixData(MatrixData<T> *data) : MatrixDataWrapper(data, data->getRows(), data->getRows()) {
		}

		T get(int row, int column) override {
			if (row == column) {
				return data->get(row, row);
			} else {
				return 0;
			}
		}
};

/**
 * Exposes a matrix obtained by multiplying the two given matrix
 * @tparam T the type of the matrix
 */
template<typename T>
class MultiplyMatrixData : public BiMatrixDataWrapper<T> {
	public:
		MultiplyMatrixData(MatrixData<T> *dataA, MatrixData<T> *dataB)
				: BiMatrixDataWrapper(dataA, dataB, dataA->getRows(), dataB->getColumns()) {
		}

		T get(int row, int column) override {
			T ret = 0;
			for (int i = 0; i < dataA->getColumns(); i++) {
				ret += dataA->get(row, i) * dataB->get(i, column);
			}
			return ret;
		}
};

/**
 * Exposes a matrix obtained by multiplying a matrix by a constant factor
 * @tparam T the type of the matrix
 */
template<typename T>
class MultiplyByFactorData : public MatrixDataWrapper<T> {
	private:
		T factor;
	public:

		MultiplyByFactorData(MatrixData<T> *data, T factor)
				: MatrixDataWrapper(data, data->getColumns(), data->getRows()), factor(factor) {
		}

		T get(int row, int column) {
			return data->get(row, column) * factor;
		}

		void set(int row, int column, T obj) override {
			data->set(row, column, obj / factor);
		}
};

/**
 * Exposes a matrix obtained by adding the two given matrix
 * @tparam T the type of the matrix
 */
template<typename T>
class SumMatrixData : public BiMatrixDataWrapper<T> {
	public:
		SumMatrixData(MatrixData<T> *dataA, MatrixData<T> *dataB)
				: BiMatrixDataWrapper(dataA, dataB, dataA->getRows(), dataA->getColumns()) {
		}

		T get(int row, int column) override {
			return dataA->get(row, column) + dataB->get(row, column);
		}
};

/**
 * Exposes a matrix obtained by adding a matrix by a constant addend
 * @tparam T the type of the matrix
 */
template<typename T>
class SumByAddendData : public MatrixDataWrapper<T> {
	private:
		T factor;
	public:

		SumByAddendData(MatrixData<T> *data, T factor)
				: MatrixDataWrapper(data, data->getColumns(), data->getRows()), factor(factor) {
		}

		T get(int row, int column) {
			return data->get(row, column) + factor;
		}

		void set(int row, int column, T obj) override {
			data->set(row, column, obj - factor);
		}
};

/**
 * Exposes a matrix that differs from the given one by a specific value
 */
template<typename T>
class WithAChangedCellData : public MatrixDataWrapper<T> {
	private:
		T value;
		int row, column;
	public:
		WithAChangedCellData(MatrixData<T> *data, int row, int column, T value)
				: MatrixDataWrapper(data, data->getRows(), data->getColumns()), value(value), row(row), column(column) {}

		T get(int row, int column) override {
			if (row == this->row && column == this->column) {
				return value;
			} else {
				return data->get(row, column);
			}
		}

		void set(int row, int column, T obj) override {
			if (row == this->row && column == this->column) {
				value = obj;
			} else {
				data->set(row, column, obj);
			}
		}
};

#endif //MATRIX_COMMONMATRIXDATA_H
