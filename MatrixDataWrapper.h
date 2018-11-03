//
// Created by molin on 02/11/2018.
//

#ifndef MATRIX_MATRIXDATAWRAPPER_H
#define MATRIX_MATRIXDATAWRAPPER_H

#include "MatrixData.h"

template<typename T>
class MatrixDataWrapper : public MatrixData<T> {
	protected:
		MatrixData<T> *data;

		MatrixDataWrapper(MatrixData<T> *data, int rows, int columns) : MatrixData(rows, columns), data(data) {
			data->incrementRefCount();
		}

	public:
		MatrixDataWrapper(MatrixDataWrapper<T> &other) {
			//TODO: è giusto che il copy-constructor non copy?
			data = other.data;
			data->incrementRefCount();
		}

		MatrixDataWrapper(MatrixDataWrapper<T> &&other) noexcept {
			data = other.data;
			data->incrementRefCount();
		}

		virtual ~MatrixDataWrapper() {
			if (data->decrementRefCount()) {
				std::cout << "Deleting data from wrapper";
				delete data;
			}
		}

};

template<typename T>
class BiMatrixDataWrapper : public MatrixData<T> {
	protected:
		MatrixData<T> *dataA;
		MatrixData<T> *dataB;

		BiMatrixDataWrapper(MatrixData<T> *dataA, MatrixData<T> *dataB, int rows, int columns)
				: MatrixData(rows, columns), dataA(dataA), dataB(dataB) {
			dataA->incrementRefCount();
			dataB->incrementRefCount();
		}

	public:
		BiMatrixDataWrapper(BiMatrixDataWrapper<T> &other) {
			dataA = other.dataA;
			dataB = other.dataB;
			dataA->incrementRefCount();
			dataB->incrementRefCount();
		}

		BiMatrixDataWrapper(BiMatrixDataWrapper<T> &&other) noexcept {
			dataA = other.dataA;
			dataB = other.dataB;
			dataA->incrementRefCount();
			dataB->incrementRefCount();
		}

		virtual ~BiMatrixDataWrapper() {
			if (dataA->decrementRefCount()) {
				delete dataA;
			}
			if (dataB->decrementRefCount()) {
				delete dataB;
			}
		}

};

#endif //MATRIX_MATRIXDATAWRAPPER_H
