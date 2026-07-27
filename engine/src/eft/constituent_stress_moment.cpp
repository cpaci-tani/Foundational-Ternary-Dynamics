#include "ftd/eft/constituent_stress_moment.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double tensor_difference(const SymmetricTensor3& lhs,
                         const SymmetricTensor3& rhs) {
  return std::max({std::abs(lhs.xx - rhs.xx),
                   std::abs(lhs.yy - rhs.yy),
                   std::abs(lhs.zz - rhs.zz),
                   std::abs(lhs.xy - rhs.xy),
                   std::abs(lhs.xz - rhs.xz),
                   std::abs(lhs.yz - rhs.yz)});
}

SymmetricTensor3 outer(const Vec3& value, double scale = 1.0) {
  return {scale * value.x * value.x,
          scale * value.y * value.y,
          scale * value.z * value.z,
          scale * value.x * value.y,
          scale * value.x * value.z,
          scale * value.y * value.z};
}

double energy(const Vec3& momentum,
              double rest_energy,
              double c_speed) {
  return std::sqrt(rest_energy * rest_energy
      + c_speed * c_speed * momentum.mag2());
}

double determinant(const SymmetricTensor3& tensor) {
  return tensor.xx * tensor.yy * tensor.zz
      + 2.0 * tensor.xy * tensor.xz * tensor.yz
      - tensor.xx * tensor.yz * tensor.yz
      - tensor.yy * tensor.xz * tensor.xz
      - tensor.zz * tensor.xy * tensor.xy;
}

double frobenius_squared(const SymmetricTensor3& tensor) {
  return tensor.xx * tensor.xx + tensor.yy * tensor.yy
      + tensor.zz * tensor.zz
      + 2.0 * (tensor.xy * tensor.xy
               + tensor.xz * tensor.xz
               + tensor.yz * tensor.yz);
}

double momentum_set_separation(std::vector<Vec3> lhs,
                               const std::vector<Vec3>& rhs) {
  double best_global = INFINITY;
  std::sort(lhs.begin(), lhs.end(), [](const Vec3& a, const Vec3& b) {
    return std::array<double, 3>{a.x, a.y, a.z}
        < std::array<double, 3>{b.x, b.y, b.z};
  });
  std::array<int, 4> permutation{{0, 1, 2, 3}};
  do {
    double residual = 0.0;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
      residual = std::max(residual,
          (lhs[i] - rhs[static_cast<std::size_t>(permutation[i])]).mag());
    }
    best_global = std::min(best_global, residual);
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return best_global;
}

struct FourthMoment {
  double xxxx = 0.0;
  double yyyy = 0.0;
  double xxyy = 0.0;
};

FourthMoment fourth_moment(const std::vector<Vec3>& momenta) {
  FourthMoment result;
  for (const auto& p : momenta) {
    result.xxxx += p.x * p.x * p.x * p.x;
    result.yyyy += p.y * p.y * p.y * p.y;
    result.xxyy += p.x * p.x * p.y * p.y;
  }
  return result;
}

}  // namespace

double SymmetricTensor3::component(int row, int column) const {
  if (row < 0 || row > 2 || column < 0 || column > 2) return NAN;
  if (row == 0 && column == 0) return xx;
  if (row == 1 && column == 1) return yy;
  if (row == 2 && column == 2) return zz;
  if ((row == 0 && column == 1) || (row == 1 && column == 0))
    return xy;
  if ((row == 0 && column == 2) || (row == 2 && column == 0))
    return xz;
  return yz;
}

SymmetricTensor3 operator+(const SymmetricTensor3& lhs,
                           const SymmetricTensor3& rhs) {
  return {lhs.xx + rhs.xx, lhs.yy + rhs.yy, lhs.zz + rhs.zz,
          lhs.xy + rhs.xy, lhs.xz + rhs.xz, lhs.yz + rhs.yz};
}

SymmetricTensor3 operator-(const SymmetricTensor3& lhs,
                           const SymmetricTensor3& rhs) {
  return {lhs.xx - rhs.xx, lhs.yy - rhs.yy, lhs.zz - rhs.zz,
          lhs.xy - rhs.xy, lhs.xz - rhs.xz, lhs.yz - rhs.yz};
}

