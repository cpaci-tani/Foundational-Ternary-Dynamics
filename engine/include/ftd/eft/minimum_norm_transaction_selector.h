#pragma once
/**
 * @file minimum_norm_transaction_selector.h
 * @brief Independent certificate for the minimum-norm zero-energy selector.
 */

#include "ftd/eft/supported_paired_recoil_capacity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace ftd::eft {

struct MinimumNormSelectorCertificate {
  int support_sites = 0;
  int ambient_dimension = 0;
  int constraint_rank = 0;
  int nullity = 0;
  int zero_shell_dimension = 0;
  long double determinant = 0.0L;
  long double radius = 0.0L;
  long double projected_coefficient_norm = 0.0L;
  long double selected_norm2 = 0.0L;
  long double norm2_lower_bound = 0.0L;
  long double norm2_bound_residual = 0.0L;
  long double radius_residual = 0.0L;
  long double projected_minimum_residual = 0.0L;
  long double selected_direction_residual = 0.0L;
  long double selected_energy_residual = 0.0L;
  double selected_momentum_residual = 0.0;
  double projected_coefficient_momentum_residual = 0.0;
  int alternative_count = 0;
  long double minimum_alternative_norm2_excess =
      std::numeric_limits<long double>::infinity();
  long double worst_alternative_energy_residual = 0.0L;
  double worst_alternative_momentum_residual = 0.0;
  std::vector<Vec3> projected_coefficient;
  std::vector<Vec3> selected_impulse;
  bool tangent_found = false;
  bool valid = false;
};

