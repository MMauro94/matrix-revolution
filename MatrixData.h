#ifndef MATRIX_MATRIXDATA_H
#define MATRIX_MATRIXDATA_H

#include <memory>
#include <vector>
#include <algorithm>

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

		template<typename T, class MD1, class MD2> friend
		class MultiplyMatrix;

		unsigned int _rows, _columns;

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
		MatrixData(unsigned int rows, unsigned int columns) : _rows(rows), _columns(columns) {}

		/**
		 * @return number of columns
		 */
		unsigned int columns() const {
			return this->_columns;
		}

		/**
		 * @return number of rows
		 */
		unsigned int rows() const {
			return this->_rows;
		}

		/**
		 * @return a copy of this data
		 */
		VectorMatrixData<T> copy();

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
		std::vector<T> vector;

	public:
		VectorMatrixData(unsigned int rows, unsigned int columns) : MatrixData<T>(rows, columns), vector(rows * columns) {
		}

		T get(unsigned int row, unsigned int col) const {
			return this->vector[row * this->columns() + col];
		}

		void set(unsigned int row, unsigned int col, T t) {
			this->vector[row * this->columns() + col] = t;
		}

};

template<typename T>
VectorMatrixData<T> MatrixData<T>::copy() {
	VectorMatrixData<T> ret(rows(), columns());
	for (unsigned int i = 0; i < rows(); ++i) {
		for (unsigned int j = 0; j < columns(); ++j) {
			ret.set(i, j, get(i, j));
		}
	}
	return ret;
}

/**
 * Implementation of <code>MatrixData</code> that exposes a submatrix of another <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class SubmatrixMD : public MatrixData<T> {

	private:
		std::shared_ptr<MD> wrapped;
		unsigned int rowOffset, colOffset;


	public:

		SubmatrixMD(unsigned int rowOffset, unsigned int colOffset, unsigned int rows, unsigned int columns,
					const std::shared_ptr<MD> &wrapped) : MatrixData<T>(rows,
																		columns),
														  rowOffset(rowOffset),
														  colOffset(colOffset),
														  wrapped(wrapped) {
		}

		T get(unsigned int row, unsigned int col) const {
			return this->wrapped->get(row + this->rowOffset, col + this->colOffset);
		}

		void set(unsigned int row, unsigned int col, T t) {
			this->wrapped->set(row + this->rowOffset, col + this->colOffset, t);
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes the transposed matrix of another <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class TransposedMD : public MatrixData<T> {

	private:

		std::shared_ptr<MD> wrapped;

	public:

		explicit TransposedMD(const std::shared_ptr<MD> &wrapped) : MatrixData<T>(wrapped->columns(), wrapped->rows()), wrapped(wrapped) {
		}

		T get(unsigned int row, unsigned int col) const {
			return this->wrapped->get(col, row);
		}

		void set(unsigned int row, unsigned int col, T t) {
			this->wrapped->set(col, row, t);
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes the diagonal vector of another squared <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class DiagonalMD : public MatrixData<T> {

	private:

		std::shared_ptr<MD> wrapped;

	public:

		explicit DiagonalMD(const std::shared_ptr<MD> &wrapped) : MatrixData<T>(wrapped->rows(), 1), wrapped(wrapped) {
		}

		T get(unsigned int row, unsigned int col) const {
			return this->wrapped->get(row, row);
		}

		void set(unsigned int row, unsigned int col, T t) {
			this->wrapped->set(row, row, t);
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes a square diagonal matrix of another vector <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class DiagonalMatrixMD : public MatrixData<T> {

	private:

		std::shared_ptr<MD> wrapped;

	public:

		explicit DiagonalMatrixMD(const std::shared_ptr<MD> &wrapped) : MatrixData<T>(wrapped->rows(), wrapped->rows()),
																		wrapped(wrapped) {
		}

		T get(unsigned int row, unsigned int col) const {
			if (row == col) {
				return this->wrapped->get(row, 0);
			} else {
				return 0;
			}
		}
};


/**
 * Implementation of <code>MatrixData</code> that exposes the sum of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class SumMatrix : public MatrixData<T> {

	private:
		std::shared_ptr<MD1> first;
		std::shared_ptr<MD2> second;

	public:

		explicit SumMatrix(const std::shared_ptr<MD1> &first, const std::shared_ptr<MD2> &second) :
				MatrixData<T>(first->rows(), second->columns()), first(first), second(second) {
		}

		T get(unsigned int row, unsigned int col) const {
			return this->first->get(row, col) + this->second->get(row, col);
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes the multiplication of the two given matrices
 * @tparam T type of the data
 */
template<typename T, class MD1, class MD2>
class MultiplyMatrix : public MatrixData<T> {

	private:
		std::shared_ptr<MD1> left;
		std::shared_ptr<MD2> right;
		mutable bool optimized = false;

		template<typename T, class MD1, class MD2> friend
		class MultiplyMatrix;


	public:

		explicit MultiplyMatrix(const std::shared_ptr<MD1> &first, const std::shared_ptr<MD2> &second) :
				MatrixData<T>(first->rows(), second->columns()), left(first), right(second) {
		}

		T get(unsigned int row, unsigned int col) const {
			if (!this->optimized) {
				this->optimize();
			}
			T ret = 0;
			for (unsigned int j = 0; j < this->left->columns(); j++) {
				ret += this->left->get(row, j) * this->right->get(j, col);
			}
			return ret;
			/*this->optimize();
			if (this->optimized == NULL) {
				throw "Illegal state";
			}
			//std::cout << "Accessing " << row << "," << col << " (" << this->optimized->rows() << "x" << this->optimized->columns() << ")" << std::endl;
			return this->optimized->get(row, col);*/
		}

		void printForMultiplicationDebug() const {
			std::cout << "(";
			this->left->printForMultiplicationDebug();
			std::cout << ")*(";
			this->right->printForMultiplicationDebug();
			std::cout << ")";
		}

	private:

		const MatrixData<T> *findRightmostForMultiplication() const {
			return this->right->findRightmostForMultiplication();
		}
		const MatrixData<T> *findLeftmostForMultiplication() const {
			return this->left->findRightmostForMultiplication();
		}

		const MatrixData<T> *findOptimalMatrixForMultiplication(const MatrixData<T> *rightmost) const {
			auto maxLeft = this->left->findOptimalMatrixForMultiplication(rightmost);
			auto maxRight = this->right->findOptimalMatrixForMultiplication(rightmost);
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
		static std::shared_ptr<MatrixData<T>> computeMultiplication(MatrixData<T> &first, MatrixData<T> &second) {
			auto ret = std::make_shared<VectorMatrixData<T>>(first.rows(), second.columns());
			for (unsigned int r = 0; r < ret->rows(); r++) {
				for (unsigned int c = 0; c < ret->columns(); c++) {
					T cell = 0;
					for (unsigned j = 0; j < first.columns(); j++) {
						cell += first.get(r, j) * second.get(j, c);
					}
					ret->set(r, c, cell);
				}
			}
			return std::dynamic_pointer_cast<MatrixData<T>>(ret);
		}
};

#endif //MATRIX_MATRIXDATA_H
