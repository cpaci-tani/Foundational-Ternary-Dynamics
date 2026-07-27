#pragma once
/**
 * @file fixed_j_recoil_capacity.h
 * @brief Global minimum tick-energy cost of a fixed-J wave-velocity recoil.
 */

#include "ftd/eft/native_energy_contract.h"

#include <array>
#include <cmath>
#include <vector>

namespace ftd::eft {

struct FixedJRecoilCapacity {
  Vec3 requested_recoil{};
  Vec3 realized_recoil{};
  std::array<std::array<long double, 3>, 3> gram{};
  long double determinant = 0.0L;
  long double minimum_energy_change = 0.0L;
  long double direct_energy_change = 0.0L;
  double momentum_residual = 0.0;
  std::vector<Vec3> wave_vel_update;
  bool valid = false;
};

inline std::array<Vec3, 3> central_flux_derivatives(
    const RenderBridge& bridge, int index) {
  const auto coordinate = bridge.lattice().coord(index);
  const auto& voxels = bridge.voxels();
  const auto derivative = [&](int dx, int dy, int dz) {
    return (voxels[static_cast<std::size_t>(bridge.lattice().index(
                coordinate.x + dx, coordinate.y + dy, coordinate.z + dz))].flux
            - voxels[static_cast<std::size_t>(bridge.lattice().index(
                coordinate.x - dx, coordinate.y - dy, coordinate.z - dz))].flux)
        * 0.5;
  };
  return {{derivative(1, 0, 0), derivative(0, 1, 0),
           derivative(0, 0, 1)}};
}

inline Vec3 central_field_momentum(const RenderBridge& bridge) {
  Vec3 result{};
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto derivatives = central_flux_derivatives(bridge, index);
    const auto& wave_vel = bridge.voxels()[static_cast<std::size_t>(index)]
        .wave_vel;
    result.x -= wave_vel.dot(derivatives[0]);
    result.y -= wave_vel.dot(derivatives[1]);
    result.z -= wave_vel.dot(derivatives[2]);
  }
  return result;
}

inline bool invert_symmetric_3x3(
    const std::array<std::array<long double, 3>, 3>& matrix,
    std::array<std::array<long double, 3>, 3>& inverse,
    long double& determinant) {
  const long double a = matrix[0][0];
  const long double b = matrix[0][1];
  const long double c = matrix[0][2];
  const long double d = matrix[1][1];
  const long double e = matrix[1][2];
  const long double f = matrix[2][2];
  determinant = a * (d * f - e * e) - b * (b * f - c * e)
      + c * (b * e - c * d);
  if (!std::isfinite(determinant) || std::abs(determinant) <= 1e-30L)
    return false;
  inverse[0][0] = (d * f - e * e) / determinant;
  inverse[0][1] = (c * e - b * f) / determinant;
  inverse[0][2] = (b * e - c * d) / determinant;
  inverse[1][0] = inverse[0][1];
  inverse[1][1] = (a * f - c * c) / determinant;
  inverse[1][2] = (b * c - a * e) / determinant;
  inverse[2][0] = inverse[0][2];
  inverse[2][1] = inverse[1][2];
  inverse[2][2] = (a * d - b * b) / determinant;
  return true;
}

inline FixedJRecoilCapacity minimize_fixed_j_recoil_energy(
    const RenderBridge& bridge, const Vec3& requested_recoil) {
  FixedJRecoilCapacity result;
  result.requested_recoil = requested_recoil;
  const int count = static_cast<int>(bridge.voxels().size());
  result.wave_vel_update.assign(static_cast<std::size_t>(count), {});
  std::vector<std::array<Vec3, 3>> rows(static_cast<std::size_t>(count));
  std::vector<Vec3> b_vectors(static_cast<std::size_t>(count));
  std::array<long double, 3> a_dot_b{};
  long double b_norm2 = 0.0L;
  const double c2 = C_WAVE * C_WAVE;

  for (int index = 0; index < count; ++index) {
    const auto derivatives = central_flux_derivatives(bridge, index);
    for (int axis = 0; axis < 3; ++axis)
      rows[static_cast<std::size_t>(index)][static_cast<std::size_t>(axis)] =
          derivatives[static_cast<std::size_t>(axis)] * -1.0;
    const auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    const Vec3 b = voxel.wave_vel + bridge.laplacian_flux(index) * (0.5 * c2);
    b_vectors[static_cast<std::size_t>(index)] = b;
    b_norm2 += dot_long_double(b, b);
    for (int i = 0; i < 3; ++i) {
      a_dot_b[static_cast<std::size_t>(i)] += dot_long_double(
          rows[static_cast<std::size_t>(index)][static_cast<std::size_t>(i)], b);
      for (int j = 0; j < 3; ++j)
        result.gram[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
            += dot_long_double(
                rows[static_cast<std::size_t>(index)][static_cast<std::size_t>(i)],
                rows[static_cast<std::size_t>(index)][static_cast<std::size_t>(j)]);
    }
  }

  std::array<std::array<long double, 3>, 3> inverse{};
  if (!invert_symmetric_3x3(result.gram, inverse, result.determinant))
    return result;
  const std::array<long double, 3> recoil{{requested_recoil.x,
                                           requested_recoil.y,
                                           requested_recoil.z}};
  std::array<long double, 3> shifted{};
  std::array<long double, 3> lambda{};
  for (int i = 0; i < 3; ++i)
    shifted[static_cast<std::size_t>(i)] =
        recoil[static_cast<std::size_t>(i)]
        + a_dot_b[static_cast<std::size_t>(i)];
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      lambda[static_cast<std::size_t>(i)] +=
          inverse[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          * shifted[static_cast<std::size_t>(j)];

  long double minimum_shifted_norm2 = 0.0L;
  for (int i = 0; i < 3; ++i)
    minimum_shifted_norm2 += shifted[static_cast<std::size_t>(i)]
        * lambda[static_cast<std::size_t>(i)];
  result.minimum_energy_change =
      0.5L * minimum_shifted_norm2 - 0.5L * b_norm2;

  long double direct = 0.0L;
  for (int index = 0; index < count; ++index) {
    Vec3 shifted_minimum{};
    for (int axis = 0; axis < 3; ++axis)
      shifted_minimum += rows[static_cast<std::size_t>(index)]
          [static_cast<std::size_t>(axis)]
          * static_cast<double>(lambda[static_cast<std::size_t>(axis)]);
    const Vec3 update = shifted_minimum
        - b_vectors[static_cast<std::size_t>(index)];
    result.wave_vel_update[static_cast<std::size_t>(index)] = update;
    direct += 0.5L * dot_long_double(update, update)
        + dot_long_double(b_vectors[static_cast<std::size_t>(index)], update);
    result.realized_recoil.x += update.dot(
        rows[static_cast<std::size_t>(index)][0]);
    result.realized_recoil.y += update.dot(
        rows[static_cast<std::size_t>(index)][1]);
    result.realized_recoil.z += update.dot(
        rows[static_cast<std::size_t>(index)][2]);
  }
  result.direct_energy_change = direct;
  result.momentum_residual =
      (result.realized_recoil - requested_recoil).mag();
  result.valid = std::isfinite(result.minimum_energy_change)
      && std::isfinite(result.direct_energy_change)
      && std::isfinite(result.momentum_residual);
  return result;
}

}  // namespace ftd::eft

