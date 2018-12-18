//
// Created by MMarco on 09/12/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#ifndef MATRIX_STATICSIZEMATRIX_H
#define MATRIX_STATICSIZEMATRIX_H

#include "Matrix.h"

template<unsigned int ROWS, unsigned int COLUMNS, typename T, class MD = VectorMatrixData<T>>
class StaticSizeMatrix : public Matrix<T, MD> {
	private:
		//This allows us to access protected members of another StaticSizeMatrix of a different size
		template<unsigned int R, unsigned int C, typename U, class MDD> friend
		class StaticSizeMatrix;

		//This allows us to access protected members of StaticSizeMatrix from Matrix
		template<typename U, class MDD> friend
		class Matrix;

	protected:
		explicit StaticSizeMatrix(const std::shared_ptr<MD> &data) : Matrix(data) {
			if (data->columns() != COLUMNS) {
				throw "Invalid columns count";
			} else if (data->rows() != ROWS) {
				throw "Invalid rows count";
			}
		}

	public:
		StaticSizeMatrix() : Matrix(ROWS, COLUMNS) {
		}

		template<unsigned int ROW, unsigned int COL>
		typename std::enable_if<ROW >= 0 && COL >= 0 && ROW < ROWS && COL < COLUMNS, MatrixCell<T, MD>>::type get() {
			return (*this)(ROW, COL);
		}

		template<unsigned int ROW, unsigned int COL>
		typename std::enable_if<ROW >= 0 && COL >= 0 && ROW < ROWS && COL < COLUMNS, T>::type get() const {
			return (*this)(ROW, COL);
		}


		/**
		 * @return a submatrix. The method is only enabled when the bounds are valid
		 */
		template<unsigned int ROW_OFFSET, unsigned int COL_OFFSET, unsigned int ROW_COUNT, unsigned int COL_COUNT>
		typename std::enable_if<ROW_OFFSET + ROW_COUNT <= ROWS &&
								COL_OFFSET + COL_COUNT <= COLUMNS, StaticSizeMatrix<ROW_COUNT, COL_COUNT, T, SubmatrixMD<T, MD>>>::type
		submatrix() {
			return StaticSizeMatrix<ROW_COUNT, COL_COUNT, T, SubmatrixMD<T, MD>>(
					std::make_shared<SubmatrixMD<T, MD>>(ROW_OFFSET, COL_OFFSET, ROW_COUNT, COL_COUNT, this->data));
		}

		template<unsigned int ROW_OFFSET, unsigned int COL_OFFSET, unsigned int ROW_COUNT, unsigned int COL_COUNT>
		const typename std::enable_if<
				ROW_OFFSET + ROW_COUNT <= ROWS &&
				COL_OFFSET + COL_COUNT <= COLUMNS, StaticSizeMatrix<ROW_COUNT, COL_COUNT, T, SubmatrixMD<T, MD>>>::type
		submatrix() const {
			return StaticSizeMatrix<ROW_COUNT, COL_COUNT, T, SubmatrixMD<T, MD>>(
					std::make_shared<SubmatrixMD<T, MD>>(ROW_OFFSET, COL_OFFSET, ROW_COUNT, COL_COUNT, this->data));
		}

		using Matrix<T, MD>::submatrix;

		/**
		 * @return the transposed matrix
		 */
		StaticSizeMatrix<COLUMNS, ROWS, T, TransposedMD<T, MD>> transpose() {
			return StaticSizeMatrix<COLUMNS, ROWS, T, TransposedMD<T, MD>>(std::make_shared<TransposedMD<T, MD>>(this->data));
		}

		const StaticSizeMatrix<COLUMNS, ROWS, T, TransposedMD<T, MD>> transpose() const {
			return StaticSizeMatrix<COLUMNS, ROWS, T, TransposedMD<T, MD>>(std::make_shared<TransposedMD<T, MD>>(this->data));
		}

