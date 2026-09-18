#pragma once
/**
 * @file square_matrix_algebra.h
 * @brief Fixed-size dense square-matrix helpers (multiply, transpose,
 *        subtract, max-abs, Gauss-Jordan rank, partial-pivot determinant)
 *        shared by the observer-only genesis symplectic analyses.
 *
 * Extracted verbatim from genesis_cubic_canonical_form.cpp,
 * genesis_environment_feedback.cpp and genesis_minimal_bath.cpp, each of which
 * carried its own identical copy (the environment-feedback copy was
 * monomorphised to a fixed 6x6 type). Arithmetic, operation order, pivot
 * selection and tolerances are unchanged from those copies: the engine pins
 * /fp:precise (MSVC) and -ffp-contract=off (gcc) for bit-reproducible gates, so
 * these routines must stay numerically identical, not merely equivalent.
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <utility>

namespace ftd::eft::linalg {

template <std::size_t N>
using Matrix = std::array<std::array<double, N>, N>;

template <std::size_t N>
Matrix<N> multiply(const Matrix<N>& lhs, const Matrix<N>& rhs) {
  Matrix<N> result{};
  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < N; ++j) {
      for (std::size_t k = 0; k < N; ++k) result[i][j] += lhs[i][k] * rhs[k][j];
    }
  }
  return result;
}

template <std::size_t N>
Matrix<N> transpose(const Matrix<N>& value) {
  Matrix<N> result{};
  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < N; ++j) result[i][j] = value[j][i];
  }
  return result;
}

template <std::size_t N>
Matrix<N> subtract(const Matrix<N>& lhs, const Matrix<N>& rhs) {
  Matrix<N> result{};
  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < N; ++j) result[i][j] = lhs[i][j] - rhs[i][j];
  }
  return result;
}

template <std::size_t N>
double max_abs(const Matrix<N>& value) {
  double result = 0.0;
  for (const auto& row : value) {
    for (double entry : row) result = std::max(result, std::abs(entry));
  }
  return result;
}

/// Gauss-Jordan elimination with partial pivoting; a column whose best pivot
/// is <= tolerance in magnitude is skipped without incrementing the rank.
template <std::size_t N>
int numerical_rank(Matrix<N> value, double tolerance = 1e-10) {
  int rank = 0;
  for (std::size_t column = 0; column < N && rank < static_cast<int>(N);
       ++column) {
    int pivot = rank;
    for (int row = rank + 1; row < static_cast<int>(N); ++row) {
      if (std::abs(value[static_cast<std::size_t>(row)][column])
          > std::abs(value[static_cast<std::size_t>(pivot)][column])) {
        pivot = row;
      }
    }
    if (std::abs(value[static_cast<std::size_t>(pivot)][column]) <= tolerance) {
      continue;
    }
    std::swap(value[static_cast<std::size_t>(rank)],
              value[static_cast<std::size_t>(pivot)]);
    const double divisor = value[static_cast<std::size_t>(rank)][column];
    for (std::size_t j = column; j < N; ++j) {
      value[static_cast<std::size_t>(rank)][j] /= divisor;
    }
    for (int row = 0; row < static_cast<int>(N); ++row) {
      if (row == rank) continue;
      const double factor = value[static_cast<std::size_t>(row)][column];
      for (std::size_t j = column; j < N; ++j) {
        value[static_cast<std::size_t>(row)][j]
            -= factor * value[static_cast<std::size_t>(rank)][j];
      }
    }
    ++rank;
  }
  return rank;
}

/// Gaussian elimination with partial pivoting; returns exactly 0.0 when the
/// best pivot of any column falls below 1e-15 in magnitude.
template <std::size_t N>
double determinant(Matrix<N> value) {
  double result = 1.0;
  int sign = 1;
  for (std::size_t column = 0; column < N; ++column) {
    std::size_t pivot = column;
    for (std::size_t row = column + 1; row < N; ++row) {
      if (std::abs(value[row][column]) > std::abs(value[pivot][column])) pivot = row;
    }
    if (std::abs(value[pivot][column]) < 1e-15) return 0.0;
    if (pivot != column) {
      std::swap(value[pivot], value[column]);
      sign = -sign;
    }
    const double diagonal = value[column][column];
    result *= diagonal;
    for (std::size_t row = column + 1; row < N; ++row) {
      const double factor = value[row][column] / diagonal;
      for (std::size_t j = column + 1; j < N; ++j) {
        value[row][j] -= factor * value[column][j];
      }
    }
  }
  return static_cast<double>(sign) * result;
}

}  // namespace ftd::eft::linalg
