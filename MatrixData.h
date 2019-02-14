#ifndef MATRIX_MATRIXDATA_H
#define MATRIX_MATRIXDATA_H

#include <memory>
#include <vector>
#include <algorithm>
#include <tuple>
#include "ThreadPool.h"

template<typename T>
class VectorMatrixData;

template<typename T, class MD1, class MD2>
class MultiplyMatrix;

//This macro is used to add methods to implementations of MatrixData, without copy-pasting code.
//It is necessary, since those methods call an inherited non-virtual method
#define COMMON_MATRIX_DATA_METHODS        \
T virtualGet(unsigned row, unsigned col) const override {\
    if(row < 0 || row >= this->rows() || col < 0 || col >= this->columns()){\
        Utils::error("Illegal index");\
    }\
    return this->get(row, col);\
}\
\
VectorMatrixData<T> materialize(unsigned rowOffset, unsigned colOffset, unsigned rows, unsigned columns) const override {\
    if (rowOffset + rows > this->rows() || colOffset + columns > this->columns()) {\
        Utils::error("Illegal bounds");\
    }\
    VectorMatrixData<T> ret(rows, columns);\
    for (unsigned r = 0; r < rows; r++) {\
        for (unsigned c = 0; c < columns; c++) {\
            ret.set(r, c, this->get(r + rowOffset, c + colOffset));\
        }\
    }\
    return ret;\
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

		virtual VectorMatrixData<T> materialize(unsigned rowOffset, unsigned colOffset, unsigned rows, unsigned columns) const = 0;

		virtual void optimize(ThreadPool *threadPool) const {
		}

		virtual void printTree() const {
			this->printDebugTree("", false, false);
		}

		virtual void printDebugTree(const std::string &prefix, bool isLeft, bool hasVirtualBarrier) const {
			std::cout << prefix + (isLeft ? "|" : "\\") + (hasVirtualBarrier ? "!-" : "--") + this->getDebugName() + "\n";
		};

		virtual const std::string getDebugName() const = 0;

		static void
		printDebugChildrenTree(const std::string &prefix, bool isLeft, std::vector<const MatrixData<T> *> children, bool hasVirtualBarrier) {
			// enter the next tree level - left and right branch
			for (auto it = children.begin(); it < children.end(); it++) {
				(*it)->printDebugTree(prefix + (isLeft ? "|   " : "    "), (it + 1) < children.end(), hasVirtualBarrier);
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
		std::string debugName;
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

		COMMON_MATRIX_DATA_METHODS

		void set(unsigned row, unsigned col, T t) {
			(*this->vector.get())[row * this->columns() + col] = t;
		}

		VectorMatrixData<T> copy() const {
			//std::cout << "copying" << std::endl;
			return VectorMatrixData<T>(this->rows(), this->columns(), std::make_shared<std::vector<T>>(*this->vector.get()));
		}

		void setDebugName(std::string debugName) {
			this->debugName = debugName;
		}

		const std::string getDebugName() const override {
			if (this->debugName.empty()) {
				return "?";
			} else {
				return this->debugName;
			}
		}

		template<class MD>
		static VectorMatrixData<T> toVector(MD matrixData) {
			return matrixData.materialize(0, 0, matrixData.rows(), matrixData.columns());
		}
};

/**
 * An abstract class that wraps a MD=MatrixData<T>
 */
template<typename T, class MD>
class SingleMatrixWrapper : public MatrixData<T> {

	protected:
		MD wrapped;
		const std::string wrapName;

	public:

		SingleMatrixWrapper(std::string wrapName, MD wrapped, unsigned rows, unsigned columns) : MatrixData<T>(rows, columns), wrapped(wrapped),
																								 wrapName(wrapName) {
		}

		void optimize(ThreadPool *threadPool) const override {
			this->wrapped.optimize(threadPool);
		}

		void printDebugTree(const std::string &prefix, bool isLeft, bool hasVirtualBarrier) const override {
			MatrixData::printDebugTree(prefix, isLeft, hasVirtualBarrier);
			MatrixData::printDebugChildrenTree(prefix, isLeft, {&this->wrapped}, false);
		};

		const std::string getDebugName() const override {
			return this->wrapName;
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
		const std::string wrapName;

	public:

		BiMatrixWrapper(std::string wrapName, MD1 left, MD2 right, unsigned rows, unsigned columns) : MatrixData<T>(rows, columns),
																									  left(left), right(right), wrapName(wrapName) {
		}

		void optimize(ThreadPool *threadPool) const override {
			this->left.optimize(threadPool);
			this->right.optimize(threadPool);
		}

		void printDebugTree(const std::string &prefix, bool isLeft, bool hasVirtualBarrier) const override {
			MatrixData::printDebugTree(prefix, isLeft, hasVirtualBarrier);
			std::vector<const MatrixData<T> *> children;
			children.push_back(&this->left);
			children.push_back(&this->right);
			MatrixData::printDebugChildrenTree(prefix, isLeft, children, false);
		};

		const std::string getDebugName() const override {
			return this->wrapName;
		}
};

/**
 * An abstract class that wraps a vector of MD=MatrixData<T>
 */
template<typename T, class MD>
class MultiMatrixWrapper : public MatrixData<T> {

	protected:
		std::deque<MD> wrapped; // Should work even without move constructor
		const std::string wrapName;

		std::deque<MD> copyWrapped() const {
			std::deque<MD> ret();
			for (it = this->wrapped.begin(); it != this->wrapped.end(); it++, i++) {
				ret().push_back(it.copy());
			}
			return ret;
		}

	public:

		MultiMatrixWrapper(std::string wrapName, std::deque<MD> wrapped, unsigned rows, unsigned columns) : MatrixData<T>(rows, columns),
																											wrapped(wrapped), wrapName(wrapName) {
		}

		void optimize(ThreadPool *threadPool) const override {
			for (auto it = this->wrapped.begin(); it < this->wrapped.end(); it++) {
				it->optimize(threadPool);
			}
		}

		void printDebugTree(const std::string &prefix, bool isLeft, bool hasVirtualBarrier) const override {
			MatrixData::printDebugTree(prefix, isLeft, hasVirtualBarrier);
			std::vector<const MatrixData<T> *> pointers;
			for (auto it = this->wrapped.begin(); it < wrapped.end(); it++) {
				pointers.push_back(&(*it));
			}
			MatrixData::printDebugChildrenTree(prefix, isLeft, pointers, false);
		};

		const std::string getDebugName() const override {
			return this->wrapName;
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
				: SingleMatrixWrapper<T, MD>("Submatrix", wrapped, rows, columns), rowOffset(rowOffset), colOffset(colOffset) {
			if (rowOffset + rows > wrapped.rows() || colOffset + columns > wrapped.columns()) {
				Utils::error("Illegal bounds");
			}
		}

		T get(unsigned row, unsigned col) const {
			return this->wrapped.get(row + this->rowOffset, col + this->colOffset);
		}

		COMMON_MATRIX_DATA_METHODS

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
class TransposedMD : public SingleMatrixWrapper<T, MD> {

	public:

		explicit TransposedMD(MD wrapped) : SingleMatrixWrapper<T, MD>("Transposed", wrapped, wrapped.columns(), wrapped.rows()) {
		}

		T get(unsigned row, unsigned col) const {
			return this->wrapped.get(col, row);
		}

		COMMON_MATRIX_DATA_METHODS

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
class DiagonalMD : public SingleMatrixWrapper<T, MD> {
	public:

		explicit DiagonalMD(MD wrapped) : SingleMatrixWrapper<T, MD>("Diagonal", wrapped, wrapped.rows(), 1) {
			if (wrapped.rows() != wrapped.columns()) {
				Utils::error("diagonal() can only be called on squared matrices");
			}
		}

		T get(unsigned row, unsigned col) const {
			return this->wrapped.get(row, row);
		}

		COMMON_MATRIX_DATA_METHODS

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
class DiagonalMatrixMD : public SingleMatrixWrapper<T, MD> {
	public:

		explicit DiagonalMatrixMD(MD wrapped) : SingleMatrixWrapper<T, MD>("Diagonal", wrapped.rows(), wrapped.rows()) {
			if (wrapped.columns() != 1) {
				Utils::error("diagonalMatrix() can only be called on vectors (nx1 matrices)");
			}
		}

		T get(unsigned row, unsigned col) const {
			if (row == col) {
				return this->wrapped.get(row, 0);
			} else {
				return 0;
			}
		}

		COMMON_MATRIX_DATA_METHODS

		DiagonalMatrixMD<T, MD> copy() const {
			return DiagonalMatrixMD<T, MD>(this->wrapped.copy());
		}

};

/**
 * Given a vector of matrices, creates a new matrix composed by a concatenation of the given matrices.
 *
 * Let's suppose A, B, C and D are 2x2 matrices.
 * If I crate a MatrixConcatenation((A,B,C,D), 4, 4), I will obtained a 4x4 matrices, whose blocks are:
 *
 * A|B
 * C|D
 *
 * @tparam T
 * @tparam MD
 */
template<typename T, class MD>
class MatrixConcatenation : public MultiMatrixWrapper<T, MD> {
	public:
		explicit MatrixConcatenation(std::deque<MD> blocks, unsigned rows, unsigned columns) :
				MultiMatrixWrapper<T, MD>("Concatenation", blocks, rows, columns) {
			//Checking that all the blocks have the same size
			unsigned blockRows = blocks[0].rows();
			unsigned blockCols = blocks[0].columns();
			for (auto it = blocks.begin(); it != blocks.end(); it++) {
				if (it->rows() != blockRows || it->columns() != blockCols) {
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

		T get(unsigned row, unsigned col) const {
			unsigned blockRows = this->wrapped[0].rows();
			unsigned blockCols = this->wrapped[0].columns();
			unsigned blockRowIndex = row / blockRows;
			unsigned blockColIndex = col / blockCols;
			unsigned blockIndex = blockRowIndex * (this->columns() / blockCols) + blockColIndex;
			return this->wrapped[blockIndex].get(row % blockRows, col % blockCols);
		}

		COMMON_MATRIX_DATA_METHODS

		DiagonalMatrixMD<T, MD> copy() const {
			return MatrixConcatenation<T, MD>(this->copyWrapped());
		}

		void optimize(ThreadPool *threadPool) const override {
			for (auto it = this->wrapped.begin(); it != this->wrapped.end(); it++) {
				it->optimize(threadPool);
			}
		}
};

/**
 * Resize the given matrix to a new matrix of the given size.
 * Elements outside of the bounds of the given matrices are 0.
 */
template<typename T, class MD>
class MatrixResizer : public SingleMatrixWrapper<T, MD> {
	public:
		MatrixResizer(MD wrapped, unsigned rows, unsigned columns) : SingleMatrixWrapper<T, MD>("Resizing", wrapped, rows, columns) {
		}

		T get(unsigned row, unsigned col) const {
			if (row < this->wrapped.rows() && col < this->wrapped.columns()) {
				return this->wrapped.get(row, col);
			} else {
				return 0;
			}
		}

		COMMON_MATRIX_DATA_METHODS

		MatrixResizer<T, MD> copy() const {
			return MatrixResizer<T, MD>(this->wrapped.copy(), this->rows(), this->columns());
		}
};

#endif //MATRIX_MATRIXDATA_H
