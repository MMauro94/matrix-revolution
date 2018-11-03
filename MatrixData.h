#ifndef MATRIX_MATRIXDATA_H
#define MATRIX_MATRIXDATA_H

template<typename T>
class MatrixData {
	private:
		int rows, columns;
		int refCount = 0;

	protected:
		MatrixData(int rows, int columns) {
			this->rows = rows;
			this->columns = columns;
		}

	public:

		virtual T get(int row, int column) = 0;

		virtual void set(int row, int column, T obj) {
			throw std::exception("Unsupported operation");
		}

		virtual ~MatrixData() {
		}

		void incrementRefCount() {
			refCount++;
		}

		bool decrementRefCount() {
			refCount--;
			return refCount == 0;
		}

		int getRows() const {
			return rows;
		}

		int getColumns() const {
			return columns;
		}
};

#endif //MATRIX_MATRIXDATA_H
