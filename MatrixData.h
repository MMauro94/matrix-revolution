#ifndef MATRIX_MATRIXDATA_H
#define MATRIX_MATRIXDATA_H

#include <memory>
#include <vector>

template<typename T>
class VectorMatrixData;

/**
 * Abstract class that exposes the data of the matrix
 * @tparam T type of the data
 */
template<typename T>
class MatrixData {

	private:
		unsigned int _rows, _columns;

	public:
		MatrixData(unsigned int rows, unsigned int columns) : _rows(rows), _columns(columns) {}

		virtual ~MatrixData() = default;

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
		 * @param row the row index
		 * @param col the column index
		 * @return the value at the given position
		 */
		virtual T get(unsigned int row, unsigned int col) const = 0;

		/**
		 * Sets the new value at the given position. Can throw exception if the operation is not supported.
		 * @param row the row index
		 * @param col the column index
		 * @param t the new value
		 */
		virtual void set(unsigned int row, unsigned int col, T t) {
			throw "Unsupported";
		}

		/**
		 * @return a copy of this data
		 */
		VectorMatrixData<T> copy();
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

		T get(unsigned int row, unsigned int col) const override {
			return this->vector[row * this->columns() + col];
		}

		void set(unsigned int row, unsigned int col, T t) override {
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
template<typename T>
class SubmatrixMD : public MatrixData<T> {

	private:

		std::shared_ptr<MatrixData<T>> wrapped;
		unsigned int rowOffset, colOffset;


	public:

		SubmatrixMD(unsigned int rowOffset, unsigned int colOffset, unsigned int rows, unsigned int columns,
					const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData<T>(rows,
																				   columns),
																	 rowOffset(rowOffset),
																	 colOffset(colOffset),
																	 wrapped(wrapped) {
		}

		T get(unsigned int row, unsigned int col) const override {
			return this->wrapped->get(row + this->rowOffset, col + this->colOffset);
		}

		void set(unsigned int row, unsigned int col, T t) override {
			this->wrapped->set(row + this->rowOffset, col + this->colOffset, t);
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes the transposed matrix of another <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T>
class TransposedMD : public MatrixData<T> {

	private:

		std::shared_ptr<MatrixData<T>> wrapped;

	public:

		explicit TransposedMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData<T>(wrapped->columns(), wrapped->rows()), wrapped(wrapped) {
		}

		T get(unsigned int row, unsigned int col) const override {
			return this->wrapped->get(col, row);
		}

		void set(unsigned int row, unsigned int col, T t) override {
			this->wrapped->set(col, row, t);
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes the diagonal vector of another squared <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T>
class DiagonalMD : public MatrixData<T> {

	private:

		std::shared_ptr<MatrixData<T>> wrapped;

	public:

		explicit DiagonalMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData<T>(wrapped->rows(), 1), wrapped(wrapped) {
		}

		T get(unsigned int row, unsigned int col) const override {
			return this->wrapped->get(row, row);
		}

		void set(unsigned int row, unsigned int col, T t) override {
			this->wrapped->set(row, row, t);
		}

};

/**
 * Implementation of <code>MatrixData</code> that exposes a square diagonal matrix of another vector <code>MatrixData</code>
 * @tparam T type of the data
 */
template<typename T>
class DiagonalMatrixMD : public MatrixData<T> {

	private:

		std::shared_ptr<MatrixData<T>> wrapped;

	public:

		explicit DiagonalMatrixMD(const std::shared_ptr<MatrixData<T>> &wrapped) : MatrixData<T>(wrapped->rows(), wrapped->rows()),
																				   wrapped(wrapped) {
		}

		T get(unsigned int row, unsigned int col) const override {
			if (row == col) {
				return this->wrapped->get(row, 0);
			} else {
				return 0;
			}
		}
};

template<typename I, typename O>
class ReadOnlyCaster : public MatrixData<O> {
	protected:

		std::shared_ptr<MatrixData<I>> wrapped;
	public:

		explicit ReadOnlyCaster(const std::shared_ptr<MatrixData<I>> &wrapped) : MatrixData<O>(wrapped->rows(), wrapped->columns()),
																				 wrapped(wrapped) {
		}

		O get(unsigned int row, unsigned int col) const override {
			return this->wrapped->get(row, col);
		}
};

template<typename I, typename O>
class Caster : public ReadOnlyCaster<I, O> {
	public:

		explicit Caster(const std::shared_ptr<MatrixData<I>> &wrapped) : ReadOnlyCaster<I, O>(wrapped) {
		}

		void set(unsigned int row, unsigned int col, O t) override {
			this->wrapped->set(row, col, t);
		}
};

/**
 * Implementation of <code>MatrixData</code> that exposes the multiplication of the two given matrices
 * @tparam T type of the data
 */
template<typename T>
class MultiplyMatrix : public MatrixData<T> {

	private:
		std::shared_ptr<MatrixData<T>> first;
		std::shared_ptr<MatrixData<T>> second;
		mutable std::shared_ptr<MatrixData<T>> optimized;

	public:

		explicit MultiplyMatrix(const std::shared_ptr<MatrixData<T>> &first, const std::shared_ptr<MatrixData<T>> &second) :
				MatrixData<T>(first->rows(), second->columns()), first(first), second(second) {
		}

		T get(unsigned int row, unsigned int col) const override {
			this->optimize();
			return optimized->get(row, col);
		}

	private:
		void optimize() const {
			if (optimized.get() == NULL) {
				//Step 1: getting the chain of multiplications to perform
				std::vector<std::shared_ptr<MatrixData<T>>> multiplicationChain;
				this->addToMultiplicationChain(multiplicationChain);

				//Step 2: execute the multiplications in the most efficient order, until a single matrix data is left
				while (multiplicationChain.size() > 1) {
					//Step 2a: find the multiplication that reduces the multiplication the most
					auto b = multiplicationChain.begin();
					auto best = b;
					for (++b; b < multiplicationChain.end() - 1; ++b) {
						MatrixData<T> &matrix = **b;
						if (matrix.columns() > (**best).columns()) {
							best = b;
						}
					}
					//Step 2b: performing the multiplication
					MatrixData<T> &bestMatrix = **best;
					MatrixData<T> &followingMatrix = **(best + 1);
					std::cout << "Performing (" << bestMatrix.rows() << "x" << bestMatrix.columns() << ") x (" << followingMatrix.rows() << "x"
							  << followingMatrix.columns() << ")" << std::endl;

					const std::shared_ptr<MatrixData<T>> &multiplied = computeMultiplication(bestMatrix, followingMatrix);

					//Step 2c: Dispatching the optimization to children, so useful calculations are not lost
					this->dispatchOptimized(bestMatrix, followingMatrix, multiplied);

					//Step 2d: replacing the two matrices in the chain with the computed product
					(*best) = multiplied;
					multiplicationChain.erase(best + 1);
				}
				this->optimized = multiplicationChain[0];
			}
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

		void
		dispatchOptimized(MatrixData<T> &first, MatrixData<T> &second,
						  const std::shared_ptr<MatrixData<T>> &multiplied) const {
			if (&first == this->first.get() && &second == this->second.get()) {
				this->optimized = multiplied;
			} else {
				if (MultiplyMatrix<T> *casted = dynamic_cast<MultiplyMatrix<T> *>(this->first.get())) {
					casted->dispatchOptimized(first, second, multiplied);
				}
				if (MultiplyMatrix<T> *casted = dynamic_cast<MultiplyMatrix<T> *>(this->second.get())) {
					casted->dispatchOptimized(first, second, multiplied);
				}
			}
		}

		void addToMultiplicationChain(std::vector<std::shared_ptr<MatrixData<T>>> &multiplicationChain) const {
			if (this->optimized.get() != NULL) {
				throw "Matrix is optimized!";
			}
			addToMultiplicationChain(multiplicationChain, this->first);
			addToMultiplicationChain(multiplicationChain, this->second);
		}

		void addToMultiplicationChain(std::vector<std::shared_ptr<MatrixData<T>>> &multiplicationChain, std::shared_ptr<MatrixData<T>> m) const {
			MultiplyMatrix<T> *casted = dynamic_cast<MultiplyMatrix<T> *>(m.get());
			if (casted == NULL || casted->optimized.get() != NULL) {
				multiplicationChain.push_back(m);
			} else {
				casted->addToMultiplicationChain(multiplicationChain);
			}
		}
};

/**
 * Implementation of <code>MatrixData</code> that exposes the sum of the two given matrices
 * @tparam T type of the data
 */
template<typename T, typename U>
class SumMatrix : public MatrixData<decltype(T() * U())> {

	private:
		std::shared_ptr<MatrixData<T>> first;
		std::shared_ptr<MatrixData<U>> second;

	public:

		explicit SumMatrix(const std::shared_ptr<MatrixData<T>> &first, const std::shared_ptr<MatrixData<U>> &second) :
				MatrixData<decltype(T() * U())>(first->rows(), second->columns()), first(first), second(second) {
		}

		decltype(T() * U()) get(unsigned int row, unsigned int col) const override {
			return this->first->get(row, col) + this->second->get(row, col);
		}

};

#endif //MATRIX_MATRIXDATA_H
