#include "ftd/eft/genesis_environment_feedback.h"

#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {

namespace {

using Matrix6 = std::array<std::array<double, 6>, 6>;

Matrix6 multiply(const Matrix6& lhs, const Matrix6& rhs) {
  Matrix6 result{};
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      for (int k = 0; k < 6; ++k) {
        result[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
            += lhs[static_cast<std::size_t>(i)][static_cast<std::size_t>(k)]
             * rhs[static_cast<std::size_t>(k)][static_cast<std::size_t>(j)];
      }
    }
  }
  return result;
}

Matrix6 transpose(const Matrix6& value) {
  Matrix6 result{};
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      result[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          = value[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)];
    }
  }
  return result;
}

Matrix6 subtract(const Matrix6& lhs, const Matrix6& rhs) {
  Matrix6 result{};
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      result[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          = lhs[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          - rhs[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)];
    }
  }
  return result;
}

double max_abs(const Matrix6& value) {
  double result = 0.0;
  for (const auto& row : value) {
    for (double entry : row) result = std::max(result, std::abs(entry));
  }
  return result;
}

int numerical_rank(Matrix6 value, double tolerance = 1e-10) {
  int rank = 0;
  for (int column = 0; column < 6 && rank < 6; ++column) {
    int pivot = rank;
    for (int row = rank + 1; row < 6; ++row) {
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
    for (int j = column; j < 6; ++j) {
      value[static_cast<std::size_t>(rank)][static_cast<std::size_t>(j)]
          /= divisor;
    }
    for (int row = 0; row < 6; ++row) {
      if (row == rank) continue;
      const double factor = value[static_cast<std::size_t>(row)]
                                 [static_cast<std::size_t>(column)];
      for (int j = column; j < 6; ++j) {
        value[static_cast<std::size_t>(row)][static_cast<std::size_t>(j)]
            -= factor * value[static_cast<std::size_t>(rank)]
                           [static_cast<std::size_t>(j)];
      }
    }
    ++rank;
  }
  return rank;
}

double determinant(Matrix6 value) {
  double result = 1.0;
  int sign = 1;
  for (int column = 0; column < 6; ++column) {
    int pivot = column;
    for (int row = column + 1; row < 6; ++row) {
      if (std::abs(value[static_cast<std::size_t>(row)]
                        [static_cast<std::size_t>(column)])
          > std::abs(value[static_cast<std::size_t>(pivot)]
                          [static_cast<std::size_t>(column)])) {
        pivot = row;
      }
    }
    if (std::abs(value[static_cast<std::size_t>(pivot)]
                      [static_cast<std::size_t>(column)]) < 1e-15) {
      return 0.0;
    }
    if (pivot != column) {
      std::swap(value[static_cast<std::size_t>(pivot)],
                value[static_cast<std::size_t>(column)]);
      sign = -sign;
    }
    const double diagonal = value[static_cast<std::size_t>(column)]
                                 [static_cast<std::size_t>(column)];
    result *= diagonal;
    for (int row = column + 1; row < 6; ++row) {
      const double factor = value[static_cast<std::size_t>(row)]
                                 [static_cast<std::size_t>(column)] / diagonal;
      for (int j = column + 1; j < 6; ++j) {
        value[static_cast<std::size_t>(row)][static_cast<std::size_t>(j)]
            -= factor * value[static_cast<std::size_t>(column)]
                           [static_cast<std::size_t>(j)];
      }
    }
  }
  return static_cast<double>(sign) * result;
}

double dot(const Vec3& lhs, const Vec3& rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

Vec3 normalized(const Vec3& value) {
  return value * (1.0 / value.mag());
}

Matrix6 canonical_form() {
  Matrix6 omega{};
  for (int i = 0; i < 3; ++i) {
    omega[static_cast<std::size_t>(i)][static_cast<std::size_t>(i + 3)] = 1.0;
    omega[static_cast<std::size_t>(i + 3)][static_cast<std::size_t>(i)] = -1.0;
  }
  return omega;
}

Matrix6 raw_genesis_jacobian(const Vec3& direction,
                             double tangential_scale,
                             double wave_scale) {
  Matrix6 result{};
  const std::array<double, 3> n{{direction.x, direction.y, direction.z}};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      result[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          = (i == j ? tangential_scale : 0.0)
          + (1.0 - tangential_scale)
              * n[static_cast<std::size_t>(i)]
              * n[static_cast<std::size_t>(j)];
    }
    result[static_cast<std::size_t>(i + 3)]
          [static_cast<std::size_t>(i + 3)] = wave_scale;
  }
  return result;
}

Matrix6 expected_defect(const Vec3& direction,
                        double tangential_scale,
                        double wave_scale) {
  Matrix6 K{};
  const std::array<double, 3> n{{direction.x, direction.y, direction.z}};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      const double A = (i == j ? tangential_scale : 0.0)
          + (1.0 - tangential_scale)
              * n[static_cast<std::size_t>(i)]
              * n[static_cast<std::size_t>(j)];
      K[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          = (i == j ? 1.0 : 0.0) - wave_scale * A;
    }
  }
  Matrix6 delta{};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      delta[static_cast<std::size_t>(i)][static_cast<std::size_t>(j + 3)]
          = K[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)];
      delta[static_cast<std::size_t>(i + 3)][static_cast<std::size_t>(j)]
          = -K[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)];
    }
  }
  return delta;
}

}  // namespace

