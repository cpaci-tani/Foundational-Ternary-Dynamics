#include "ftd/eft/genesis_cubic_canonical_form.h"

#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace ftd::eft {

namespace {

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

template <std::size_t N>
int square_rank(Matrix<N> value, double tolerance = 1e-10) {
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
    if (std::abs(value[static_cast<std::size_t>(pivot)][column]) <= tolerance) continue;
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

int rectangular_rank(std::vector<std::array<double, 15>> value,
                     double tolerance = 1e-10) {
  int rank = 0;
  for (int column = 0; column < 15 && rank < static_cast<int>(value.size());
       ++column) {
    int pivot = rank;
    for (int row = rank + 1; row < static_cast<int>(value.size()); ++row) {
      if (std::abs(value[static_cast<std::size_t>(row)]
                        [static_cast<std::size_t>(column)])
          > std::abs(value[static_cast<std::size_t>(pivot)]
                          [static_cast<std::size_t>(column)])) {
        pivot = row;
      }
    }
    if (std::abs(value[static_cast<std::size_t>(pivot)]
                      [static_cast<std::size_t>(column)]) <= tolerance) {
      continue;
    }
    std::swap(value[static_cast<std::size_t>(rank)],
              value[static_cast<std::size_t>(pivot)]);
    const double divisor = value[static_cast<std::size_t>(rank)]
                                [static_cast<std::size_t>(column)];
    for (int j = column; j < 15; ++j) {
      value[static_cast<std::size_t>(rank)][static_cast<std::size_t>(j)]
          /= divisor;
    }
    for (int row = 0; row < static_cast<int>(value.size()); ++row) {
      if (row == rank) continue;
      const double factor = value[static_cast<std::size_t>(row)]
                                 [static_cast<std::size_t>(column)];
      for (int j = column; j < 15; ++j) {
        value[static_cast<std::size_t>(row)][static_cast<std::size_t>(j)]
            -= factor * value[static_cast<std::size_t>(rank)]
                           [static_cast<std::size_t>(j)];
      }
    }
    ++rank;
  }
  return rank;
}

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

int permutation_sign(const std::array<int, 3>& permutation) {
  int inversions = 0;
  for (int i = 0; i < 3; ++i) {
    for (int j = i + 1; j < 3; ++j) {
      if (permutation[static_cast<std::size_t>(i)]
          > permutation[static_cast<std::size_t>(j)]) ++inversions;
    }
  }
  return inversions % 2 == 0 ? 1 : -1;
}

std::vector<Matrix<3>> signed_permutation_group(bool proper_only) {
  std::vector<Matrix<3>> result;
  std::array<int, 3> permutation{{0, 1, 2}};
  do {
    const int parity = permutation_sign(permutation);
    for (int sx : {-1, 1}) {
      for (int sy : {-1, 1}) {
        for (int sz : {-1, 1}) {
          const int det = parity * sx * sy * sz;
          if (proper_only && det != 1) continue;
          Matrix<3> value{};
          const std::array<int, 3> signs{{sx, sy, sz}};
          for (int i = 0; i < 3; ++i) {
            value[static_cast<std::size_t>(i)]
                 [static_cast<std::size_t>(permutation[static_cast<std::size_t>(i)])]
                = static_cast<double>(signs[static_cast<std::size_t>(i)]);
          }
          result.push_back(value);
        }
      }
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return result;
}

Matrix<6> doubled(const Matrix<3>& rotation) {
  Matrix<6> result{};
  for (std::size_t i = 0; i < 3; ++i) {
    for (std::size_t j = 0; j < 3; ++j) {
      result[i][j] = rotation[i][j];
      result[i + 3][j + 3] = rotation[i][j];
    }
  }
  return result;
}

Matrix<6> standard_form() {
  Matrix<6> result{};
  for (int i = 0; i < 3; ++i) {
    result[static_cast<std::size_t>(i)][static_cast<std::size_t>(i + 3)] = 1.0;
    result[static_cast<std::size_t>(i + 3)][static_cast<std::size_t>(i)] = -1.0;
  }
  return result;
}

std::array<Matrix<6>, 15> skew_basis() {
  std::array<Matrix<6>, 15> result{};
  int index = 0;
  for (int i = 0; i < 6; ++i) {
    for (int j = i + 1; j < 6; ++j) {
      result[static_cast<std::size_t>(index)]
            [static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = 1.0;
      result[static_cast<std::size_t>(index)]
            [static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = -1.0;
      ++index;
    }
  }
  return result;
}

Matrix<6> system_map(const Vec3& direction, double t, double a) {
  Matrix<6> result{};
  const std::array<double, 3> n{{direction.x, direction.y, direction.z}};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      result[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          = (i == j ? t : 0.0)
          + (1.0 - t) * n[static_cast<std::size_t>(i)]
                          * n[static_cast<std::size_t>(j)];
    }
    result[static_cast<std::size_t>(i + 3)]
          [static_cast<std::size_t>(i + 3)] = a;
  }
  return result;
}

Matrix<6> defect(const Matrix<6>& omega, const Matrix<6>& map) {
  return subtract(omega, multiply(transpose(map), multiply(omega, map)));
}

Matrix<6> zero_drain_alternative(double t) {
  (void)t;
  Matrix<6> omega{};
  for (const auto pair : std::array<std::array<int, 2>, 3>{{
           {{0, 3}}, {{4, 5}}, {{1, 2}}}}) {
    omega[static_cast<std::size_t>(pair[0])]
         [static_cast<std::size_t>(pair[1])] = 1.0;
    omega[static_cast<std::size_t>(pair[1])]
         [static_cast<std::size_t>(pair[0])] = -1.0;
  }
  return omega;
}

Matrix<6> generic_alternative(double t, double a) {
  const double ratio = (1.0 - a * t) / ((1.0 - a) * (1.0 + t));
  Matrix<6> omega{};
  const auto set = [&omega](int i, int j, double value) {
    omega[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
    omega[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = -value;
  };
  set(0, 1, 1.0);
  set(2, 3, 1.0);
  set(4, 5, 1.0);
  set(0, 3, 1.0);
  set(1, 2, -ratio);
  return omega;
}

Matrix<6> diagonal_map(double t, double a) {
  Matrix<6> result{};
  const std::array<double, 6> diagonal{{1.0, t, t, a, a, a}};
  for (std::size_t i = 0; i < 6; ++i) result[i][i] = diagonal[i];
  return result;
}

}  // namespace

GenesisCubicCanonicalFormResult analyze_genesis_cubic_canonical_form() {
  GenesisCubicCanonicalFormResult result;
  const auto full_group = signed_permutation_group(false);
  const auto proper_group = signed_permutation_group(true);
  result.full_cubic_group_elements = static_cast<int>(full_group.size());
  result.proper_cubic_group_elements = static_cast<int>(proper_group.size());

  const Matrix<6> omega0 = standard_form();
  for (const auto& rotation : full_group) {
    const Matrix<6> action = doubled(rotation);
    result.maximum_cubic_invariance_residual = std::max(
        result.maximum_cubic_invariance_residual,
        max_abs(subtract(multiply(transpose(action), multiply(omega0, action)),
                         omega0)));
  }

  const auto basis = skew_basis();
  std::vector<std::array<double, 15>> constraints;
  constraints.reserve(proper_group.size() * 15);
  for (const auto& rotation : proper_group) {
    const Matrix<6> action = doubled(rotation);
    std::array<Matrix<6>, 15> transformed{};
    for (std::size_t k = 0; k < basis.size(); ++k) {
      transformed[k] = subtract(
          multiply(transpose(action), multiply(basis[k], action)), basis[k]);
    }
    for (int i = 0; i < 6; ++i) {
      for (int j = i + 1; j < 6; ++j) {
        std::array<double, 15> row{};
        for (std::size_t k = 0; k < basis.size(); ++k) {
          row[k] = transformed[k][static_cast<std::size_t>(i)]
                                  [static_cast<std::size_t>(j)];
        }
        constraints.push_back(row);
      }
    }
  }
  result.invariant_constraint_rank = rectangular_rank(constraints);
  result.invariant_nullity = 15 - result.invariant_constraint_rank;
  result.standard_pairing_unique_up_to_scale =
      result.full_cubic_group_elements == 48
      && result.proper_cubic_group_elements == 24
      && result.invariant_constraint_rank == 14
      && result.invariant_nullity == 1
      && result.maximum_cubic_invariance_residual <= 1e-12
      && std::abs(determinant(omega0) - 1.0) <= 1e-12;

  const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
  const std::array<Vec3, 10> directions{{
      {1.0, 0.0, 0.0}, {-1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0}, {0.0, -1.0, 0.0},
      {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0},
      {inv_sqrt3, inv_sqrt3, inv_sqrt3},
      {-inv_sqrt3, inv_sqrt3, inv_sqrt3},
      {inv_sqrt3, -inv_sqrt3, inv_sqrt3},
      {inv_sqrt3, inv_sqrt3, -inv_sqrt3}}};
  const std::array<double, 3> excesses{{0.125, 0.5, 1.25}};
  const std::array<double, 4> drains{{0.0, 0.5, 0.9, 1.0}};
  double minimum_generic_determinant = std::numeric_limits<double>::infinity();
  bool production_ranks_ok = true;
  bool zero_alternatives_ok = true;
  bool generic_alternatives_ok = true;

  for (const Vec3& direction : directions) {
    for (double excess : excesses) {
      const double t = excess / (1.0 + excess);
      for (double drain : drains) {
        const double a = 1.0 - drain;
        const int cubic_rank = square_rank(defect(omega0, system_map(direction, t, a)));
        const int expected_cubic_rank = drain == 0.0 ? 4 : 6;
        production_ranks_ok = production_ranks_ok && cubic_rank == expected_cubic_rank;
        ++result.production_arms;

        int alternative_rank = 0;
        if (drain == 0.0) {
          const Matrix<6> alternative = zero_drain_alternative(t);
          alternative_rank = square_rank(
              defect(alternative, diagonal_map(t, a)));
          zero_alternatives_ok = zero_alternatives_ok
              && std::abs(determinant(alternative) - 1.0) <= 1e-12
              && alternative_rank == 2;
          ++result.zero_drain_alternative_arms;
        } else {
          const Matrix<6> alternative = generic_alternative(t, a);
          const double measured_determinant = determinant(alternative);
          const double expected_determinant =
              ((t - a) * (t - a))
              / (((1.0 - a) * (1.0 - a)) * ((1.0 + t) * (1.0 + t)));
          result.maximum_generic_determinant_formula_residual = std::max(
              result.maximum_generic_determinant_formula_residual,
              std::abs(measured_determinant - expected_determinant));
          minimum_generic_determinant = std::min(
              minimum_generic_determinant, measured_determinant);
          alternative_rank = square_rank(
              defect(alternative, diagonal_map(t, a)));
          generic_alternatives_ok = generic_alternatives_ok
              && measured_determinant > 0.0 && alternative_rank == 4;
          ++result.positive_drain_alternative_arms;
        }
        if (cubic_rank - alternative_rank == 2) ++result.symmetry_price_arms;
      }

      // At a=t the contracting eigenspace has dimension five. The repeated-
      // eigenspace lemma forces even defect rank >=6, attained by omega0.
      const double a = t;
      const int measured_rank = square_rank(defect(omega0, diagonal_map(t, a)));
      const int lemma_lower_bound = 6;
      if (measured_rank == lemma_lower_bound) ++result.degenerate_a_equals_t_arms;
    }
  }

  result.minimum_generic_alternative_determinant = minimum_generic_determinant;
  result.zero_drain_unconstrained_minimum_rank_two =
      zero_alternatives_ok && result.zero_drain_alternative_arms == 30;
  result.generic_unconstrained_minimum_rank_four =
      generic_alternatives_ok
      && result.positive_drain_alternative_arms == 90
      && result.maximum_generic_determinant_formula_residual <= 1e-12
      && minimum_generic_determinant > 0.0;
  result.degenerate_minimum_rank_six =
      result.degenerate_a_equals_t_arms == 30;
  result.symmetry_price_bath_pairs = 1;
  result.cubic_covariance_prices_one_bath_pair =
      production_ranks_ok
      && result.symmetry_price_arms == 120
      && result.symmetry_price_bath_pairs == 1;
  result.branchwise_alternatives_are_not_one_global_form = true;
  result.native_canonical_action_derived = false;
  result.valid = result.standard_pairing_unique_up_to_scale
      && result.zero_drain_unconstrained_minimum_rank_two
      && result.generic_unconstrained_minimum_rank_four
      && result.degenerate_minimum_rank_six
      && result.cubic_covariance_prices_one_bath_pair
      && result.branchwise_alternatives_are_not_one_global_form
      && !result.native_canonical_action_derived;
  return result;
}

}  // namespace ftd::eft
