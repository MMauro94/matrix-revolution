#ifndef MATRIX_MATRIXCELL_H
#define MATRIX_MATRIXCELL_H

#include <memory>
#include "MatrixData.h"

template<typename T, class MD>
class MatrixCell {
	private:

		std::shared_ptr<MD> data;
		unsigned int row, col;


	public:

		MatrixCell(const std::shared_ptr<MD> &data, unsigned int row, unsigned int col) : row(row), col(col), data(data) {
			if (row < 0 || row >= data->rows()) {
				throw "Row out of bounds";
			} else if (col < 0 || col >= data->columns()) {
				throw "Column out of bounds";
			}
		}

		/** Deleted because it would have allowed to make a <code>const MatrixCell&lt;T&gt;</code> non constant */
		MatrixCell(const MatrixCell<T, MD> &) = delete; //Copy constructor

		MatrixCell(MatrixCell<T, MD> &&) noexcept = default; //Move constructor



		MatrixCell<T, MD> &operator=(T const &obj) {
			this->data->set(this->row, this->col, obj);
			return *this;
		}

		operator const T() const {
			return this->data->get(this->row, this->col);
		}

};

#endif //MATRIX_MATRIXCELL_H
