#ifndef MATRIX_MATRIXDATA_H
#define MATRIX_MATRIXDATA_H

#include <memory>
#include <vector>
#include <algorithm>
#include <tuple>
#include <deque>
#include <mutex>
#include "Utils.h"

template<typename T>
class VectorMatrixData;

template<typename T, class MD1, class MD2>
class MultiplyMD;

//This macro is used to add the method virtualMaterialize() to implementations of MatrixData, without copy-pasting code.
//It is necessary, since this methods call an inherited non-virtual method (i.e. get(r,c))
#define MATERIALIZE_IMPL        \
VectorMatrixData<T> virtualMaterialize(unsigned rowOffset, unsigned colOffset, unsigned rows, unsigned columns) const override {\
    if (rows < 0 || columns < 0 || rowOffset < 0 || colOffset < 0 || rowOffset + rows > this->rows() || colOffset + columns > this->columns()) {\
        Utils::error("Illegal bounds");\
    }\
    if (!this->optimizeHasBeenCalled) {\
        this->optimize();\
    }\
    VectorMatrixData<T> ret(rows, columns);\
    for (unsigned r = 0; r < rows; r++) {\
        for (unsigned c = 0; c < columns; c++) {\
            ret.set(r, c, this->doGet(r + rowOffset, c + colOffset));\
        }\
    }\
    return ret;\
}\
\
T get(unsigned row, unsigned col) const {\
    if (!this->optimizeHasBeenCalled) {\
        this->optimize();\
    }\
    return this->doGet(row, col);\
}

/**
 * Abstract class that exposes the data of the matrix
 * @tparam T type of the data
 */
template<typename T>
class MatrixData {

	private:
		unsigned _rows, _columns;

		template<typename U, class MD1, class MD2> friend
		class MultiplyMD;

	protected:

		mutable bool optimizeHasBeenCalled = false;

