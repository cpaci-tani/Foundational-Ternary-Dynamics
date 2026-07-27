#pragma once
/**
 * @file supported_paired_recoil_capacity.h
 * @brief Paired J/W recoil minimization inside a fixed site-support mask.
 */

#include "ftd/eft/paired_jw_recoil_capacity.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace ftd::eft {

struct SupportedPairedRecoilCapacity {
  long double determinant = 0.0L;
  long double minimum_total_energy_change = 0.0L;
  long double direct_minimum_energy_change = 0.0L;
  long double direct_zero_energy_change = 0.0L;
  long double covariant_null_norm = 0.0L;
  double minimum_momentum_residual = 0.0;
  double zero_momentum_residual = 0.0;
  std::vector<Vec3> minimum_impulse;
  std::vector<Vec3> zero_energy_impulse;
  bool zero_energy_solution = false;
  bool support_exact = false;
  bool valid = false;
};

inline SupportedPairedRecoilCapacity minimize_supported_paired_recoil_energy(
    const RenderBridge& old_state, const RenderBridge& control_state,
    int target_index, std::int8_t charge, const Vec3& requested_recoil,
    const std::vector<std::uint8_t>& support) {
  SupportedPairedRecoilCapacity result;
  const int count = static_cast<int>(old_state.voxels().size());
  if (control_state.voxels().size() != old_state.voxels().size()
      || support.size() != old_state.voxels().size()
      || target_index < 0 || target_index >= count || charge == 0)
    return result;

  result.minimum_impulse.assign(static_cast<std::size_t>(count), {});
  result.zero_energy_impulse.assign(static_cast<std::size_t>(count), {});
  std::vector<std::array<Vec3, 3>> rows(static_cast<std::size_t>(count));
  std::vector<Vec3> coefficient(static_cast<std::size_t>(count));
  std::vector<Vec3> control_j(static_cast<std::size_t>(count));
  std::vector<Vec3> control_w(static_cast<std::size_t>(count));
  for (int index = 0; index < count; ++index) {
    control_j[static_cast<std::size_t>(index)] =
        control_state.voxels()[static_cast<std::size_t>(index)].flux;
    control_w[static_cast<std::size_t>(index)] =
        control_state.voxels()[static_cast<std::size_t>(index)].wave_vel;
  }
  // Populate the complete control fields before applying the nonlocal stencil.
  // Computing K(J) or K(W) in the loop above would read not-yet-copied sites.
  for (int index = 0; index < count; ++index) {
    const auto derivatives = central_flux_derivatives(old_state, index);
    for (int axis = 0; axis < 3; ++axis)
      rows[static_cast<std::size_t>(index)][static_cast<std::size_t>(axis)] =
          derivatives[static_cast<std::size_t>(axis)] * -1.0;
    coefficient[static_cast<std::size_t>(index)] =
        control_w[static_cast<std::size_t>(index)]
        + lattice_operator_k(control_state, control_j, index) * 0.5
        - lattice_operator_k(control_state, control_w, index) * 0.5;
  }

  const auto target = control_state.lattice().coord(target_index);
  const double interaction = 0.5 * G_C * static_cast<double>(charge);
  const std::array<std::array<int, 3>, 3> units{{
      {{1, 0, 0}}, {{0, 1, 0}}, {{0, 0, 1}}}};
  for (int axis = 0; axis < 3; ++axis) {
    const auto& unit = units[static_cast<std::size_t>(axis)];
    const int plus = control_state.lattice().index(
        target.x + unit[0], target.y + unit[1], target.z + unit[2]);
    const int minus = control_state.lattice().index(
        target.x - unit[0], target.y - unit[1], target.z - unit[2]);
    if (axis == 0) {
      coefficient[static_cast<std::size_t>(plus)].x -= interaction;
      coefficient[static_cast<std::size_t>(minus)].x += interaction;
    } else if (axis == 1) {
      coefficient[static_cast<std::size_t>(plus)].y -= interaction;
      coefficient[static_cast<std::size_t>(minus)].y += interaction;
    } else {
      coefficient[static_cast<std::size_t>(plus)].z -= interaction;
      coefficient[static_cast<std::size_t>(minus)].z += interaction;
    }
  }

  std::array<std::array<long double, 3>, 3> gram{};
  std::array<long double, 3> a_dot_c{};
  long double c_norm2 = 0.0L;
  for (int index = 0; index < count; ++index) {
    if (support[static_cast<std::size_t>(index)] == 0) continue;
    const auto& c = coefficient[static_cast<std::size_t>(index)];
    c_norm2 += dot_long_double(c, c);
    for (int i = 0; i < 3; ++i) {
      const auto& row_i = rows[static_cast<std::size_t>(index)]
          [static_cast<std::size_t>(i)];
      a_dot_c[static_cast<std::size_t>(i)] += dot_long_double(row_i, c);
      for (int j = 0; j < 3; ++j)
        gram[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
            += dot_long_double(row_i,
                rows[static_cast<std::size_t>(index)]
                    [static_cast<std::size_t>(j)]);
    }
  }

  std::array<std::array<long double, 3>, 3> inverse{};
  if (!invert_symmetric_3x3(gram, inverse, result.determinant)) return result;
  const std::array<long double, 3> recoil{{requested_recoil.x,
                                           requested_recoil.y,
                                           requested_recoil.z}};
  std::array<long double, 3> shifted{};
  std::array<long double, 3> lambda{};
  std::array<long double, 3> lambda_c{};
  for (int i = 0; i < 3; ++i)
    shifted[static_cast<std::size_t>(i)] =
        recoil[static_cast<std::size_t>(i)]
        + a_dot_c[static_cast<std::size_t>(i)];
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j) {
      lambda[static_cast<std::size_t>(i)] +=
          inverse[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          * shifted[static_cast<std::size_t>(j)];
      lambda_c[static_cast<std::size_t>(i)] +=
          inverse[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          * a_dot_c[static_cast<std::size_t>(j)];
    }

  long double shifted_norm2 = 0.0L;
  for (int i = 0; i < 3; ++i)
    shifted_norm2 += shifted[static_cast<std::size_t>(i)]
        * lambda[static_cast<std::size_t>(i)];
  result.minimum_total_energy_change =
      0.5L * shifted_norm2 - 0.5L * c_norm2;

  std::vector<Vec3> null_c(static_cast<std::size_t>(count));
  long double null_norm2 = 0.0L;
  for (int index = 0; index < count; ++index) {
    if (support[static_cast<std::size_t>(index)] == 0) continue;
    Vec3 y_min{};
    Vec3 c_projection{};
    for (int axis = 0; axis < 3; ++axis) {
      const auto& row = rows[static_cast<std::size_t>(index)]
          [static_cast<std::size_t>(axis)];
      y_min += row * static_cast<double>(
          lambda[static_cast<std::size_t>(axis)]);
      c_projection += row * static_cast<double>(
          lambda_c[static_cast<std::size_t>(axis)]);
    }
    result.minimum_impulse[static_cast<std::size_t>(index)] =
        y_min - coefficient[static_cast<std::size_t>(index)];
    null_c[static_cast<std::size_t>(index)] =
        coefficient[static_cast<std::size_t>(index)] - c_projection;
    null_norm2 += dot_long_double(
        null_c[static_cast<std::size_t>(index)],
        null_c[static_cast<std::size_t>(index)]);
  }
  result.covariant_null_norm = std::sqrt(std::max(0.0L, null_norm2));

  const auto evaluate = [&](const std::vector<Vec3>& impulse,
                            Vec3& realized) {
    long double energy = 0.0L;
    realized = {};
    for (int index = 0; index < count; ++index) {
      if (support[static_cast<std::size_t>(index)] == 0) continue;
      const auto& value = impulse[static_cast<std::size_t>(index)];
      energy += 0.5L * dot_long_double(value, value)
          + dot_long_double(coefficient[static_cast<std::size_t>(index)],
                            value);
      realized.x += value.dot(rows[static_cast<std::size_t>(index)][0]);
      realized.y += value.dot(rows[static_cast<std::size_t>(index)][1]);
      realized.z += value.dot(rows[static_cast<std::size_t>(index)][2]);
    }
    return energy;
  };

  Vec3 realized_minimum{};
  result.direct_minimum_energy_change = evaluate(
      result.minimum_impulse, realized_minimum);
  result.minimum_momentum_residual =
      (realized_minimum - requested_recoil).mag();

  if (result.minimum_total_energy_change <= 0.0L
      && result.covariant_null_norm > 1e-18L) {
    const long double scale = std::sqrt(
        std::max(0.0L, -2.0L * result.minimum_total_energy_change))
        / result.covariant_null_norm;
    for (int index = 0; index < count; ++index) {
      if (support[static_cast<std::size_t>(index)] == 0) continue;
      const Vec3 y_min = result.minimum_impulse[static_cast<std::size_t>(index)]
          + coefficient[static_cast<std::size_t>(index)];
      result.zero_energy_impulse[static_cast<std::size_t>(index)] =
          y_min + null_c[static_cast<std::size_t>(index)]
              * static_cast<double>(scale)
          - coefficient[static_cast<std::size_t>(index)];
    }
    Vec3 realized_zero{};
    result.direct_zero_energy_change = evaluate(
        result.zero_energy_impulse, realized_zero);
    result.zero_momentum_residual =
        (realized_zero - requested_recoil).mag();
    result.zero_energy_solution = true;
  }

  result.support_exact = true;
  for (int index = 0; index < count; ++index) {
    if (support[static_cast<std::size_t>(index)] != 0) continue;
    result.support_exact = result.support_exact
        && result.minimum_impulse[static_cast<std::size_t>(index)].mag2() == 0.0
        && result.zero_energy_impulse[static_cast<std::size_t>(index)].mag2()
            == 0.0;
  }
  result.valid = result.support_exact
      && std::isfinite(result.minimum_total_energy_change)
      && std::isfinite(result.direct_minimum_energy_change)
      && std::isfinite(result.minimum_momentum_residual)
      && std::isfinite(result.covariant_null_norm);
  return result;
}

}  // namespace ftd::eft
