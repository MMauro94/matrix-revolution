#ifndef MATRIX_MATRIXITERATOR_H
#define MATRIX_MATRIXITERATOR_H

#include <memory>
#include "MatrixData.h"

/**
 * Base iterator with common methods
 * @tparam T type of data
 */
template<typename T, class MD>
class BaseMatrixIterator {
	protected:
		std::shared_ptr<MD> data;
		unsigned int row, col;

	public:
		BaseMatrixIterator(const std::shared_ptr<MD> &data, unsigned int row, unsigned int col) : data(data), row(row), col(col) {}

		T operator*() {
			return data->get(row, col);
		}

		bool operator==(const BaseMatrixIterator<T, MD> &other) const {
			return row == other.row && col == other.col && data == other.data;
		}

		bool operator!=(const BaseMatrixIterator<T, MD> &other) const {
			return !(*this == other);
		}
};

/**
 * Iterator that iterates in row-major order
 * @tparam T type of data
 */
template<typename T, class MD>
class MatrixRowMajorIterator : public BaseMatrixIterator<T, MD> {
	public:
		MatrixRowMajorIterator(const std::shared_ptr<MD> &data, unsigned int row, unsigned int col) : BaseMatrixIterator<T, MD>(data, row,
																																col) {}

		void operator++() {
			if (this->col + 1 >= this->data->columns()) {
				this->row++;
				this->col = 0;
			} else {
				this->col++;
			}
		}

		void operator--() {
			if (this->col == 0) {
				this->row--;
				this->col = this->data->columns() - 1;
			} else {
				this->col--;
			}
		}

};

/**
 * Iterator that iterates in column-major order
 * @tparam T type of data
 */
template<typename T, class MD>
class MatrixColumnMajorIterator : public BaseMatrixIterator<T, MD> {
	public:
		MatrixColumnMajorIterator(const std::shared_ptr<MD> &data, unsigned int row, unsigned int col) : BaseMatrixIterator<T, MD>(data, row,
																																   col) {}

		void operator++() {
			if (this->row + 1 >= this->data->rows()) {
				this->col++;
				this->row = 0;
			} else {
				this->row++;
			}
		}

		void operator--() {
			if (this->row == 0) {
				this->col--;
				this->row = this->data->rows() - 1;
			} else {
				this->row--;
			}
		}

};


#endif //MATRIX_MATRIXITERATOR_H