		/**
		 * Adds itself to the multiplication chain
		 */
		void addToMultiplicationChain(std::vector<const MatrixData<T> *> &multiplicationChain) const {
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

		virtual VectorMatrixData<T> virtualMaterialize(unsigned rowOffset, unsigned colOffset, unsigned rows, unsigned columns) const = 0;

		virtual std::vector<const MatrixData<T> *> virtualGetChildren() const {
			return std::vector<const MatrixData<T> *>();
		}

		virtual void virtualOptimize() const {
			this->optimize();
		}

		virtual void optimize() const {
			this->optimizeHasBeenCalled = true;
			for (auto &child : this->virtualGetChildren()) {
				child->virtualOptimize();
			}
		}
		virtual void virtualWaitOptimized() const {
			for (auto &child : this->virtualGetChildren()) {
				child->virtualWaitOptimized();
			}
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

		VectorMatrixData(unsigned rows, unsigned columns, std::shared_ptr<std::vector<T>> vector) : MatrixData<T>(rows, columns), vector(vector) {
		}

		VectorMatrixData(unsigned rows, unsigned columns) : MatrixData<T>(rows, columns), vector(std::make_shared<std::vector<T >>(rows * columns)) {
		}

		MATERIALIZE_IMPL

		void set(unsigned row, unsigned col, T t) {
			(*this->vector.get())[row * this->columns() + col] = t;
		}

		VectorMatrixData<T> copy() const {
			//std::cout << "copying" << std::endl;
			return VectorMatrixData<T>(this->rows(), this->columns(), std::make_shared<std::vector<T>>(*this->vector.get()));
		}

		template<class MD>
		static VectorMatrixData<T> toVector(MD matrixData) {
			return matrixData.virtualMaterialize(0, 0, matrixData.rows(), matrixData.columns());
		}

	private:
		T doGet(unsigned row, unsigned col) const {
			return (*this->vector.get())[row * this->columns() + col];
		}
};

/**
 * An abstract class that wraps a MD=MatrixData<T>
 */
template<typename T, class MD>
class SingleMatrixWrapper : public MatrixData<T> {

	protected:
		MD wrapped;

	public:

		SingleMatrixWrapper(MD wrapped, unsigned rows, unsigned columns) : MatrixData<T>(rows, columns), wrapped(wrapped) {
		}

		std::vector<const MatrixData<T> *> virtualGetChildren() const override {
			return {&this->wrapped};
		}
};

/**
 * An abstract class that wraps a MD=MatrixData<T>
 */
template<typename T, class MD1, class MD2>
class BiMatrixWrapper : public MatrixData<T> {

	protected:
		MD1 left;
		MD2 right;

	public:

		BiMatrixWrapper(MD1 left, MD2 right, unsigned rows, unsigned columns) : MatrixData<T>(rows, columns), left(left), right(right) {
		}

		std::vector<const MatrixData<T> *> virtualGetChildren() const override {
			const MatrixData<T> *left = &this->left;
			const MatrixData<T> *right = &this->right;
			return {left, right};
		}
};

/**
 * An abstract class that wraps a vector of MD=MatrixData<T>
 */
template<typename T, class MD>
class MultiMatrixWrapper : public MatrixData<T> {

	protected:
		std::deque<MD> wrapped; // Should work even without move constructor

		std::deque<MD> copyWrapped() const {
			std::deque<MD> ret;
			for (auto &m : this->wrapped) {
				ret.push_back(m.copy());
			}
			return ret;
		}

	public:

		MultiMatrixWrapper(std::deque<MD> wrapped, unsigned rows, unsigned columns) : MatrixData<T>(rows, columns), wrapped(wrapped) {
		}

		std::vector<const MatrixData<T> *> virtualGetChildren() const override {
			std::vector<const MatrixData<T> *> pointers;
			for (auto it = this->wrapped.begin(); it < wrapped.end(); it++) {
				pointers.push_back(&(*it));
			}
			return pointers;
		}
};


/**
 * Implementation of <code>MatrixData</code> that exposes a submatrix of another <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class SubmatrixMD : public SingleMatrixWrapper<T, MD> {

	private:
		unsigned rowOffset, colOffset;

	public:

		SubmatrixMD(unsigned rowOffset, unsigned colOffset, unsigned rows, unsigned columns, MD wrapped)
				: SingleMatrixWrapper<T, MD>(wrapped, rows, columns), rowOffset(rowOffset), colOffset(colOffset) {
			if (rowOffset + rows > wrapped.rows() || colOffset + columns > wrapped.columns()) {
				Utils::error("Illegal bounds");
			}
		}

		MATERIALIZE_IMPL

		void set(unsigned row, unsigned col, T t) {
			this->wrapped.set(row + this->rowOffset, col + this->colOffset, t);
		}

		SubmatrixMD<T, MD> copy() const {
			return SubmatrixMD<T, MD>(this->rowOffset, this->colOffset, this->rows(), this->columns(), this->wrapped.copy());
		}

	private:
		T doGet(unsigned row, unsigned col) const {
			return this->wrapped.get(row + this->rowOffset, col + this->colOffset);
		}
};

/**
 * Implementation of <code>MatrixData</code> that exposes the transposed matrix of another <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class TransposedMD : public SingleMatrixWrapper<T, MD> {

	public:

		explicit TransposedMD(MD wrapped) : SingleMatrixWrapper<T, MD>(wrapped, wrapped.columns(), wrapped.rows()) {
		}

		MATERIALIZE_IMPL

		void set(unsigned row, unsigned col, T t) {
			this->wrapped.set(col, row, t);
		}

		TransposedMD<T, MD> copy() const {
			return TransposedMD<T, MD>(this->wrapped.copy());
		}

	private:
		T doGet(unsigned row, unsigned col) const {
			return this->wrapped.get(col, row);
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes the diagonal vector of another squared <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class DiagonalMD : public SingleMatrixWrapper<T, MD> {
	public:

		explicit DiagonalMD(MD wrapped) : SingleMatrixWrapper<T, MD>(wrapped, wrapped.rows(), 1) {
			if (wrapped.rows() != wrapped.columns()) {
				Utils::error("diagonal() can only be called on squared matrices");
			}
		}

		MATERIALIZE_IMPL

		void set(unsigned row, unsigned col, T t) {
			this->wrapped.set(row, row, t);
		}

		DiagonalMD<T, MD> copy() const {
			return DiagonalMD<T, MD>(this->wrapped.copy());
		}

	private:
		T doGet(unsigned row, unsigned col) const {
			return this->wrapped.get(row, row);
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes a square diagonal matrix of another vector <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T, class MD>
class DiagonalMatrixMD : public SingleMatrixWrapper<T, MD> {
	public:

		explicit DiagonalMatrixMD(MD wrapped) : SingleMatrixWrapper<T, MD>(wrapped, wrapped.rows(), wrapped.rows()) {
			if (wrapped.columns() != 1) {
				Utils::error("diagonalMatrix() can only be called on vectors (nx1 matrices)");
			}
		}

		MATERIALIZE_IMPL

		DiagonalMatrixMD<T, MD> copy() const {
			return DiagonalMatrixMD<T, MD>(this->wrapped.copy());
		}

	private:
		T doGet(unsigned row, unsigned col) const {
			if (row == col) {
				return this->wrapped.get(row, 0);
			} else {
				return 0;
			}
		}

};

/**
 * Given a vector of matrices, creates a new matrix composed by the concatenation of the given matrices.
 *
 * Let's suppose A, B, C and D are 2x2 matrices.
 * If I crate a ConcatenationMD((A,B,C,D), 4, 4), I will obtained a 4x4 matrix, whose blocks are:
 *
 * A|B
 * C|D
 *
 * @tparam T
 * @tparam MD
 */
template<typename T, class MD>
class ConcatenationMD : public MultiMatrixWrapper<T, MD> {
	public:
		explicit ConcatenationMD(std::deque<MD> blocks, unsigned rows, unsigned columns) :
				MultiMatrixWrapper<T, MD>(blocks, rows, columns) {
			//Checking that all the blocks have the same size
			unsigned blockRows = this->getRowsOfBlocks();
			unsigned blockCols = this->getColumnsOfBlocks();
			for (auto &block: blocks) {
				if (block.rows() != blockRows || block.columns() != blockCols) {
					Utils::error("All the matrices must be of the same size!");
				}
			}
			if (rows % blockRows != 0) {
				Utils::error("The number of rows (" + std::to_string(rows) + ") must be a multiple of the number of rows of the blocks (" +
							 std::to_string(blockRows) + ")!");
			} else if (columns % blockCols != 0) {
				Utils::error("The number of cols (" + std::to_string(columns) + ") must be a multiple of the number of cols of the blocks (" +
							 std::to_string(blockCols) + ")!");
			} else if ((rows / blockRows) * (columns / blockCols) != blocks.size()) {
				Utils::error("The number of blocks (" + std::to_string(blocks.size()) + ") is not enough to cover the whole matrix");
			}
		}

		/**
		 * @return the number of vertical blocks
		 */
		unsigned getNumberOfColumnBlocks() const { return this->columns() / this->getColumnsOfBlocks(); }

		/**
		 * @return the number of horizontal blocks
		 */
		unsigned getNumberOfRowBlocks() const { return this->rows() / this->getRowsOfBlocks(); }

		/**
		 * @return the number of rows of each block
		 */
		unsigned getRowsOfBlocks() const { return this->wrapped[0].rows(); }

		/**
		 * @return the number of columns of each block
		 */
		unsigned getColumnsOfBlocks() const { return this->wrapped[0].columns(); }

		MATERIALIZE_IMPL

		DiagonalMatrixMD<T, MD> copy() const {
			return ConcatenationMD<T, MD>(this->copyWrapped());
		}

	private:

		T doGet(unsigned row, unsigned col) const {
			unsigned blockRows = this->getRowsOfBlocks();
			unsigned blockCols = this->getColumnsOfBlocks();
			unsigned blockRowIndex = row / blockRows;
			unsigned blockColIndex = col / blockCols;
			unsigned blockIndex = blockRowIndex * this->getNumberOfColumnBlocks() + blockColIndex;
			return this->wrapped[blockIndex].get(row % blockRows, col % blockCols);
		}

};

/**
 * Resize the given matrix to a new matrix of the given size.
 * Elements outside of the bounds of the given matrices are 0.
 */
template<typename T, class MD>
class ResizerMD : public SingleMatrixWrapper<T, MD> {
	public:
		ResizerMD(MD wrapped, unsigned rows, unsigned columns) : SingleMatrixWrapper<T, MD>(wrapped, rows, columns) {
		}

		MATERIALIZE_IMPL

		ResizerMD<T, MD> copy() const {
			return ResizerMD<T, MD>(this->wrapped.copy(), this->rows(), this->columns());
		}

	private:
		T doGet(unsigned row, unsigned col) const {
			if (row < this->wrapped.rows() && col < this->wrapped.columns()) {
				return this->wrapped.get(row, col);
			} else {
				return 0;
			}
		}
};

template<typename T, class MD>
class MatrixCaster : public MatrixData<T> {

	protected:
		MD wrapped;

	public:

		MatrixCaster(MD wrapped) : MatrixData<T>(wrapped.rows(), wrapped.columns()), wrapped(wrapped) {
		}

		void virtualWaitOptimized() const override {
			this->wrapped.virtualWaitOptimized();
		}

		MATERIALIZE_IMPL

		MatrixCaster<T, MD> copy() const {
			return MatrixCaster<T, MD>(this->wrapped);
		}

	public:

		void optimize() const override {
			this->wrapped.optimize();
		}

		void virtualOptimize() const override {
			this->optimize();
		}

		std::vector<const MatrixData<T> *> virtualGetChildren() const override {
			//I cannot return wrapper, since it's of another type
			//return {&this->wrapped};
			return std::vector<const MatrixData<T> *>();
		}

	private:
		T doGet(unsigned row, unsigned col) const {
			return this->wrapped.get(row, col);
		}
};

#endif //MATRIX_MATRIXDATA_H