SymmetricTensor3 operator*(const SymmetricTensor3& tensor,
                           double scale) {
  return {tensor.xx * scale, tensor.yy * scale, tensor.zz * scale,
          tensor.xy * scale, tensor.xz * scale, tensor.yz * scale};
}

ConstituentStressMoment make_constituent_stress_moment(
    const std::vector<Vec3>& momenta,
    double rest_energy,
    double c_speed,
    double tolerance) {
  ConstituentStressMoment result;
  result.carrier_count = static_cast<int>(momenta.size());
  result.rest_energy = rest_energy;
  result.c_speed = c_speed;
  if (momenta.empty() || !std::isfinite(rest_energy)
      || rest_energy <= 0.0 || !std::isfinite(c_speed)
      || c_speed <= 0.0 || !std::isfinite(tolerance)
      || tolerance < 0.0) return result;
  for (const auto& momentum : momenta) {
    if (!finite(momentum)) return result;
    const double carrier_energy = energy(momentum, rest_energy, c_speed);
    result.total_momentum += momentum;
    result.total_energy += carrier_energy;
    result.stress = result.stress + outer(
        momentum, c_speed * c_speed / carrier_energy);
  }
  result.kinetic_energy = result.total_energy
      - result.carrier_count * rest_energy;
  result.stress_trace = result.stress.trace();

  const std::array<double, 7> principal_minors{{
      result.stress.xx,
      result.stress.yy,
      result.stress.zz,
      result.stress.xx * result.stress.yy
          - result.stress.xy * result.stress.xy,
      result.stress.xx * result.stress.zz
          - result.stress.xz * result.stress.xz,
      result.stress.yy * result.stress.zz
          - result.stress.yz * result.stress.yz,
      determinant(result.stress)}};
  result.minimum_principal_minor = *std::min_element(
      principal_minors.begin(), principal_minors.end());
  result.psd_residual = std::max(
      0.0, -result.minimum_principal_minor);
  result.valid = std::isfinite(result.total_energy)
      && std::isfinite(result.kinetic_energy)
      && std::isfinite(result.stress_trace)
      && result.psd_residual <= tolerance;
  return result;
}

SymmetricTensor3 transform_symmetric_tensor(
    const SymmetricTensor3& tensor,
    const int permutation[3],
    const int sign[3]) {
  SymmetricTensor3 result;
  double transformed[3][3]{};
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      transformed[i][j] = sign[i] * sign[j]
          * tensor.component(permutation[i], permutation[j]);
    }
  }
  result.xx = transformed[0][0];
  result.yy = transformed[1][1];
  result.zz = transformed[2][2];
  result.xy = 0.5 * (transformed[0][1] + transformed[1][0]);
  result.xz = 0.5 * (transformed[0][2] + transformed[2][0]);
  result.yz = 0.5 * (transformed[1][2] + transformed[2][1]);
  return result;
}

