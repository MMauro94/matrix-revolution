#ifndef MATRIX_MATRIXCELL_H
#define MATRIX_MATRIXCELL_H

#include <memory>
#include "MatrixData.h"

template<typename T, class MD>
class MatrixCell {
	private:

		MD data;
		unsigned row, col;


	public:

		MatrixCell(MD &data, unsigned row, unsigned col) : row(row), col(col), data(data) {
			if (row < 0 || row >= data.rows()) {
				Utils::error("Row out of bounds");
			} else if (col < 0 || col >= data.columns()) {
				Utils::error("Column out of bounds");
			}
		}

		/** Deleted because it would have allowed to make a <code>const MatrixCell&lt;T&gt;</code> non constant */
		MatrixCell(const MatrixCell<T, MD> &) = delete; //Copy constructor

		//TODO: capire perchè non funziona il  noexcept
		MatrixCell(MatrixCell<T, MD> &&) = default; //Move constructor



		MatrixCell<T, MD> &operator=(T const &obj) {
			this->data.set(this->row, this->col, obj);
			return *this;
		}

		operator const T() const {
			return this->data.get(this->row, this->col);
		}

};

#endif //MATRIX_MATRIXCELL_H
