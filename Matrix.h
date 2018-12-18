#ifndef MATRIX_MATRIX_H
#define MATRIX_MATRIX_H

#include <memory>
#include <string>
#include <iostream>
#include "MatrixData.h"
#include "MatrixIterator.h"
#include "MatrixCell.h"


template<unsigned int ROWS, unsigned int COLUMNS, typename T, class MD>
class StaticSizeMatrix;

/**
 * The only class exposed to the user of the library. It provides every method needed to use the matrix.
 * @tparam T the type of the data contained in each matrix cell
 */
template<typename T, class MD = VectorMatrixData<T>>
class Matrix {
	private:
		//This allows us to access protected members of another Matrix of a different type
		template<typename U, class MDD> friend
		class Matrix;

		//This allows us to access protected members of another StaticSizeMatrix of a different size
		template<unsigned int R, unsigned int C, typename U, class MDD> friend
		class StaticSizeMatrix;


	protected:
		std::shared_ptr<MD> data; //Pointer to the class holding the data

		/** Private constructor that accepts a pointer to the data */
		explicit Matrix(const std::shared_ptr<MD> &data) : data(data) {}

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
		Matrix(const Matrix<T, MD> &other) : data(std::make_shared<VectorMatrixData<T>>(other.data->copy())) {
		}

		/**
		 * Move constructor. Default behaviour.
		 * @param other the other matrix
		 */
		Matrix(Matrix<T, MD> &&other) noexcept = default;

		const T operator()(unsigned int row, unsigned int col) const {
			if (row < 0 || row >= this->rows()) {
				throw "Row out of bounds";
			} else if (col < 0 || col >= this->columns()) {
				throw "Column out of bounds";
			}
			return this->data->get(row, col);
		}