inline MinimumNormSelectorCertificate certify_minimum_norm_selector(
    const RenderBridge& old_state, const RenderBridge& control_state,
    int target_index, std::int8_t charge, const Vec3& requested_recoil,
    const std::vector<std::uint8_t>& support,
    const SupportedPairedRecoilCapacity& capacity) {
  MinimumNormSelectorCertificate result;
  const int count = static_cast<int>(old_state.voxels().size());
  if (!capacity.valid || !capacity.zero_energy_solution
      || control_state.voxels().size() != old_state.voxels().size()
      || support.size() != old_state.voxels().size()
      || target_index < 0 || target_index >= count || charge == 0)
    return result;

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
  for (int index = 0; index < count; ++index) {
    if (support[static_cast<std::size_t>(index)] == 0) continue;
    ++result.support_sites;
    const auto& c = coefficient[static_cast<std::size_t>(index)];
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
  result.ambient_dimension = 3 * result.support_sites;
  std::array<std::array<long double, 3>, 3> inverse{};
  if (!invert_symmetric_3x3(gram, inverse, result.determinant)) return result;
  result.constraint_rank = 3;
  result.nullity = result.ambient_dimension - result.constraint_rank;
  result.zero_shell_dimension = result.nullity - 1;

  const auto apply_a = [&](const std::vector<Vec3>& value) {
    Vec3 output{};
    for (int index = 0; index < count; ++index) {
      if (support[static_cast<std::size_t>(index)] == 0) continue;
      output.x += value[static_cast<std::size_t>(index)].dot(
          rows[static_cast<std::size_t>(index)][0]);
      output.y += value[static_cast<std::size_t>(index)].dot(
          rows[static_cast<std::size_t>(index)][1]);
      output.z += value[static_cast<std::size_t>(index)].dot(
          rows[static_cast<std::size_t>(index)][2]);
    }
    return output;
  };
  const auto range_projection = [&](const std::vector<Vec3>& value) {
    const Vec3 applied = apply_a(value);
    const std::array<long double, 3> rhs{{applied.x, applied.y, applied.z}};
    std::array<long double, 3> lambda{};
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        lambda[static_cast<std::size_t>(i)] +=
            inverse[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
            * rhs[static_cast<std::size_t>(j)];
    std::vector<Vec3> projection(static_cast<std::size_t>(count));
    for (int index = 0; index < count; ++index) {
      if (support[static_cast<std::size_t>(index)] == 0) continue;
      for (int axis = 0; axis < 3; ++axis)
        projection[static_cast<std::size_t>(index)] +=
            rows[static_cast<std::size_t>(index)]
                [static_cast<std::size_t>(axis)]
            * static_cast<double>(lambda[static_cast<std::size_t>(axis)]);
    }
    return projection;
  };
  const auto norm2 = [&](const std::vector<Vec3>& value) {
    long double output = 0.0L;
    for (int index = 0; index < count; ++index) {
      if (support[static_cast<std::size_t>(index)] == 0) continue;
      output += dot_long_double(value[static_cast<std::size_t>(index)],
                                value[static_cast<std::size_t>(index)]);
    }
    return output;
  };
  const auto inner = [&](const std::vector<Vec3>& a,
                         const std::vector<Vec3>& b) {
    long double output = 0.0L;
    for (int index = 0; index < count; ++index) {
      if (support[static_cast<std::size_t>(index)] == 0) continue;
      output += dot_long_double(a[static_cast<std::size_t>(index)],
                                b[static_cast<std::size_t>(index)]);
    }
    return output;
  };
  const auto evaluate_energy = [&](const std::vector<Vec3>& value) {
    long double output = 0.0L;
    for (int index = 0; index < count; ++index) {
      if (support[static_cast<std::size_t>(index)] == 0) continue;
      const auto& item = value[static_cast<std::size_t>(index)];
      output += 0.5L * dot_long_double(item, item)
          + dot_long_double(coefficient[static_cast<std::size_t>(index)], item);
    }
    return output;
  };

  std::vector<Vec3> supported_c(static_cast<std::size_t>(count));
  for (int index = 0; index < count; ++index)
    if (support[static_cast<std::size_t>(index)] != 0)
      supported_c[static_cast<std::size_t>(index)] =
          coefficient[static_cast<std::size_t>(index)];
  const auto range_c = range_projection(supported_c);
  result.projected_coefficient.assign(static_cast<std::size_t>(count), {});
  for (int index = 0; index < count; ++index)
    if (support[static_cast<std::size_t>(index)] != 0)
      result.projected_coefficient[static_cast<std::size_t>(index)] =
          supported_c[static_cast<std::size_t>(index)]
          - range_c[static_cast<std::size_t>(index)];
  const long double projected_norm2 = norm2(result.projected_coefficient);
  result.projected_coefficient_norm =
      std::sqrt(std::max(0.0L, projected_norm2));
  result.projected_coefficient_momentum_residual =
      apply_a(result.projected_coefficient).mag();

  const auto range_minimum = range_projection(capacity.minimum_impulse);
  long double projected_minimum_error2 = 0.0L;
  for (int index = 0; index < count; ++index) {
    if (support[static_cast<std::size_t>(index)] == 0) continue;
    const Vec3 residual = capacity.minimum_impulse[static_cast<std::size_t>(index)]
        - range_minimum[static_cast<std::size_t>(index)]
        + result.projected_coefficient[static_cast<std::size_t>(index)];
    projected_minimum_error2 += dot_long_double(residual, residual);
  }
  result.projected_minimum_residual =
      std::sqrt(std::max(0.0L, projected_minimum_error2));

  result.radius = std::sqrt(std::max(
      0.0L, -2.0L * capacity.minimum_total_energy_change));
  std::vector<Vec3> selected_offset(static_cast<std::size_t>(count));
  result.selected_impulse.assign(static_cast<std::size_t>(count), {});
  for (int index = 0; index < count; ++index) {
    selected_offset[static_cast<std::size_t>(index)] =
        capacity.zero_energy_impulse[static_cast<std::size_t>(index)]
        - capacity.minimum_impulse[static_cast<std::size_t>(index)];
    if (support[static_cast<std::size_t>(index)] != 0
        && result.projected_coefficient_norm > 0.0L) {
      result.selected_impulse[static_cast<std::size_t>(index)] =
          capacity.minimum_impulse[static_cast<std::size_t>(index)]
          + result.projected_coefficient[static_cast<std::size_t>(index)]
              * static_cast<double>(
                  result.radius / result.projected_coefficient_norm);
    }
  }
  result.radius_residual = std::abs(
      norm2(selected_offset) - result.radius * result.radius);
  long double selected_direction_error2 = 0.0L;
  for (int index = 0; index < count; ++index) {
    if (support[static_cast<std::size_t>(index)] == 0) continue;
    const Vec3 residual = result.selected_impulse[static_cast<std::size_t>(index)]
        - capacity.zero_energy_impulse[static_cast<std::size_t>(index)];
    selected_direction_error2 += dot_long_double(residual, residual);
  }
  result.selected_direction_residual =
      std::sqrt(std::max(0.0L, selected_direction_error2));
  result.selected_norm2 = norm2(capacity.zero_energy_impulse);
  result.norm2_lower_bound = norm2(capacity.minimum_impulse)
      + result.radius * result.radius
      - 2.0L * result.radius * result.projected_coefficient_norm;
  result.norm2_bound_residual =
      std::abs(result.selected_norm2 - result.norm2_lower_bound);
  result.selected_energy_residual =
      std::abs(evaluate_energy(capacity.zero_energy_impulse));
  result.selected_momentum_residual =
      (apply_a(capacity.zero_energy_impulse) - requested_recoil).mag();

  std::vector<Vec3> tangent(static_cast<std::size_t>(count));
  for (int index = 0; index < count && !result.tangent_found; ++index) {
    if (support[static_cast<std::size_t>(index)] == 0) continue;
    for (int component = 0; component < 3 && !result.tangent_found;
         ++component) {
      std::vector<Vec3> basis(static_cast<std::size_t>(count));
      if (component == 0) basis[static_cast<std::size_t>(index)].x = 1.0;
      if (component == 1) basis[static_cast<std::size_t>(index)].y = 1.0;
      if (component == 2) basis[static_cast<std::size_t>(index)].z = 1.0;
      const auto range_basis = range_projection(basis);
      for (int site = 0; site < count; ++site)
        if (support[static_cast<std::size_t>(site)] != 0)
          tangent[static_cast<std::size_t>(site)] =
              basis[static_cast<std::size_t>(site)]
              - range_basis[static_cast<std::size_t>(site)];
      if (projected_norm2 > 0.0L) {
        const long double along = inner(tangent, result.projected_coefficient)
            / projected_norm2;
        for (int site = 0; site < count; ++site)
          if (support[static_cast<std::size_t>(site)] != 0)
            tangent[static_cast<std::size_t>(site)] -=
                result.projected_coefficient[static_cast<std::size_t>(site)]
                * static_cast<double>(along);
      }
      const long double tangent_norm2 = norm2(tangent);
      if (tangent_norm2 > 1e-20L) {
        const double inverse_norm = static_cast<double>(
            1.0L / std::sqrt(tangent_norm2));
        for (auto& item : tangent) item *= inverse_norm;
        result.tangent_found = true;
      }
    }
  }

  if (result.tangent_found && result.radius > 0.0L
      && result.projected_coefficient_norm > 0.0L) {
    const std::array<long double, 5> angles{{
        PI / 8.0L, PI / 4.0L, PI / 2.0L, 3.0L * PI / 4.0L, PI}};
    for (long double angle : angles) {
      std::vector<Vec3> alternative(static_cast<std::size_t>(count));
      for (int index = 0; index < count; ++index) {
        if (support[static_cast<std::size_t>(index)] == 0) continue;
        const Vec3 shell_offset =
            result.projected_coefficient[static_cast<std::size_t>(index)]
                * static_cast<double>(
                    result.radius / result.projected_coefficient_norm
                    * std::cos(angle))
            + tangent[static_cast<std::size_t>(index)]
                * static_cast<double>(result.radius * std::sin(angle));
        alternative[static_cast<std::size_t>(index)] =
            capacity.minimum_impulse[static_cast<std::size_t>(index)]
            + shell_offset;
      }
      ++result.alternative_count;
      result.minimum_alternative_norm2_excess = std::min(
          result.minimum_alternative_norm2_excess,
          norm2(alternative) - result.selected_norm2);
      result.worst_alternative_energy_residual = std::max(
          result.worst_alternative_energy_residual,
          std::abs(evaluate_energy(alternative)));
      result.worst_alternative_momentum_residual = std::max(
          result.worst_alternative_momentum_residual,
          (apply_a(alternative) - requested_recoil).mag());
    }
  }

  result.valid = result.constraint_rank == 3
      && result.ambient_dimension == 108
      && result.nullity == 105
      && result.zero_shell_dimension == 104
      && capacity.minimum_total_energy_change < -1e-8L
      && result.radius > 1e-8L
      && result.projected_coefficient_norm > 1e-8L
      && result.tangent_found && result.alternative_count == 5
      && std::isfinite(result.selected_norm2)
      && std::isfinite(result.minimum_alternative_norm2_excess);
  return result;
}

}  // namespace ftd::eft

