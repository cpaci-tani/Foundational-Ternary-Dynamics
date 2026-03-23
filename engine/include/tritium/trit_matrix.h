#pragma once
// tritium/trit_matrix.h — Row-major trit matrix with mat-vec and mat-mat operations
//
// Each row is a TritVector. Results of matrix operations are integer-valued
// (since sums of trit products are integers, not trits).

#include "trit_vector.h"
#include <vector>
#include <cassert>

namespace tritium {

class TritMatrix {
    std::vector<TritVector> rows_;
    size_t rows_count_;
    size_t cols_count_;

public:
    TritMatrix() : rows_count_(0), cols_count_(0) {}

    TritMatrix(size_t rows, size_t cols)
        : rows_(rows, TritVector(cols)), rows_count_(rows), cols_count_(cols) {}

    // Construct from 2D array of trits
    TritMatrix(const Trit* data, size_t rows, size_t cols)
        : TritMatrix(rows, cols) {
        for (size_t r = 0; r < rows; ++r)
            for (size_t c = 0; c < cols; ++c)
                rows_[r].set(c, data[r * cols + c]);
    }

    size_t rows() const { return rows_count_; }
    size_t cols() const { return cols_count_; }

    // Element access
    Trit get(size_t r, size_t c) const { return rows_[r][c]; }
    void set(size_t r, size_t c, Trit t) { rows_[r].set(c, t); }

    // Row access
    const TritVector& row(size_t r) const { return rows_[r]; }
    TritVector& row(size_t r) { return rows_[r]; }

    // ========================================================================
    // Matrix-vector multiply: result[i] = row[i] . vec (returns int vector)
    // ========================================================================
    std::vector<int> matvec(const TritVector& v) const {
        assert(v.size() == cols_count_);
        std::vector<int> result(rows_count_);
        for (size_t r = 0; r < rows_count_; ++r)
            result[r] = rows_[r].dot(v);
        return result;
    }

    // ========================================================================
    // Matrix-matrix multiply: C[i][j] = Σ_k A[i][k] * B[k][j]
    // ========================================================================
    // Returns integer matrix since sums of trit products are integers.
    // For efficiency, we transpose B and use row-wise dot products.
    std::vector<std::vector<int>> matmul(const TritMatrix& B) const {
        assert(cols_count_ == B.rows_count_);

        // Transpose B for row-wise dot products
        TritMatrix BT = B.transpose();

        std::vector<std::vector<int>> result(rows_count_,
            std::vector<int>(B.cols_count_, 0));

        for (size_t i = 0; i < rows_count_; ++i)
            for (size_t j = 0; j < B.cols_count_; ++j)
                result[i][j] = rows_[i].dot(BT.rows_[j]);

        return result;
    }

    // ========================================================================
    // Transpose
    // ========================================================================
    TritMatrix transpose() const {
        TritMatrix result(cols_count_, rows_count_);
        for (size_t r = 0; r < rows_count_; ++r)
            for (size_t c = 0; c < cols_count_; ++c)
                result.set(c, r, get(r, c));
        return result;
    }
};

} // namespace tritium
