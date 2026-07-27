#include "ftd/eft/two_slab_variational_force.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

struct Dual3 {
  long double value = 0.0L;
  std::array<long double, 3> derivative{};
};

Dual3 constant(long double value) {
  Dual3 result;
  result.value = value;
  return result;
}

Dual3 variable(long double value, int axis) {
  Dual3 result = constant(value);
  result.derivative[static_cast<std::size_t>(axis)] = 1.0L;
  return result;
}

Dual3 operator+(const Dual3& lhs, const Dual3& rhs) {
  Dual3 result;
  result.value = lhs.value + rhs.value;
  for (int axis = 0; axis < 3; ++axis) {
    result.derivative[static_cast<std::size_t>(axis)] =
        lhs.derivative[static_cast<std::size_t>(axis)]
        + rhs.derivative[static_cast<std::size_t>(axis)];
  }
  return result;
}

Dual3 operator-(const Dual3& lhs, const Dual3& rhs) {
  Dual3 result;
  result.value = lhs.value - rhs.value;
  for (int axis = 0; axis < 3; ++axis) {
    result.derivative[static_cast<std::size_t>(axis)] =
        lhs.derivative[static_cast<std::size_t>(axis)]
        - rhs.derivative[static_cast<std::size_t>(axis)];
  }
  return result;
}

Dual3 operator*(const Dual3& lhs, const Dual3& rhs) {
  Dual3 result;
  result.value = lhs.value * rhs.value;
  for (int axis = 0; axis < 3; ++axis) {
    result.derivative[static_cast<std::size_t>(axis)] =
        lhs.derivative[static_cast<std::size_t>(axis)] * rhs.value
        + lhs.value * rhs.derivative[static_cast<std::size_t>(axis)];
  }
  return result;
}

Dual3 operator*(const Dual3& lhs, long double rhs) {
  return lhs * constant(rhs);
}

Dual3 operator*(long double lhs, const Dual3& rhs) {
  return constant(lhs) * rhs;
}

using DualVec3 = std::array<Dual3, 3>;

double component(const Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

Vec3 to_vec3(const std::array<long double, 3>& value) {
  return {static_cast<double>(value[0]),
          static_cast<double>(value[1]),
          static_cast<double>(value[2])};
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

bool finite(const DualGaugePotentialSlab& slab) {
  const std::size_t count = slab.L > 0
      ? static_cast<std::size_t>(slab.L * slab.L * slab.L) : 0;
  return slab.L > 0 && std::isfinite(slab.temporal_scale)
      && slab.temporal_scale > 0.0
      && slab.A_start.L == slab.L && slab.A_end.L == slab.L
      && slab.A_start.x.size() == count
      && slab.A_start.y.size() == count
      && slab.A_start.z.size() == count
      && slab.A_end.x.size() == count
      && slab.A_end.y.size() == count
      && slab.A_end.z.size() == count
      && slab.Phi.size() == count
      && finite(slab.A_start.x) && finite(slab.A_start.y)
      && finite(slab.A_start.z) && finite(slab.A_end.x)
      && finite(slab.A_end.y) && finite(slab.A_end.z)
      && finite(slab.Phi);
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double value = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    value = std::max(value, std::abs(lhs[i] - rhs[i]));
  }
  return value;
}

double max_difference(const MatchedFaceFlux& lhs,
                      const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L) return INFINITY;
  return std::max({max_difference(lhs.x, rhs.x),
                   max_difference(lhs.y, rhs.y),
                   max_difference(lhs.z, rhs.z)});
}

Coord containing_cell(const Vec3& start, const Vec3& end) {
  return {static_cast<int>(std::floor(0.5 * (start.x + end.x))),
          static_cast<int>(std::floor(0.5 * (start.y + end.y))),
          static_cast<int>(std::floor(0.5 * (start.z + end.z)))};
}

bool lies_in_cell(const Vec3& start, const Vec3& end,
                  const Coord& cell, int L) {
  constexpr double tolerance = 32.0
      * std::numeric_limits<double>::epsilon();
  const int lower[3] = {cell.x, cell.y, cell.z};
  for (int axis = 0; axis < 3; ++axis) {
    const double a = component(start, axis);
    const double b = component(end, axis);
    if (lower[axis] < 0 || lower[axis] >= L
        || std::min(a, b) < lower[axis] - tolerance
        || std::max(a, b) > lower[axis] + 1.0 + tolerance) {
      return false;
    }
  }
  return true;
}