TwoStreamStressLiftResult analyze_two_stream_stress_lift(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double rest_energy,
    double c_speed,
    double tolerance) {
  TwoStreamStressLiftResult result;
  result.polarity = polarity;
  result.chart_direction = chart_direction;
  result.collision_position = collision_position;
  result.collision = analyze_constituent_relative_collision(
      L, collision_position, chart_direction, polarity, speed,
      rest_energy, c_speed, 0.25, tolerance);
  if (!result.collision.valid) return result;
  const std::vector<Vec3> momenta{{
      result.collision.momentum_first_before,
      result.collision.momentum_second_before}};
  result.moment = make_constituent_stress_moment(
      momenta, rest_energy, c_speed, tolerance);
  const Vec3 normal = result.collision.chart_normal;
  result.expected_axis_projector = outer(normal);
  if (result.moment.stress_trace > 0.0) {
    result.recovered_axis_projector = result.moment.stress
        * (1.0 / result.moment.stress_trace);
  }
  result.axis_projector_residual = tensor_difference(
      result.expected_axis_projector,
      result.recovered_axis_projector);
  result.rank_one_residual = std::max(
      std::abs(result.moment.stress_trace
                   * result.moment.stress_trace
               - frobenius_squared(result.moment.stress)),
      std::abs(determinant(result.moment.stress)));
  const double half_trace = 0.5 * result.moment.stress_trace;
  result.recovered_single_energy = 0.5 * (
      half_trace + std::sqrt(half_trace * half_trace
          + 4.0 * rest_energy * rest_energy));
  result.recovered_momentum_magnitude = std::sqrt(std::max(
      0.0, result.recovered_single_energy
          * result.recovered_single_energy
      - rest_energy * rest_energy)) / c_speed;
  result.recovered_pair_kinetic_energy = 2.0
      * (result.recovered_single_energy - rest_energy);
  const double actual_single_energy = energy(
      momenta[0], rest_energy, c_speed);
  result.energy_recovery_residual = std::abs(
      result.recovered_single_energy - actual_single_energy);
  result.momentum_recovery_residual = std::abs(
      result.recovered_momentum_magnitude
      - result.collision.momentum_magnitude);
  result.kinetic_recovery_residual = std::abs(
      result.recovered_pair_kinetic_energy
      - result.moment.kinetic_energy);
  result.vector_current_cancelled = result.moment.total_momentum.mag()
      <= tolerance;
  result.stress_retains_relative_mode = result.moment.stress_trace
      > tolerance && result.axis_projector_residual <= tolerance;
  result.valid = result.collision.valid && result.moment.valid
      && result.vector_current_cancelled
      && result.stress_retains_relative_mode
      && result.rank_one_residual <= tolerance
      && result.energy_recovery_residual <= tolerance
      && result.momentum_recovery_residual <= tolerance
      && result.kinetic_recovery_residual <= tolerance;
  return result;
}

MultistreamStressCounterexample analyze_multistream_stress_counterexample(
    double momentum_magnitude,
    double rest_energy,
    double c_speed,
    double tolerance) {
  MultistreamStressCounterexample result;
  result.carrier_count = 4;
  if (!std::isfinite(momentum_magnitude) || momentum_magnitude <= 0.0
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  const Vec3 ex{1.0, 0.0, 0.0};
  const Vec3 ey{0.0, 1.0, 0.0};
  const double inverse_sqrt_two = 1.0 / std::sqrt(2.0);
  const Vec3 d1{inverse_sqrt_two, inverse_sqrt_two, 0.0};
  const Vec3 d2{inverse_sqrt_two, -inverse_sqrt_two, 0.0};
  const std::vector<Vec3> axial{{
      ex * momentum_magnitude, ex * -momentum_magnitude,
      ey * momentum_magnitude, ey * -momentum_magnitude}};
  const std::vector<Vec3> diagonal{{
      d1 * momentum_magnitude, d1 * -momentum_magnitude,
      d2 * momentum_magnitude, d2 * -momentum_magnitude}};
  result.axial = make_constituent_stress_moment(
      axial, rest_energy, c_speed, tolerance);
  result.diagonal = make_constituent_stress_moment(
      diagonal, rest_energy, c_speed, tolerance);
  result.momentum_multiset_separation = momentum_set_separation(
      axial, diagonal);
  result.total_momentum_residual = (
      result.axial.total_momentum
      - result.diagonal.total_momentum).mag();
  result.total_energy_residual = std::abs(
      result.axial.total_energy - result.diagonal.total_energy);
  result.stress_residual = tensor_difference(
      result.axial.stress, result.diagonal.stress);
  const FourthMoment axial_fourth = fourth_moment(axial);
  const FourthMoment diagonal_fourth = fourth_moment(diagonal);
  result.fourth_xxxx_difference = std::abs(
      axial_fourth.xxxx - diagonal_fourth.xxxx);
  result.fourth_yyyy_difference = std::abs(
      axial_fourth.yyyy - diagonal_fourth.yyyy);
  result.fourth_xxyy_difference = std::abs(
      axial_fourth.xxyy - diagonal_fourth.xxyy);
  result.fourth_moment_difference = std::max({
      result.fourth_xxxx_difference,
      result.fourth_yyyy_difference,
      result.fourth_xxyy_difference});
  result.valid = result.axial.valid && result.diagonal.valid
      && result.momentum_multiset_separation > tolerance
      && result.total_momentum_residual <= tolerance
      && result.total_energy_residual <= tolerance
      && result.stress_residual <= tolerance
      && result.fourth_moment_difference > tolerance;
  return result;
}

}  // namespace ftd::eft
