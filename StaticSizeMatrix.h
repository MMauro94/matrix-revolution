//
// Created by MMarco on 09/12/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#ifndef MATRIX_STATICSIZEMATRIX_H
#define MATRIX_STATICSIZEMATRIX_H

#include "Matrix.h"

template<int ROWS, int COLUMNS, typename T>
class StaticSizeMatrix : public Matrix<T> {
	private:
		//This allows us to access protected members of another StaticSizeMatrix of a different size
		template<int R, int C, typename U> friend
		class StaticSizeMatrix;

	protected:
		explicit StaticSizeMatrix(const std::shared_ptr<MatrixData<T>> &data) : Matrix(data) {
			if (data->columns() != COLUMNS) {
				throw "Invalid columns count";
			} else if (data->rows() != ROWS) {
				throw "Invalid rows count";
			}
		}

	public:
		StaticSizeMatrix() : Matrix(ROWS, COLUMNS) {
		}

		template<int ROW, int COL>
		typename std::enable_if<ROW >= 0 && COL >= 0 && ROW < ROWS && COL < COLUMNS, MatrixCell<T>>::type get() {
			return (*this)(ROW, COL);
		}

		template<int ROW, int COL>
		typename std::enable_if<ROW >= 0 && COL >= 0 && ROW < ROWS && COL < COLUMNS, T>::type get() const {
			return (*this)(ROW, COL);
		}


		/**
		 * @return a submatrix. The method is only enabled when the bounds are valid
		 */
		template<int ROW_OFFSET, int COL_OFFSET, int ROW_COUNT, int COL_COUNT>
		typename std::enable_if<ROW_OFFSET >= 0 && COL_OFFSET >= 0 && ROW_OFFSET + ROW_COUNT < ROWS &&
								COL_OFFSET + COL_COUNT < COLUMNS, StaticSizeMatrix<ROW_COUNT, COL_COUNT, T>>::type
		submatrix() {
			return StaticSizeMatrix<ROW_COUNT, COL_COUNT, T>(
					std::make_shared<SubmatrixMD<T>>(ROW_OFFSET, COL_OFFSET, ROW_COUNT, COL_COUNT, this->data));
		}

		template<int ROW_OFFSET, int COL_OFFSET, int ROW_COUNT, int COL_COUNT>
		const typename std::enable_if<ROW_OFFSET >= 0 && COL_OFFSET >= 0 && ROW_OFFSET + ROW_COUNT < ROWS &&
									  COL_OFFSET + COL_COUNT < COLUMNS, StaticSizeMatrix<ROW_COUNT, COL_COUNT, T>>::type
		submatrix() const {
			return StaticSizeMatrix<ROW_COUNT, COL_COUNT, T>(
					std::make_shared<SubmatrixMD<T>>(ROW_OFFSET, COL_OFFSET, ROW_COUNT, COL_COUNT, this->data));
		}

		/**
		 * @return the transposed matrix
		 */
		StaticSizeMatrix<COLUMNS, ROWS, T> transpose() {
			return StaticSizeMatrix<COLUMNS, ROWS, T>(std::make_shared<TransposedMD<T>>(this->data));
		}

		const StaticSizeMatrix<COLUMNS, ROWS, T> transpose() const {
			return StaticSizeMatrix<COLUMNS, ROWS, T>(std::make_shared<TransposedMD<T>>(this->data));
		}

		/**
		 * This method is added only on squared matrix
		 */
		template<int R = ROWS, int C = COLUMNS>
		typename std::enable_if<R == C, StaticSizeMatrix<ROWS, 1, T>>::type diagonal() {
			return StaticSizeMatrix<ROWS, 1, T>(std::make_shared<DiagonalMD<T>>(this->data));
		}

		template<int R = ROWS, int C = COLUMNS>
		const typename std::enable_if<R == C, StaticSizeMatrix<ROWS, 1, T>>::type diagonal() const {
			return StaticSizeMatrix<ROWS, 1, T>(std::make_shared<DiagonalMD<T>>(this->data));
		}

		/**
		* Can only be called on a vector.
		* @return an immutable diagonal square matrix that has this vector as diagonal and <code>0</code> (zero) in all other positions.
		*/
		template<int C = COLUMNS>
		const typename std::enable_if<C == 1, StaticSizeMatrix<ROWS, ROWS, T>>::type diagonalMatrix() const {
			return StaticSizeMatrix<ROWS, ROWS, T>(std::make_shared<DiagonalMatrixMD<T>>(this->data));
		}

		/**
		 * Multiplies the two given matrices
		 */
		template<int C>
		const StaticSizeMatrix<ROWS, C, T> operator*(StaticSizeMatrix<COLUMNS, C, T> &another) const {
			return StaticSizeMatrix<ROWS, C, T>(std::make_shared<MultiplyMatrix<T>>(this->data, another.data));
		}

		/**
		 * Adds the two given matrices
		 */
		const StaticSizeMatrix<ROWS, COLUMNS, T> operator+(StaticSizeMatrix<ROWS, COLUMNS, T> &another) const {
			return StaticSizeMatrix<ROWS, COLUMNS, T>(std::make_shared<SumMatrix<T>>(this->data, another.data));
		}
};

#endif //MATRIX_STATICSIZEMATRIX_H


#pragma clang diagnostic pop