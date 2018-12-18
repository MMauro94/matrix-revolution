#ifndef MATRIX_MATRIXDATA_H
#define MATRIX_MATRIXDATA_H

#include <memory>
#include <vector>
#include <algorithm>
#include <tuple>

template<typename T>
class VectorMatrixData;

template<typename T, class MD1, class MD2>
class MultiplyMatrix;

/**
 * Abstract class that exposes the data of the matrix
 * @tparam T type of the data
 */
template<typename T>
class MatrixData {

	private:
		unsigned _rows, _columns;

		template<typename U, class MD1, class MD2> friend
		class MultiplyMatrix;

	protected:
		const MatrixData<T> *findOptimalMatrixForMultiplication(const MatrixData<T> *rightmost) const {
			return this;
		}

		const MatrixData<T> *findRightmostForMultiplication() const {
			return this;
		}

		const MatrixData<T> *findLeftmostForMultiplication() const {
			return this;
		}

	public:
		MatrixData(unsigned rows, unsigned columns) : _rows(rows), _columns(columns) {}

		/**
		 * @return number of columns
		 */
		unsigned columns() const {
			return this->_columns;
		}

		/**
		 * @return number of rows
		 */
		unsigned rows() const {
			return this->_rows;
		}

		void printForMultiplicationDebug() const {
			std::cout << this->_rows << "x" << this->_columns;
		}
};

/**
 * Implementation of <code>MatrixData</code> that actually holds the value in a <code>std::vector</code>
 * @tparam T type of the data
 */
template<typename T>
class VectorMatrixData : public MatrixData<T> {

	private:
		std::shared_ptr<std::vector<T>> vector;
	public:

		explicit VectorMatrixData(unsigned rows, unsigned columns, std::shared_ptr<std::vector<T>> vector) : MatrixData<T>(rows, columns),
																											 vector(vector) {
		}

		VectorMatrixData(unsigned rows, unsigned columns) : MatrixData<T>(rows, columns),
															vector(std::make_shared<std::vector<T>>(rows * columns)) {
		}

		T get(unsigned row, unsigned col) const {
			return (*this->vector.get())[row * this->columns() + col];
		}

		void set(unsigned row, unsigned col, T t) {
			(*this->vector.get())[row * this->columns() + col] = t;
		}

		VectorMatrixData<T> copy() const {
			//std::cout << "copying" << std::endl;
			return VectorMatrixData<T>(this->rows(), this->columns(), std::make_shared<std::vector<T>>(*this->vector.get()));
		}
};

