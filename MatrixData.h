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

/**
 * Abstract class that exposes the data of the matrix
 * @tparam T type of the data
 */
template<typename T>
class MatrixData {

	private:
		unsigned _rows, _columns;
		const char *debugName = nullptr;

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

		void setDebugName(const char *debugName) {
			this->debugName = debugName;
		}

		virtual std::string getDebugName(bool reversePolishNotation) const {
			if (this->debugName == NULL) {
				std::cout << "Debug name not set!" << std::endl;
				Utils::error("Debug name not set");
			}
			return this->debugName;
		}

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

		virtual void printDebugTree(const std::string &prefix, bool isLeft) const {
			std::cout << prefix + (isLeft ? "|--" : "\\--") + this->getDebugName(true) + "\n";
		}

		virtual void waitOptimized() const {}

		virtual void optimize(ThreadPool *threadPool) const {
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
			if (rowOffset + rows > wrapped.rows() || colOffset + columns > wrapped.columns()) {
				Utils::error("Illegal bounds");
			}
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
			if (wrapped.rows() != wrapped.columns()) {
				Utils::error("diagonal() can only be called on squared matrices");
			}
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

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

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
class MatrixConcatenation : public MatrixData<T> {
	private:
		std::vector<MD> blocks;

	public:
		explicit MatrixConcatenation(std::vector<MD> blocks, unsigned rows, unsigned columns) :
				MatrixData<T>(rows, columns), blocks(blocks) {
			//Checking that all the blocks have the same size
			unsigned blockRows = blocks[0].rows();
			unsigned blockCols = blocks[0].columns();
			for (it = blocks.begin(); it != blocks.end(); it++, i++) {
				if (it.rows() != blockRows || it.columns() != blockCols) {
					Utils::error("All the matrices must be of the same size!");
				}
			}
			if (rows % blockRows != 0) {
				Utils::error("The number of rows (" + rows + ") must be a multiple of the number of rows of the blocks (" + blockRows + ")!");
			} else if (columns % blockCols != 0) {
				Utils::error("The number of cols (" + rows + ") must be a multiple of the number of cols of the blocks (" + blockCols + ")!");
			} else if ((rows / blockRows) * (columns / blockCols) != blocks.size()) {
				Utils::error("The number of blocks (" + blocks.size() + ") is not enough to cover the whole matrix");
			}
		}

		T get(unsigned row, unsigned col) const {
			unsigned blockRows = this->blocks[0].rows();
			unsigned blockCols = this->blocks[0].columns();
			unsigned blockRowIndex = row / blockRows;
			unsigned blockColIndex = col / blockCols;
			unsigned blockIndex = blockRowIndex * (rows / blockRows) + blockColIndex;
			return this->blocks[blockIndex].get(row % blockRows, col % blockCols);
		}

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

		DiagonalMatrixMD<T, MD> copy() const {
			std::vector newBlocks();
			for (it = this->blocks.begin(); it != this->blocks.end(); it++, i++) {
				newBlocks().push_back(it.copy());
			}
			return MatrixConcatenation<T, MD>(newBlocks);
		}

		void optimize(ThreadPool *threadPool) const override {
			for (it = this->blocks.begin(); it != this->blocks.end(); it++, i++) {
				it.optimize(threadPool);
			}
		}
};

/**
 * Resize the given matrix to a new matrix of the given size.
 * Elements outside of the bounds of the given matrices are 0.
 */
template<typename T, class MD>
class MatrixResizer : public MatrixData<T> {
	private:
		MD wrapped;
	public:
		explicit MatrixResizer(MD wrapped, unsigned rows, unsigned columns) : MatrixData<T>(rows, columns), wrapped(wrapped) {
		}

		T get(unsigned row, unsigned col) const {
			if (row < this->wrapped.rows() && col < this->wrapped.columns()) {
				return this->wrapped.get(row, col);
			} else {
				return 0;
			}
		}

		T virtualGet(unsigned row, unsigned col) const override {
			return this->get(row, col);
		}

		MatrixResizer<T, MD> copy() const {
			return MatrixResizer<T, MD>(this->wrapped.copy(), this->rows(), this->columns());
		}
};

#endif //MATRIX_MATRIXDATA_H