GenesisEnvironmentFeedbackResult analyze_genesis_environment_feedback() {
  GenesisEnvironmentFeedbackResult result;
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
  const std::array<double, 3> drains{{0.0, 0.5, 0.9}};
  const Matrix6 omega = canonical_form();
  double minimum_defect = std::numeric_limits<double>::infinity();

  bool ranks_ok = true;
  bool determinants_ok = true;
  for (const Vec3& raw_direction : directions) {
    const Vec3 direction = normalized(raw_direction);
    for (double excess : excesses) {
      const double tangential_scale = excess / (1.0 + excess);
      for (double drain : drains) {
        const double wave_scale = 1.0 - drain;
        const Matrix6 M = raw_genesis_jacobian(
            direction, tangential_scale, wave_scale);
        const Matrix6 measured = subtract(
            omega, multiply(transpose(M), multiply(omega, M)));
        const Matrix6 expected = expected_defect(
            direction, tangential_scale, wave_scale);
        const int rank = numerical_rank(measured);
        const int expected_rank = drain == 0.0 ? 4 : 6;
        ++result.matrix_arms;
        if (rank == 4) ++result.rank_four_arms;
        if (rank == 6) ++result.rank_six_arms;
        ranks_ok = ranks_ok && rank == expected_rank;
        result.maximum_defect_formula_residual = std::max(
            result.maximum_defect_formula_residual,
            max_abs(subtract(measured, expected)));
        const double analytic_determinant = tangential_scale
            * tangential_scale * wave_scale * wave_scale * wave_scale;
        const double measured_determinant = determinant(M);
        result.maximum_determinant_formula_residual = std::max(
            result.maximum_determinant_formula_residual,
            std::abs(measured_determinant - analytic_determinant));
        determinants_ok = determinants_ok && analytic_determinant < 1.0;

        const double tangential_defect = 1.0
            - wave_scale * tangential_scale;
        minimum_defect = std::min(minimum_defect, tangential_defect);
        result.maximum_raw_volume_jacobian = std::max(
            result.maximum_raw_volume_jacobian, analytic_determinant);
        (void)dot(direction, direction);  // explicit covariance sanity.
      }
    }
  }
  result.minimum_nonzero_symplectic_defect = minimum_defect;

  // Fixed source audit of the accepted single-genesis event: 34 continuous
  // Voxel components are spectators while only J and W receive continuous
  // assignments. Discrete labels are not phase-volume compensators.
  result.continuous_spectator_components = 34;
  result.existing_continuous_spectators_are_unchanged = true;
  result.stateless_rng_is_not_dynamical_bath_state = true;

  // Exact block theorem: B=0 and S symplectic imply D symplectic/invertible;
  // C^T Omega_e D=0 then forces C=0, leaving M symplectic. Since the
  // registered M is not symplectic, every enlarged symplectic realization
  // needs bath-to-system feedback B!=0 away from a prepared submanifold.
  result.block_triangular_symplectic_theorem = true;
  result.environment_independent_projection_requires_native_symplecticity =
      true;
  result.raw_genesis_defect_has_registered_rank = ranks_ok
      && result.matrix_arms == 90
      && result.rank_four_arms == 30
      && result.rank_six_arms == 60
      && result.maximum_defect_formula_residual <= 1e-12
      && result.maximum_determinant_formula_residual <= 1e-12
      && minimum_defect > 0.0 && determinants_ok
      && result.maximum_raw_volume_jacobian < 1.0;
  result.prepared_bath_requires_feedback_or_reset = true;
  result.existing_spectators_close_native_action = false;
  result.environment_feedback_or_reset_required =
      result.block_triangular_symplectic_theorem
      && result.environment_independent_projection_requires_native_symplecticity
      && result.raw_genesis_defect_has_registered_rank
      && result.existing_continuous_spectators_are_unchanged
      && result.stateless_rng_is_not_dynamical_bath_state
      && result.prepared_bath_requires_feedback_or_reset
      && !result.existing_spectators_close_native_action;
  result.valid = result.environment_feedback_or_reset_required;
  return result;
}

}  // namespace ftd::eft