/**
 * Implementation of <code>MatrixData</code> that exposes a submatrix of another <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class SubmatrixMD : public MatrixData<T> {

	private:
		MD wrapped;
		unsigned rowOffset, colOffset;


	public:

		SubmatrixMD(unsigned rowOffset, unsigned colOffset, unsigned rows, unsigned columns, MD wrapped)
				: MatrixData<T>(rows, columns), rowOffset(rowOffset), colOffset(colOffset), wrapped(wrapped) {
		}

		T get(unsigned row, unsigned col) const {
			return this->wrapped.get(row + this->rowOffset, col + this->colOffset);
		}

		void set(unsigned row, unsigned col, T t) {
			this->wrapped.set(row + this->rowOffset, col + this->colOffset, t);
		}

		SubmatrixMD<T, MD> copy() const {
			return SubmatrixMD<T, MD>(this->rowOffset, this->colOffset, this->rows(), this->columns(), this->wrapped.copy());
		}
};

/**
 * Implementation of <code>MatrixData</code> that exposes the transposed matrix of another <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class TransposedMD : public MatrixData<T> {

	private:

		MD wrapped;

	public:

		explicit TransposedMD(MD wrapped) : MatrixData<T>(wrapped.columns(), wrapped.rows()), wrapped(wrapped) {
		}

		T get(unsigned row, unsigned col) const {
			return this->wrapped.get(col, row);
		}

		void set(unsigned row, unsigned col, T t) {
			this->wrapped.set(col, row, t);
		}

		TransposedMD<T, MD> copy() const {
			return TransposedMD<T, MD>(this->wrapped.copy());
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes the diagonal vector of another squared <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class DiagonalMD : public MatrixData<T> {

	private:

		MD wrapped;

	public:

		explicit DiagonalMD(MD wrapped) : MatrixData<T>(wrapped.rows(), 1), wrapped(wrapped) {
		}

		T get(unsigned row, unsigned col) const {
			return this->wrapped.get(row, row);
		}

		void set(unsigned row, unsigned col, T t) {
			this->wrapped.set(row, row, t);
		}

		DiagonalMD<T, MD> copy() const {
			return DiagonalMD<T, MD>(this->wrapped.copy());
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes a square diagonal matrix of another vector <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class DiagonalMatrixMD : public MatrixData<T> {

	private:

		MD wrapped;

	public:

		explicit DiagonalMatrixMD(MD wrapped) : MatrixData<T>(wrapped.rows(), wrapped.rows()),
												wrapped(wrapped) {
		}

		T get(unsigned row, unsigned col) const {
			if (row == col) {
				return this->wrapped.get(row, 0);
			} else {
				return 0;
			}
		}

		DiagonalMatrixMD<T, MD> copy() const {
			return DiagonalMatrixMD<T, MD>(this->wrapped.copy());
		}
};


/**
 * Implementation of <code>MatrixData</code> that exposes the sum of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class SumMatrix : public MatrixData<T> {

	private:
		MD1 left;
		MD2 right;

	public:

		explicit SumMatrix(MD1 left, MD2 right) :
				MatrixData<T>(left.rows(), right.columns()), left(left), right(right) {
		}

		T get(unsigned row, unsigned col) const {
			return this->left.get(row, col) + this->right.get(row, col);
		}


		SumMatrix<T, MD1, MD2> copy() const {
			return SumMatrix<T, MD1, MD2>(this->left.copy(), this->right.copy());
		}
};

/**
 * Implementation of <code>MatrixData</code> that exposes the multiplication of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class MultiplyMatrix : public MatrixData<T> {

	private:
		MD1 left;
		MD2 right;
		mutable bool optimized = false;

		template<typename T, class MD1, class MD2> friend
		class MultiplyMatrix;


	public:

		explicit MultiplyMatrix(MD1 left, MD2 right) :
				MatrixData<T>(left.rows(), right.columns()), left(left), right(right) {
		}

		T get(unsigned row, unsigned col) const {
			if (!this->optimized) {
				this->optimize();
			}
			T ret = 0;
			for (unsigned j = 0; j < this->left.columns(); j++) {
				ret += this->left.get(row, j) * this->right.get(j, col);
			}
			return ret;
			/*this->optimize();
			if (this->optimized == NULL) {
				throw "Illegal state";
			}
			//std::cout << "Accessing " << row << "," << col << " (" << this->optimized.rows() << "x" << this->optimized.columns() << ")" << std::endl;
			return this->optimized.get(row, col);*/
		}

		MultiplyMatrix<T, MD1, MD2> copy() const {
			return MultiplyMatrix<T, MD1, MD2>(this->left.copy(), this->right.copy());
		}

		void printForMultiplicationDebug() const {

			std::cout << "(";
			this->left.printForMultiplicationDebug();
			std::cout << ")*(";
			this->right.printForMultiplicationDebug();
			std::cout << ")";
		}

	private:

		const MatrixData<T> *findRightmostForMultiplication() const {
			return this->right.findRightmostForMultiplication();
		}

		const MatrixData<T> *findLeftmostForMultiplication() const {
			return this->left.findRightmostForMultiplication();
		}

		const MatrixData<T> *findOptimalMatrixForMultiplication(const MatrixData<T> *rightmost) const {
			auto maxLeft = this->left.findOptimalMatrixForMultiplication(rightmost);
			auto maxRight = this->right.findOptimalMatrixForMultiplication(rightmost);
			if (maxRight == rightmost || maxLeft->columns() > maxRight->columns()) {
				return maxLeft;
			} else {
				return maxRight;
			}
		}

		void optimize() const {
			std::cout << "Optimizing";
			this->printForMultiplicationDebug();
			const MatrixData<T> *rightMost = this->findRightmostForMultiplication();
			const MatrixData<T> *optimal = this->findOptimalMatrixForMultiplication(rightMost);


			std::cout << std::endl << "Max col = " << optimal->columns() << std::endl;
			this->optimized = true;
		}

	protected:
		template<typename T>
		static VectorMatrixData<T> computeMultiplication(MatrixData<T> &left, MatrixData<T> &right) {
			auto ret = VectorMatrixData<T>(left.rows(), right.columns());
			for (unsigned r = 0; r < ret.rows(); r++) {
				for (unsigned c = 0; c < ret.columns(); c++) {
					T cell = 0;
					for (unsigned j = 0; j < left.columns(); j++) {
						cell += left.get(r, j) * right.get(j, c);
					}
					ret->set(r, c, cell);
				}
			}
			return ret;
		}
};

#endif //MATRIX_MATRIXDATA_H
