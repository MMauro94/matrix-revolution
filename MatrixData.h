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

		/**
		 * Adds itself to the multiplication chain
		 */
		void addToMultiplicationChain(std::vector<MatrixData<T> *> &multiplicationChain) {
			multiplicationChain.push_back(this);
		}

	public:
		MatrixData(unsigned rows, unsigned columns) : _rows(rows), _columns(columns) {}

		virtual ~MatrixData() = default;

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


		virtual T virtualGet(unsigned row, unsigned col) const = 0;

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

		VectorMatrixData(unsigned rows, unsigned columns, std::shared_ptr<std::vector<T>> vector) : MatrixData<T>(rows, columns),
																									vector(vector) {
		}

		VectorMatrixData(unsigned rows, unsigned columns) : MatrixData<T>(rows, columns),
															vector(std::make_shared<std::vector<T>>(rows * columns)) {
		}

		T get(unsigned row, unsigned col) const {
			return (*this->vector.get())[row * this->columns() + col];
		}

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

		void set(unsigned row, unsigned col, T t) {
			(*this->vector.get())[row * this->columns() + col] = t;
		}

		VectorMatrixData<T> copy() const {
			//std::cout << "copying" << std::endl;
			return VectorMatrixData<T>(this->rows(), this->columns(), std::make_shared<std::vector<T>>(*this->vector.get()));
		}

		template<class MD>
		static VectorMatrixData<T> toVector(MD matrixData) {
			VectorMatrixData<T> ret(matrixData.rows(), matrixData.columns());
			for (unsigned r = 0; r < ret.rows(); r++) {
				for (unsigned c = 0; c < ret.columns(); c++) {
					ret.set(r, c, matrixData.get(r, c));
				}
			}
			return ret;
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

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
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

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
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

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
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

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
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

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

		SumMatrix<T, MD1, MD2> copy() const {
			return SumMatrix<T, MD1, MD2>(this->left.copy(), this->right.copy());
		}
};


#endif //MATRIX_MATRIXDATA_H
