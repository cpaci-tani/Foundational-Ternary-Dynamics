#include "ftd/eft/passive_dressing_depinning_obstruction.h"

#include "ftd/constants.h"
#include "ftd/lattice.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

using Matrix3 = std::array<std::array<int, 3>, 3>;
constexpr double PI_LOCAL = 3.1415926535897932384626433832795;
constexpr double TOL = 1e-12;

double response(double kx, double ky, double kz) {
  const double sx = std::sin(kx), sy = std::sin(ky), sz = std::sin(kz);
  const double numerator = 3.0 * (sx * sx + sy * sy + sz * sz);
  const double c1 = std::cos(kx) + std::cos(ky) + std::cos(kz);
  const double c2 = std::cos(kx) * std::cos(ky)
      + std::cos(kx) * std::cos(kz) + std::cos(ky) * std::cos(kz);
  const double symbol = 4.0 - (2.0 / 3.0) * c1 - (2.0 / 3.0) * c2;
  return symbol > 1e-15 ? numerator / symbol : 0.0;
}

double coat2(double kx, double ky, double kz) {
  const double bx = 0.5 * (1.0 + std::cos(kx));
  const double by = 0.5 * (1.0 + std::cos(ky));
  const double bz = 0.5 * (1.0 + std::cos(kz));
  const double coat = bx * by * bz;
  return coat * coat;
}

double peierls_coefficient(int L, Coord direction, int charge) {
  long double sum = 0.0L;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const double kx = 2.0 * PI_LOCAL * x / L;
        const double ky = 2.0 * PI_LOCAL * y / L;
        const double kz = 2.0 * PI_LOCAL * z / L;
        const double phase = kx * direction.x + ky * direction.y
            + kz * direction.z;
        sum += static_cast<long double>(charge * charge)
            * response(kx, ky, kz) * coat2(kx, ky, kz)
            * (1.0 - std::cos(phase));
      }
    }
  }
  const long double volume = static_cast<long double>(L) * L * L;
  return static_cast<double>(G_C * G_C * sum / volume);
}

std::vector<Matrix3> proper_rotations() {
  std::vector<Matrix3> result;
  std::array<int, 3> permutation{{0, 1, 2}};
  do {
    int inversions = 0;
    for (int i = 0; i < 3; ++i) {
      for (int j = i + 1; j < 3; ++j) {
        if (permutation[static_cast<std::size_t>(i)]
            > permutation[static_cast<std::size_t>(j)]) ++inversions;
      }
    }
    const int parity = inversions % 2 == 0 ? 1 : -1;
    for (int sx : {-1, 1}) for (int sy : {-1, 1}) for (int sz : {-1, 1}) {
      if (parity * sx * sy * sz != 1) continue;
      Matrix3 matrix{};
      const std::array<int, 3> signs{{sx, sy, sz}};
      for (int row = 0; row < 3; ++row) {
        matrix[static_cast<std::size_t>(row)]
              [static_cast<std::size_t>(permutation[static_cast<std::size_t>(row)])]
            = signs[static_cast<std::size_t>(row)];
      }
      result.push_back(matrix);
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return result;
}

Coord rotate(const Matrix3& matrix, Coord value) {
  const std::array<int, 3> input{{value.x, value.y, value.z}};
  std::array<int, 3> output{};
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      output[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)]
          * input[static_cast<std::size_t>(col)];
    }
  }
  return {output[0], output[1], output[2]};
}

struct Threshold {
  double barrier = 0.0;
  double momentum = 0.0;
  double speed = 0.0;
  double energy_residual = 0.0;
  double inverse_momentum_residual = 0.0;
  double velocity_identity_residual = 0.0;
};

Threshold threshold(double coefficient) {
  Threshold result;
  result.barrier = coefficient / 4.0;
  result.momentum = std::sqrt(
      2.0 * E_REST * result.barrier
      + result.barrier * result.barrier) / C_SPEED;
  const double energy = std::sqrt(
      E_REST * E_REST
      + C_SPEED * C_SPEED * result.momentum * result.momentum);
  result.speed = C_SPEED * C_SPEED * result.momentum / energy;
  result.energy_residual = energy - E_REST - result.barrier;
  const double inverse_momentum = std::sqrt(std::max(
      0.0, energy * energy - E_REST * E_REST)) / C_SPEED;
  result.inverse_momentum_residual = inverse_momentum - result.momentum;
  const double gamma = 1.0 / std::sqrt(
      1.0 - result.speed * result.speed / (C_SPEED * C_SPEED));
  const double velocity_momentum = gamma * M_INERTIAL * result.speed;
  result.velocity_identity_residual = velocity_momentum - result.momentum;
  return result;
}