		MatrixCell<T, MD> operator()(unsigned int row, unsigned int col) {
			return MatrixCell<T, MD>(this->data, row, col);
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

		Matrix<T, SubmatrixMD<T, MD>> submatrix(unsigned int rowOffset, unsigned int colOffset, unsigned int rows, unsigned int columns) {
			if (rowOffset + rows > this->rows() || colOffset + columns > this->columns()) {
				throw "Illegal bounds";
			}
			return Matrix<T, SubmatrixMD<T, MD>>(std::make_shared<SubmatrixMD<T, MD>>(rowOffset, colOffset, rows, columns, this->data));
		}

		const Matrix<T, SubmatrixMD<T, MD>> submatrix(unsigned int rowOffset, unsigned int colOffset, unsigned int rows, unsigned int columns) const {
			if (rowOffset + rows > this->rows() || colOffset + columns > this->columns()) {
				throw "Illegal bounds";
			}
			return Matrix<T, SubmatrixMD<T, MD>>(std::make_shared<SubmatrixMD<T, MD>>(rowOffset, colOffset, rows, columns, this->data));
		}

		template<unsigned int ROW_COUNT, unsigned int COL_COUNT>
		StaticSizeMatrix<ROW_COUNT, COL_COUNT, T, SubmatrixMD<T, MD>> submatrix(unsigned int rowOffset, unsigned int colOffset) {
			if (rowOffset + ROW_COUNT > this->rows() || colOffset + COL_COUNT > this->columns()) {
				throw "Illegal bounds";
			}
			return StaticSizeMatrix<ROW_COUNT, COL_COUNT, T, SubmatrixMD<T, MD>>(
					std::make_shared<SubmatrixMD<T, MD>>(rowOffset, colOffset, ROW_COUNT, COL_COUNT, this->data));
		}

		template<unsigned int ROW_COUNT, unsigned int COL_COUNT>
		const StaticSizeMatrix<ROW_COUNT, COL_COUNT, T, SubmatrixMD<T, MD>> submatrix(unsigned int rowOffset, unsigned int colOffset) const {
			if (rowOffset + ROW_COUNT > this->rows() || colOffset + COL_COUNT > this->columns()) {
				throw "Illegal bounds";
			}
			return StaticSizeMatrix<ROW_COUNT, COL_COUNT, T, SubmatrixMD<T, MD>>(
					std::make_shared<SubmatrixMD<T, MD>>(rowOffset, colOffset, ROW_COUNT, COL_COUNT, this->data));
		}

		Matrix<T, TransposedMD<T, MD>> transpose() {
			return Matrix<T, TransposedMD<T, MD>>(std::make_shared<TransposedMD<T, MD>>(this->data));
		}

		const Matrix<T, TransposedMD<T, MD>> transpose() const {
			return Matrix<T, TransposedMD<T, MD>>(std::make_shared<TransposedMD<T, MD>>(this->data));
		}

		Matrix<T, DiagonalMD<T, MD>> diagonal() {
			if (!isSquared()) {
				throw "diagonal() can only be called on squared matrices";
			}
			return Matrix<T, DiagonalMD<T, MD>>(std::make_shared<DiagonalMD<T, MD>>(this->data));
		}

		const Matrix<T, DiagonalMD<T, MD>> diagonal() const {
			if (!isSquared()) {
				throw "diagonal() can only be called on squared matrices";
			}
			return Matrix<T, DiagonalMD<T, MD>>(std::make_shared<DiagonalMD<T, MD>>(this->data));
		}

		/**
		* Can only be called on a vector.
		* @return an immutable diagonal square matrix that has this vector as diagonal and <code>0</code> (zero) in all other positions.
		*/
		const Matrix<T, DiagonalMatrixMD<T, MD>> diagonalMatrix() const {
			if (!isVector()) {
				throw "diagonalMatrix() can only be called on vectors (nx1 matrices)";
			}
			return Matrix<T, DiagonalMatrixMD<T, MD>>(std::make_shared<DiagonalMatrixMD<T, MD>>(this->data));
		}

		/**
		 * Multiplies the two given matrices
		 */
		template<class MD2>
		const Matrix<T, MultiplyMatrix<T, MD, MD2>> operator*(const Matrix<T, MD2> &another) const {
			if (this->columns() != another.rows()) {
				throw "Multiplication should be performed on compatible matrices";
			}
			return Matrix<T, MultiplyMatrix<T, MD, MD2>>(std::make_shared<MultiplyMatrix<T, MD, MD2>>(this->data, another.data));
		}

		/**
		 * Adds the two given matrices
		 */
		template<typename U, class MD2>
		const Matrix<decltype(T() + U()), SumMatrix<decltype(T() + U()), MD, MD2>> operator+(const Matrix<U, MD2> &another) const {
			if (this->columns() != another.columns() || this->rows() != another.rows()) {
				throw "Addetion should be performed on compatible matrices";
			}
			return Matrix<decltype(T() + U()), SumMatrix<decltype(T() + U()), MD, MD2>>(
					std::make_shared<SumMatrix<decltype(T() + U()), MD, MD2>>(this->data, another.data));
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
		MatrixRowMajorIterator<T, MD> beginRowMajor() const {
			return MatrixRowMajorIterator<T, MD>(this->data, 0, 0);
		}

		/**
		 * @return an iterator on the last position. This iterator moves from left to right, and then top to bottom.
		 */
		MatrixRowMajorIterator<T, MD> endRowMajor() const {
			return MatrixRowMajorIterator<T, MD>(this->data, rows(), 0);
		}

		/**
		 * @return an iterator on the first position. This iterator moves from top to bottom, and then left to right.
		 */
		MatrixColumnMajorIterator<T, MD> beginColumnMajor() const {
			return MatrixColumnMajorIterator<T, MD>(this->data, 0, 0);
		}

		/**
		* @return an iterator on the last position. This iterator moves from top to bottom, and then left to right.
		*/
		MatrixColumnMajorIterator<T, MD> endColumnMajor() const {
			return MatrixColumnMajorIterator<T, MD>(this->data, 0, columns());
		}

		void printForMultiplicationDebug() const {
			this->data->printForMultiplicationDebug();
			std::cout << std::endl;
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