		/**
		 * This method is added only on squared matrix
		 */
		template<unsigned int R = ROWS, unsigned int C = COLUMNS>
		typename std::enable_if<R == C, StaticSizeMatrix<ROWS, 1, T, DiagonalMD<T, MD>>>::type diagonal() {
			return StaticSizeMatrix<ROWS, 1, T, DiagonalMD<T, MD>>(std::make_shared<DiagonalMD<T, MD>>(this->data));
		}

		template<unsigned int R = ROWS, unsigned int C = COLUMNS>
		const typename std::enable_if<R == C, StaticSizeMatrix<ROWS, 1, T, DiagonalMD<T, MD>>>::type diagonal() const {
			return StaticSizeMatrix<ROWS, 1, T, DiagonalMD<T, MD>>(std::make_shared<DiagonalMD<T, MD>>(this->data));
		}

		/**
		* Can only be called on a vector.
		* @return an immutable diagonal square matrix that has this vector as diagonal and <code>0</code> (zero) in all other positions.
		*/
		template<unsigned int C = COLUMNS>
		const typename std::enable_if<C == 1, StaticSizeMatrix<ROWS, ROWS, T, DiagonalMatrixMD<T, MD>>>::type diagonalMatrix() const {
			return StaticSizeMatrix<ROWS, ROWS, T, DiagonalMatrixMD<T, MD>>(std::make_shared<DiagonalMatrixMD<T, MD>>(this->data));
		}

		/**
		 * Multiplies the two given matrices
		 */
		template<unsigned int C, typename U, class MD2>
		const StaticSizeMatrix<ROWS, C, decltype(T() * U()), MultiplyMatrix<decltype(T() * U()), MD, MD2>>
		operator*(const StaticSizeMatrix<COLUMNS, C, U, MD2> &another) const {
			return StaticSizeMatrix<ROWS, C, decltype(T() * U()), MultiplyMatrix<decltype(T() * U()), MD, MD2>>(
					std::make_shared<MultiplyMatrix<decltype(T() * U()), MD, MD2>>(this->data, another.data));
		}

		template<typename U, class MD2>
		const Matrix<decltype(T() * U()), MultiplyMatrix<decltype(T() * U()), MD, MD2>> operator*(const Matrix<U, MD2> &another) const {
			if (COLUMNS != another.rows()) {
				throw "Multiplication should be performed on compatible matrices";
			}
			return Matrix<decltype(T() * U()), MultiplyMatrix<decltype(T() * U()), MD, MD2>>(
					std::make_shared<MultiplyMatrix<decltype(T() * U()), MD, MD2>>(this->data, another.data));
		}

		/**
		 * Adds the two given matrices
		 */
		template<typename U, class MD2>
		const StaticSizeMatrix<ROWS, COLUMNS, decltype(T() + U()), SumMatrix<decltype(T() + U()), MD, MD2>>
		operator+(const StaticSizeMatrix<ROWS, COLUMNS, U, MD2> &another) const {
			return StaticSizeMatrix<ROWS, COLUMNS, decltype(T() + U()), SumMatrix<decltype(T() + U()), MD, MD2>>(
					std::make_shared<SumMatrix<decltype(T() + U()), MD, MD2>>(this->data, another.data));
		}

		template<typename U, class MD2>
		const StaticSizeMatrix<ROWS, COLUMNS, decltype(T() + U()), SumMatrix<decltype(T() + U()), MD, MD2>>
		operator+(const Matrix<U, MD2> &another) const {
			if (COLUMNS != another.columns() || ROWS != another.rows()) {
				throw "Addetion should be performed on compatible matrices";
			}
			return StaticSizeMatrix<ROWS, COLUMNS, decltype(T() + U()), SumMatrix<decltype(T() + U()), MD, MD2>>(
					std::make_shared<SumMatrix<decltype(T() + U()), MD, MD2>>(this->data, another.data));
		}

};

#endif //MATRIX_STATICSIZEMATRIX_H


#pragma clang diagnostic pop