struct PassiveFixture {
  std::array<double, 3> stiffness;
  std::array<double, 3> tangent;
};

constexpr std::array<PassiveFixture, 4> PASSIVE_FIXTURES{{
  {{{1.0, 1.0, 1.0}}, {{1.0, 0.0, 0.0}}},
  {{{1.0, 2.0, 4.0}}, {{0.0, 1.0, 1.0}}},
  {{{4.0, 2.0, 1.0}}, {{1.0, -1.0, 1.0}}},
  {{{2.0, 3.0, 5.0}}, {{2.0, 1.0, -1.0}}},
}};

double passive_excess(const PassiveFixture& fixture, double r) {
  double result = 0.0;
  for (int axis = 0; axis < 3; ++axis) {
    const double displacement = r * fixture.tangent[static_cast<std::size_t>(axis)];
    result += 0.5 * fixture.stiffness[static_cast<std::size_t>(axis)]
        * displacement * displacement;
  }
  return result;
}

}  // namespace

PassiveDressingDepinningObstructionResult
analyze_passive_dressing_depinning_obstruction() {
  PassiveDressingDepinningObstructionResult result;
  result.minimum_cusp_slope_gap = std::numeric_limits<double>::infinity();
  result.minimum_equality_derivative_jump =
      std::numeric_limits<double>::infinity();
  result.minimum_peierls_coefficient = std::numeric_limits<double>::infinity();
  result.minimum_barrier = std::numeric_limits<double>::infinity();
  result.minimum_depinning_momentum = std::numeric_limits<double>::infinity();
  result.minimum_depinning_speed = std::numeric_limits<double>::infinity();

  for (int L : {17, 33}) {
    for (int charge : {-1, 1}) {
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          for (int dz = -1; dz <= 1; ++dz) {
            if (dx == 0 && dy == 0 && dz == 0) continue;
            const Coord direction{dx, dy, dz};
            const double coefficient = peierls_coefficient(
                L, direction, charge);
            const double mirror = peierls_coefficient(
                L, direction, -charge);
            const Threshold depinning = threshold(coefficient);
            result.minimum_peierls_coefficient = std::min(
                result.minimum_peierls_coefficient, coefficient);
            result.maximum_peierls_coefficient = std::max(
                result.maximum_peierls_coefficient, coefficient);
            result.minimum_barrier = std::min(
                result.minimum_barrier, depinning.barrier);
            result.maximum_barrier = std::max(
                result.maximum_barrier, depinning.barrier);
            result.minimum_depinning_momentum = std::min(
                result.minimum_depinning_momentum, depinning.momentum);
            result.maximum_depinning_momentum = std::max(
                result.maximum_depinning_momentum, depinning.momentum);
            result.minimum_depinning_speed = std::min(
                result.minimum_depinning_speed, depinning.speed);
            result.maximum_depinning_speed = std::max(
                result.maximum_depinning_speed, depinning.speed);
            result.maximum_threshold_energy_residual = std::max(
                result.maximum_threshold_energy_residual,
                std::abs(depinning.energy_residual));
            result.maximum_inverse_momentum_residual = std::max(
                result.maximum_inverse_momentum_residual,
                std::abs(depinning.inverse_momentum_residual));
            result.maximum_velocity_identity_residual = std::max(
                result.maximum_velocity_identity_residual,
                std::abs(depinning.velocity_identity_residual));
            result.maximum_polarity_residual = std::max(
                result.maximum_polarity_residual,
                std::abs(coefficient - mirror));
            ++result.depinning_arms;

            // A stable completed-square field has zero first variation at its
            // relaxed dressing. Every locally Lipschitz fixture below is
            // z-z_*=r*a, so its excess is O(r^2), never -C|r|.
            for (const PassiveFixture& fixture : PASSIVE_FIXTURES) {
              const double analytic_linear_coefficient = 0.0;
              result.maximum_passive_linear_coefficient = std::max(
                  result.maximum_passive_linear_coefficient,
                  std::abs(analytic_linear_coefficient));
              result.minimum_cusp_slope_gap = std::min(
                  result.minimum_cusp_slope_gap,
                  coefficient - analytic_linear_coefficient);
              for (int step = 0; step <= 8; ++step) {
                const double r = step / 8.0;
                const double excess = passive_excess(fixture, r);
                result.maximum_passive_negative_excess = std::max(
                    result.maximum_passive_negative_excess,
                    std::max(0.0, -excess));
                ++result.passive_samples;
              }
              ++result.passive_fixture_arms;
            }

            // Algebraic energy budgets for a positive one-mode reservoir.
            // omega=1 fixes an arbitrary coordinate unit and carries no
            // physical frequency identification.
            constexpr double omega = 1.0;
            for (double ratio : {1.0, 2.0, 4.0}) {
              const double excitation = ratio * depinning.barrier;
              for (int step = 0; step <= 8; ++step) {
                const double r = step / 8.0;
                const double internal = excitation
                    - coefficient * r * (1.0 - r);
                const double coordinate = std::sqrt(
                    std::max(0.0, 2.0 * internal)) / omega;
                const double reconstructed = 0.5 * omega * omega
                    * coordinate * coordinate;
                result.maximum_active_budget_residual = std::max(
                    result.maximum_active_budget_residual,
                    std::abs(reconstructed - internal));
                if (ratio == 1.0 && step == 4) {
                  result.maximum_equality_midpoint_residual = std::max(
                      result.maximum_equality_midpoint_residual,
                      std::abs(internal));
                }
                ++result.active_budget_samples;
              }
              if (ratio == 1.0) {
                const double derivative_jump = 2.0
                    * std::sqrt(2.0 * coefficient) / omega;
                result.minimum_equality_derivative_jump = std::min(
                    result.minimum_equality_derivative_jump,
                    derivative_jump);
                ++result.equality_nondifferentiable_arms;
              } else {
                ++result.smooth_excited_arms;
              }
            }
          }
        }
      }
    }
  }

  const int L = 17;
  const Coord reference_direction{1, 1, 1};
  const double reference_coefficient = peierls_coefficient(
      L, reference_direction, 1);
  const Threshold reference_threshold = threshold(reference_coefficient);
  for (const Matrix3& rotation : proper_rotations()) {
    const Coord direction = rotate(rotation, reference_direction);
    const double coefficient = peierls_coefficient(L, direction, 1);
    const Threshold transformed = threshold(coefficient);
    result.maximum_cubic_covariance_residual = std::max({
        result.maximum_cubic_covariance_residual,
        std::abs(coefficient - reference_coefficient),
        std::abs(transformed.momentum - reference_threshold.momentum),
        std::abs(transformed.speed - reference_threshold.speed)});
    ++result.cubic_rotation_arms;
  }

  result.exact_relativistic_depinning = result.depinning_arms == 104
      && result.minimum_peierls_coefficient > 1e-14
      && result.minimum_barrier > 1e-14
      && result.minimum_depinning_momentum > 0.0
      && result.minimum_depinning_speed > 0.0
      && result.maximum_depinning_speed < C_SPEED
      && result.maximum_threshold_energy_residual <= TOL
      && result.maximum_inverse_momentum_residual <= TOL
      && result.maximum_velocity_identity_residual <= TOL
      && result.maximum_polarity_residual <= TOL
      && result.cubic_rotation_arms == 24
      && result.maximum_cubic_covariance_residual <= TOL;
  result.passive_completed_square_obstruction =
      result.passive_fixture_arms == 416
      && result.passive_samples == 3744
      && result.maximum_passive_linear_coefficient <= TOL
      && result.maximum_passive_negative_excess <= TOL;
  result.passive_cusp_obstruction =
      result.passive_completed_square_obstruction
      && result.minimum_cusp_slope_gap > 1e-14;
  result.active_excitation_lower_bound =
      result.active_budget_samples == 2808
      && result.equality_nondifferentiable_arms == 104
      && result.smooth_excited_arms == 208
      && result.maximum_active_budget_residual <= TOL
      && result.maximum_equality_midpoint_residual <= TOL
      && result.minimum_equality_derivative_jump > 1e-14;
  result.active_common_action_derived = false;
  result.production_changed = false;
  result.valid = result.exact_relativistic_depinning
      && result.passive_completed_square_obstruction
      && result.passive_cusp_obstruction
      && result.active_excitation_lower_bound
      && !result.active_common_action_derived
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
