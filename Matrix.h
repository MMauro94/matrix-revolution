#ifndef MATRIX_MATRIX_H
#define MATRIX_MATRIX_H

#include <ostream>
#include "MatrixData.h"
#include "ArrayMatrixData.h"
#include "matrix_exceptions.h"
#include "MatrixDataWrapper.h"
#include "CommonMatrixData.h"
#include "MatrixAccessors.h"

template<typename T>
class Matrix {

	private:
		MatrixData<T> *data;

		void checkSquare() const {
			if (!isSquare()) {
				throw new SquareMatrixRequiredException;
			}
		}

		void checkVector() const {
			if (!isVector()) {
				throw new VectorRequiredException;
			}
		}

		explicit Matrix(MatrixData<T> *data) : data(data) {
			data->incrementRefCount();
		}

	public:

		Matrix(const Matrix<T> &other) {
			//TODO: è giusto che il copy-constructor non copy?
			data = other.data;
			data->incrementRefCount();
		}

		Matrix(const Matrix<T> &&other) noexcept {
			data = other.data;
			data->incrementRefCount();
		}

		Matrix(int rows, int columns) : Matrix(new ArrayMatrixData<T>(rows, columns)) {}

		~Matrix() {
			if (data->decrementRefCount()) {
				delete (data);
			}
		}

		void clear() {
			initialize(0);
		}

		int rows() const {
			return data->getRows();
		}

		int columns() const {
			return data->getColumns();
		}

		const ReadOnlyMatrixRow<T> operator[](int index) const {
			return ReadOnlyMatrixRow<T>(data, index);
		}

		const MatrixRow<T> operator[](int index) {
			return MatrixRow<T>(data, index);
		}

		void initialize(T val) {
			for (int i = 0; i < rows(); ++i) {
				for (int j = 0; j < columns(); ++j) {
					(*this)[i][j] = val;
				}
			}
		}

		const Matrix<T> operator*(const Matrix<T> other) const {
			if (columns() != other.rows()) {
				throw new InvalidMatrixSizeException("Multiplication can only be performed between matrices NxM and MxO");
			}
			return Matrix<T>(new MultiplyMatrixData<T>(data, other.data));
		}

		const Matrix<T> operator*(T factor) const {
			return Matrix<T>(new MultiplyByFactorData<T>(data, factor));
		}

		const Matrix<T> operator/(T factor) const {
			return (*this) * (1 / factor);
		}

		const Matrix<T> operator+(const Matrix<T> addend) const {
			if (columns() != addend.columns() || rows() != addend.rows()) {
				throw new InvalidMatrixSizeException("Sum can only be performed between matrices of the same size");
			}
			return Matrix<T>(new SumMatrixData<T>(data, addend.data));
		}

		const Matrix<T> operator-(const Matrix<T> addend) const {
			T minusOne = -1;
			return (*this) + (addend * minusOne);
		}

		const Matrix<T> operator+(T addend) const {
			return Matrix<T>(new SumByAddendData<T>(data, addend));
		}

		const Matrix<T> operator-(T addend) const {
			return (*this) + (-addend);
		}

		const Matrix<T> with(int row, int column, T obj) const {
			return Matrix<T>(new WithAChangedCellData<T>(data, row, column, obj));
		}

		Matrix<T> with(int row, int column, T obj) {
			return Matrix<T>(new WithAChangedCellData<T>(data, row, column, obj));
		}

		bool isSquare() const {
			return data->getColumns() == data->getRows();
		}

		const Matrix<T> diagonal() const {
			checkSquare();
			return Matrix<T>(new DiagonalData<T>(data));
		}

		Matrix<T> diagonal() {
			checkSquare();
			return Matrix<T>(new DiagonalData<T>(data));
		}

		const Matrix<T> transposed() const {
			return Matrix<T>(new TransposedData<T>(data));
		}

		Matrix<T> transposed() {
			return Matrix<T>(new TransposedData<T>(data));
		}

		const Matrix<T> submatrix(int rowOffset, int rowCount, int columnOffset, int columnCount) const {
			return Matrix<T>(new SubmatrixData<T>(data, rowOffset, rowCount, columnOffset, columnCount));
		}

		Matrix<T> submatrix(int rowOffset, int rowCount, int columnOffset, int columnCount) {
			return Matrix<T>(new SubmatrixData<T>(data, rowOffset, rowCount, columnOffset, columnCount));
		}

		bool isVector() const {
			return columns() == 1;
		}

		bool isCovector() const {
			return rows() == 1;
		}

		const Matrix<T> toDiagonalMatrix() {
			checkVector();
			return Matrix<T>(new ToDiagonalMatrixData<T>(data));
		}

		const Matrix<T> column(int columnIndex) const {
			return submatrix(0, rows(), columnIndex, 1);
		}

		Matrix<T> column(int columnIndex) {
			return submatrix(0, rows(), columnIndex, 1);
		}

		const Matrix<T> row(int rowIndex) const {
			return submatrix(rowIndex, 1, 0, columns());
		}

		Matrix<T> row(int rowIndex) {
			return submatrix(rowIndex, 1, 0, columns());
		}

		Matrix<T> copy() {
			Matrix<T> ret(rows(), columns());
			for (int i = 0; i < rows(); i++) {
				for (int j = 0; j < columns(); j++) {
					ret[i][j] = data->get(i, j);
				}
			}
			return ret;
		}

		T scalarProduct(const Matrix<T> other) const {
			checkVector();
			other.checkVector();

			if (rows() != other.rows()) {
				throw new InvalidMatrixSizeException("Must have the same number of rows");
			}

			T ret = 0;
			for (int i = 0; i < rows(); ++i) {
				ret += (*this)[i][0] * other[i][0];
			}
			return ret;
		}

		template<typename T>
		friend std::ostream &operator<<(std::ostream &os, const Matrix<T> &matrix) {
			for (int i = 0; i < matrix.rows(); ++i) {
				for (int j = 0; j < matrix.columns(); ++j) {
					os << matrix[i][j] << "\t";
				}
				os << std::endl;
			}
			return os;
		}
};


#endif //MATRIX_MATRIX_H