Dual3 component(const DualVec3& value, int axis) {
  return value[static_cast<std::size_t>(axis)];
}

int field_index(const MatchedFaceFlux& field,
                int x, int y, int z) {
  return field.index(x, y, z);
}

Dual3 interpolate_link_component(
    const MatchedFaceFlux& potential,
    int link_axis,
    const Coord& cell,
    const DualVec3& position) {
  const int transverse_a = (link_axis + 1) % 3;
  const int transverse_b = (link_axis + 2) % 3;
  const int lower[3] = {cell.x, cell.y, cell.z};
  const Dual3 fraction_a = component(position, transverse_a)
      - constant(lower[transverse_a]);
  const Dual3 fraction_b = component(position, transverse_b)
      - constant(lower[transverse_b]);
  Dual3 result = constant(0.0L);
  const std::vector<double>* field = link_axis == 0 ? &potential.x
      : (link_axis == 1 ? &potential.y : &potential.z);
  for (int bit_a = 0; bit_a <= 1; ++bit_a) {
    const Dual3 weight_a = bit_a == 0
        ? constant(1.0L) - fraction_a : fraction_a;
    for (int bit_b = 0; bit_b <= 1; ++bit_b) {
      const Dual3 weight_b = bit_b == 0
          ? constant(1.0L) - fraction_b : fraction_b;
      int coordinate[3] = {cell.x, cell.y, cell.z};
      coordinate[transverse_a] += bit_a;
      coordinate[transverse_b] += bit_b;
      const int index = field_index(potential,
          coordinate[0], coordinate[1], coordinate[2]);
      result = result + weight_a * weight_b
          * static_cast<long double>((*field)[static_cast<std::size_t>(index)]);
    }
  }
  return result;
}

Dual3 interpolate_site_scalar(
    const DualGaugePotentialSlab& slab,
    const Coord& cell,
    const DualVec3& position) {
  const int lower[3] = {cell.x, cell.y, cell.z};
  Dual3 fraction[3] = {
      component(position, 0) - constant(lower[0]),
      component(position, 1) - constant(lower[1]),
      component(position, 2) - constant(lower[2])};
  Dual3 result = constant(0.0L);
  for (int dx = 0; dx <= 1; ++dx) {
    const Dual3 wx = dx == 0 ? constant(1.0L) - fraction[0] : fraction[0];
    for (int dy = 0; dy <= 1; ++dy) {
      const Dual3 wy = dy == 0 ? constant(1.0L) - fraction[1] : fraction[1];
      for (int dz = 0; dz <= 1; ++dz) {
        const Dual3 wz = dz == 0 ? constant(1.0L) - fraction[2] : fraction[2];
        const int index = slab.index(
            cell.x + dx, cell.y + dy, cell.z + dz);
        result = result + wx * wy * wz
            * static_cast<long double>(
                slab.Phi[static_cast<std::size_t>(index)]);
      }
    }
  }
  return result;
}

Dual3 slab_action(const DualGaugePotentialSlab& slab,
                  const Coord& cell,
                  const DualVec3& start,
                  const DualVec3& end,
                  int charge,
                  double coupling) {
  constexpr long double inverse_sqrt_three =
      0.577350269189625764509148780501957456L;
  const long double nodes[2] = {
      0.5L * (1.0L - inverse_sqrt_three),
      0.5L * (1.0L + inverse_sqrt_three)};
  DualVec3 displacement{};
  for (int axis = 0; axis < 3; ++axis) {
    displacement[static_cast<std::size_t>(axis)] =
        end[static_cast<std::size_t>(axis)]
        - start[static_cast<std::size_t>(axis)];
  }
  Dual3 integral = constant(0.0L);
  for (long double tau : nodes) {
    DualVec3 position{};
    for (int axis = 0; axis < 3; ++axis) {
      position[static_cast<std::size_t>(axis)] =
          start[static_cast<std::size_t>(axis)]
          + tau * displacement[static_cast<std::size_t>(axis)];
    }
    Dual3 integrand = constant(0.0L);
    for (int axis = 0; axis < 3; ++axis) {
      const Dual3 before = interpolate_link_component(
          slab.A_start, axis, cell, position);
      const Dual3 after = interpolate_link_component(
          slab.A_end, axis, cell, position);
      const Dual3 at_time = (1.0L - tau) * before + tau * after;
      integrand = integrand + at_time
          * displacement[static_cast<std::size_t>(axis)];
    }
    integrand = integrand - static_cast<long double>(slab.temporal_scale)
        * interpolate_site_scalar(slab, cell, position);
    integral = integral + 0.5L * integrand;
  }
  return static_cast<long double>(charge)
      * static_cast<long double>(coupling) * integral;
}

