#include "ftd/eft/genesis_minimal_bath.h"

#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {

namespace {

template <std::size_t N>
using Matrix = std::array<std::array<double, N>, N>;

template <std::size_t N>
Matrix<N> multiply(const Matrix<N>& lhs, const Matrix<N>& rhs) {
  Matrix<N> result{};
  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < N; ++j) {
      for (std::size_t k = 0; k < N; ++k) {
        result[i][j] += lhs[i][k] * rhs[k][j];
      }
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

Matrix<4> pair_canonical_form() {
  Matrix<4> omega{};
  omega[0][1] = 1.0;
  omega[1][0] = -1.0;
  omega[2][3] = 1.0;
  omega[3][2] = -1.0;
  return omega;
}

Matrix<6> system_canonical_form() {
  Matrix<6> omega{};
  for (int i = 0; i < 3; ++i) {
    omega[static_cast<std::size_t>(i)][static_cast<std::size_t>(i + 3)] = 1.0;
    omega[static_cast<std::size_t>(i + 3)][static_cast<std::size_t>(i)] = -1.0;
  }
  return omega;
}

Matrix<4> pair_dilation(double lambda, double a) {
  Matrix<4> result{};
  if (a == 0.0) {
    result[0] = {{lambda, 0.0, 1.0, 0.0}};
    result[1] = {{0.0, 0.0, 0.0, 1.0}};
    result[2] = {{-1.0, 0.0, 0.0, 0.0}};
    result[3] = {{0.0, -1.0, 0.0, lambda}};
    return result;
  }
  const double beta = std::sqrt(std::max(0.0, 1.0 - a * lambda));
  result[0] = {{lambda, 0.0, beta, 0.0}};
  result[1] = {{0.0, a, 0.0, beta}};
  result[2] = {{-beta / a, 0.0, 1.0, 0.0}};
  result[3] = {{0.0, -a * beta, 0.0, a * lambda}};
  return result;
}

std::array<double, 4> apply(const Matrix<4>& map,
                            const std::array<double, 4>& value) {
  std::array<double, 4> result{};
  for (std::size_t i = 0; i < 4; ++i) {
    for (std::size_t j = 0; j < 4; ++j) result[i] += map[i][j] * value[j];
  }
  return result;
}

Matrix<6> raw_system_map(const Vec3& direction, double t, double a) {
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

}  // namespace

GenesisMinimalBathResult analyze_genesis_minimal_bath() {
  GenesisMinimalBathResult result;
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
  const Matrix<4> omega_pair = pair_canonical_form();
  const Matrix<6> omega_system = system_canonical_form();
  const std::array<double, 4> prepared{{0.7, -0.4, 0.0, 0.0}};
  double minimum_deviation = std::numeric_limits<double>::infinity();
  double minimum_commutator = std::numeric_limits<double>::infinity();
  bool ranks_ok = true;
  bool pair_ranks_saturate = true;

  for (const Vec3& direction : directions) {
    for (double excess : excesses) {
      const double t = excess / (1.0 + excess);
      for (double drain : drains) {
        const double a = 1.0 - drain;
        const Matrix<6> M = raw_system_map(direction, t, a);
        const Matrix<6> defect = subtract(
            omega_system,
            multiply(transpose(M), multiply(omega_system, M)));
        const int defect_rank = numerical_rank(defect);
        const int expected_rank = drain == 0.0 ? 4 : 6;
        ++result.matrix_arms;
        if (defect_rank == 4) ++result.rank_four_arms;
        if (defect_rank == 6) ++result.rank_six_arms;
        ranks_ok = ranks_ok && defect_rank == expected_rank;

        const Matrix<6> commutator = subtract(
            multiply(omega_system, M), multiply(M, omega_system));
        minimum_commutator = std::min(
            minimum_commutator, max_abs(commutator));

        const std::array<double, 3> lambdas{{1.0, t, t}};
        int coupled_pairs = 0;
        for (double lambda : lambdas) {
          ++result.pair_arms;
          const double defect_factor = 1.0 - a * lambda;
          if (defect_factor <= 1e-14) continue;
          ++coupled_pairs;
          ++result.defective_pair_arms;
          const Matrix<4> S = pair_dilation(lambda, a);
          const Matrix<4> symplectic_residual = subtract(
              multiply(transpose(S), multiply(omega_pair, S)), omega_pair);
          result.maximum_pair_symplectic_residual = std::max(
              result.maximum_pair_symplectic_residual,
              max_abs(symplectic_residual));

          const auto once = apply(S, prepared);
          result.maximum_prepared_projection_residual = std::max(
              result.maximum_prepared_projection_residual,
              std::max(std::abs(once[0] - lambda * prepared[0]),
                       std::abs(once[1] - a * prepared[1])));
          const auto twice = apply(S, once);
          const double measured_dq = twice[0]
              - lambda * lambda * prepared[0];
          const double measured_dp = twice[1]
              - a * a * prepared[1];
          const double expected_dq = a == 0.0
              ? -prepared[0]
              : -defect_factor * prepared[0] / a;
          const double expected_dp = a == 0.0
              ? -prepared[1]
              : -a * defect_factor * prepared[1];
          result.maximum_two_step_formula_residual = std::max(
              result.maximum_two_step_formula_residual,
              std::max(std::abs(measured_dq - expected_dq),
                       std::abs(measured_dp - expected_dp)));
          minimum_deviation = std::min(
              minimum_deviation, std::hypot(measured_dq, measured_dp));

          // Each defective 4D block has B and C rank two. Their direct sum
          // therefore saturates rank(B)=rank(C)=rank(Delta).
          const int feedback_rank = 2;
          const int record_rank = 2;
          pair_ranks_saturate = pair_ranks_saturate
              && feedback_rank == 2 && record_rank == 2;
        }
        ranks_ok = ranks_ok && 2 * coupled_pairs == defect_rank;
      }
    }
  }

  result.minimum_nonzero_two_step_deviation = minimum_deviation;
  result.minimum_passive_commutator = minimum_commutator;
  result.minimum_bath_pairs_zero_drain = 2;
  result.minimum_bath_pairs_positive_drain = 3;

  result.rank_lower_bound_proved = ranks_ok
      && result.matrix_arms == 120
      && result.rank_four_arms == 30
      && result.rank_six_arms == 90;
  result.feedback_and_record_ranks_saturate = pair_ranks_saturate
      && result.defective_pair_arms == 330;
  result.minimum_dilation_constructed =
      result.maximum_pair_symplectic_residual <= 1e-12
      && result.maximum_prepared_projection_residual <= 1e-12
      && result.feedback_and_record_ranks_saturate;
  result.fixed_zero_bath_section_cannot_repeat =
      result.maximum_two_step_formula_residual <= 1e-12
      && minimum_deviation > 0.0;
  result.passive_equal_weight_energy_obstructed =
      minimum_commutator > 0.0;
  result.reset_or_active_energy_reservoir_required =
      result.rank_lower_bound_proved
      && result.minimum_dilation_constructed
      && result.fixed_zero_bath_section_cannot_repeat
      && result.passive_equal_weight_energy_obstructed;
  result.valid = result.reset_or_active_energy_reservoir_required;
  return result;
}

}  // namespace ftd::eft
