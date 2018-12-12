#ifndef MATRIX_MATRIX_H
#define MATRIX_MATRIX_H

#include <memory>
#include <string>
#include <iostream>
#include "MatrixData.h"
#include "MatrixIterator.h"
#include "MatrixCell.h"

/**
 * The only class exposed to the user of the library. It provides every method needed to use the matrix.
 * @tparam T the type of the data contained in each matrix cell
 */
template<typename T>
class Matrix {
	protected:
		std::shared_ptr<MatrixData<T>> data; //Pointer to the class holding the data

		/** Private constructor that accepts a pointer to the data */
		explicit Matrix(const std::shared_ptr<MatrixData<T>> &data) : data(data) {}

	public:

		/**
		 * Base constructor that creates a new matrix of the given size
		 * @param rows number of rows
		 * @param columns number of columns
		 */
		explicit Matrix(unsigned int rows, unsigned int columns) : data(std::make_shared<VectorMatrixData<T>>(rows, columns)) {
		}


		/**
		 * Copy constructor that triggers a deep copy of the matrix
		 * @param other the other matrix
		 */
		Matrix(const Matrix<T> &other) : data(std::make_shared<VectorMatrixData<T>>(other.data->copy())) {
		}

		/**
		 * Move constructor. Default behaviour.
		 * @param other the other matrix
		 */
		Matrix(Matrix<T> &&other) noexcept = default;

		const T operator()(unsigned int row, unsigned int col) const {
			return this->data->get(row, col);
		}

		MatrixCell<T> operator()(unsigned int row, unsigned int col) {
			return MatrixCell<T>(this->data, row, col);
		}

		/**
		 * @return the number of columns
		 */
		unsigned int columns() const {
			return this->data->columns();
		}

		/**
		 * @return the number of rows
		 */
		unsigned int rows() const {
			return this->data->rows();
		}

		/**
		 * @return the total number of cells (rows*columns)
		 */
		unsigned int size() const {
			return rows() * columns();
		}

		Matrix<T> submatrix(unsigned int rowOffset, unsigned int colOffset, unsigned int rows, unsigned int columns) {
			return Matrix<T>(std::make_shared<SubmatrixMD<T>>(rowOffset, colOffset, rows, columns, this->data));
		}

		const Matrix<T> submatrix(unsigned int rowOffset, unsigned int colOffset, unsigned int rows, unsigned int columns) const {
			return Matrix<T>(std::make_shared<SubmatrixMD<T>>(rowOffset, colOffset, rows, columns, this->data));
		}

		Matrix<T> transpose() {
			return Matrix<T>(std::make_shared<TransposedMD<T>>(this->data));
		}

		const Matrix<T> transpose() const {
			return Matrix<T>(std::make_shared<TransposedMD<T>>(this->data));
		}

		Matrix<T> diagonal() {
			if (!isSquared()) {
				throw "diagonal() can only be called on squared matrices";
			}
			return Matrix<T>(std::make_shared<DiagonalMD<T>>(this->data));
		}

		const Matrix<T> diagonal() const {
			if (!isSquared()) {
				throw "diagonal() can only be called on squared matrices";
			}
			return Matrix<T>(std::make_shared<DiagonalMD<T>>(this->data));
		}

		/**
		* Can only be called on a vector.
		* @return an immutable diagonal square matrix that has this vector as diagonal and <code>0</code> (zero) in all other positions.
		*/
		const Matrix<T> diagonalMatrix() const {
			if (!isVector()) {
				throw "diagonalMatrix() can only be called on vectors (nx1 matrices)";
			}
			return Matrix<T>(std::make_shared<DiagonalMatrixMD<T>>(this->data));
		}

		template<typename O>
		Matrix<O> cast() {
			return Matrix<O>(std::make_shared<Caster<T, O>>(this->data));
		}

		template<typename O>
		const Matrix<O> cast() const {
			return Matrix<O>(std::make_shared<Caster<T, O>>(this->data));
		}

		template<typename O>
		const Matrix<O> readOnlyCast() const {
			return Matrix<O>(std::make_shared<ReadOnlyCaster<T, O>>(this->data));
		}

		/**
		 * Multiplies the two given matrices
		 */
		const Matrix<T> operator*(Matrix<T> &another) const {
			if (this->columns() != another.rows()) {
				throw "Multiplication should be performed on compatible matrices";
			}
			return Matrix<T>(std::make_shared<MultiplyMatrix<T>>(this->data, another.data));
		}

		/**
		 * Adds the two given matrices
		 */
		template<typename U>
		const Matrix<decltype(T() * U())> operator+(Matrix<U> &another) const {
			if (this->columns() != another.columns() || this->rows() != another.rows()) {
				throw "Addetion should be performed on compatible matrices";
			}
			return Matrix<decltype(T() * U())>(std::make_shared<SumMatrix<T, U>>(this->data, another.data));
		}

		/**
 		 * @return true if this matrix is a square (has the same number of rows and columns)
 		 */
		bool isSquared() const {
			return rows() == columns();
		}

		/**
		 * @return true if this matrix is a vector (has only one column)
		 */
		bool isVector() const {
			return columns() == 1;
		}

		/**
		  * @return true if this matrix is a covector (has only one row)
		*/
		bool isCovector() const {
			return rows() == 1;
		}


		/**
		 * @return an iterator on the first position. This iterator moves from left to right, and then top to bottom.
		 */
		MatrixRowMajorIterator<T> beginRowMajor() const {
			return MatrixRowMajorIterator<T>(this->data, 0, 0);
		}

		/**
		 * @return an iterator on the last position. This iterator moves from left to right, and then top to bottom.
		 */
		MatrixRowMajorIterator<T> endRowMajor() const {
			return MatrixRowMajorIterator<T>(this->data, rows(), 0);
		}

		/**
		 * @return an iterator on the first position. This iterator moves from top to bottom, and then left to right.
		 */
		MatrixColumnMajorIterator<T> beginColumnMajor() const {
			return MatrixColumnMajorIterator<T>(this->data, 0, 0);
		}

		/**
		* @return an iterator on the last position. This iterator moves from top to bottom, and then left to right.
		*/
		MatrixColumnMajorIterator<T> endColumnMajor() const {
			return MatrixColumnMajorIterator<T>(this->data, 0, columns());
		}

		/**
		 * Prints the content of this matrix to the standard output
		 * @param format the format string to use when printing values
		 * @param separator the separator between each column
		 */
		void print(const char *format, const char *separator = "  ") const {
			for (unsigned int row = 0; row < this->rows(); ++row) {
				for (unsigned int col = 0; col < this->columns(); ++col) {
					if (col > 0) {
						std::cout << separator;
					}
					//Since we are mixing cout and format, buffers need to be flushed
					std::cout.flush();
					printf(format, (T) (*this)(row, col));
					fflush(stdout);
				}
				std::cout << std::endl;
			}
		}
};

#endif //MATRIX_MATRIX_H