DualVec3 fixed_position(const Vec3& position) {
  return {constant(position.x), constant(position.y), constant(position.z)};
}

DualVec3 shared_position(const Vec3& position) {
  return {variable(position.x, 0),
          variable(position.y, 1),
          variable(position.z, 2)};
}

void decompose(const Vec3& position, Coord& anchor, Vec3& remainder) {
  anchor = {static_cast<int>(std::floor(position.x)),
            static_cast<int>(std::floor(position.y)),
            static_cast<int>(std::floor(position.z))};
  remainder = {position.x - anchor.x,
               position.y - anchor.y,
               position.z - anchor.z};
}

double deposited_action(const Vec3& start,
                        const Vec3& end,
                        int charge,
                        const DualGaugePotentialSlab& slab,
                        double coupling) {
  Coord start_anchor{};
  Coord end_anchor{};
  Vec3 start_remainder{};
  Vec3 end_remainder{};
  decompose(start, start_anchor, start_remainder);
  decompose(end, end_anchor, end_remainder);
  const auto current = make_spacetime_worldline_current(
      slab.L, start_anchor, start_remainder,
      end_anchor, end_remainder, charge, slab.temporal_scale);
  const std::vector<double> zero(
      static_cast<std::size_t>(slab.L * slab.L * slab.L), 0.0);
  const auto result = evaluate_spacetime_gauge_coupling(
      current, slab, zero, zero, coupling);
  return result.valid ? result.interaction_action : NAN;
}

}  // namespace

TwoSlabVariationalForceResult evaluate_two_slab_variational_force(
    const Vec3& previous_position,
    const Vec3& shared_position_value,
    const Vec3& next_position,
    int charge,
    const DualGaugePotentialSlab& previous_slab,
    const DualGaugePotentialSlab& next_slab,
    double coupling) {
  TwoSlabVariationalForceResult result;
  result.L = previous_slab.L;
  result.charge = charge;
  result.coupling = coupling;
  result.temporal_scale = previous_slab.temporal_scale;
  result.previous_position = previous_position;
  result.shared_position = shared_position_value;
  result.next_position = next_position;
  if ((charge != -1 && charge != 1) || !std::isfinite(coupling)
      || !finite(previous_position) || !finite(shared_position_value)
      || !finite(next_position) || !finite(previous_slab)
      || !finite(next_slab) || previous_slab.L != next_slab.L
      || previous_slab.temporal_scale != next_slab.temporal_scale) {
    return result;
  }

  result.connection_join_residual = max_difference(
      previous_slab.A_end, next_slab.A_start);
  if (result.connection_join_residual > 1e-12) return result;
  const Coord previous_cell = containing_cell(
      previous_position, shared_position_value);
  const Coord next_cell = containing_cell(
      shared_position_value, next_position);
  if (!lies_in_cell(previous_position, shared_position_value,
                    previous_cell, previous_slab.L)
      || !lies_in_cell(shared_position_value, next_position,
                       next_cell, next_slab.L)) {
    return result;
  }

  const DualVec3 shared = shared_position(shared_position_value);
  const Dual3 previous_action = slab_action(
      previous_slab, previous_cell,
      fixed_position(previous_position), shared, charge, coupling);
  const Dual3 next_action = slab_action(
      next_slab, next_cell,
      shared, fixed_position(next_position), charge, coupling);
  const Dual3 total = previous_action + next_action;
  result.interaction_action = static_cast<double>(total.value);
  result.interaction_impulse = to_vec3(total.derivative);

  const double previous_deposited = deposited_action(
      previous_position, shared_position_value, charge,
      previous_slab, coupling);
  const double next_deposited = deposited_action(
      shared_position_value, next_position, charge,
      next_slab, coupling);
  result.previous_deposit_action_residual = std::abs(
      static_cast<double>(previous_action.value) - previous_deposited);
  result.next_deposit_action_residual = std::abs(
      static_cast<double>(next_action.value) - next_deposited);
  result.valid = finite(result.interaction_impulse)
      && std::isfinite(result.interaction_action)
      && std::isfinite(result.previous_deposit_action_residual)
      && std::isfinite(result.next_deposit_action_residual);
  return result;
}

}  // namespace ftd::eft
