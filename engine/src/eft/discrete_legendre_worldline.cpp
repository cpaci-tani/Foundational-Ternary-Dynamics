#include "ftd/eft/discrete_legendre_worldline.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

struct Dual6 {
  long double value = 0.0L;
  std::array<long double, 6> derivative{};
};

Dual6 constant(long double value) {
  Dual6 result;
  result.value = value;
  return result;
}

Dual6 variable(long double value, int component) {
  Dual6 result = constant(value);
  result.derivative[static_cast<std::size_t>(component)] = 1.0L;
  return result;
}

Dual6 operator+(const Dual6& lhs, const Dual6& rhs) {
  Dual6 result;
  result.value = lhs.value + rhs.value;
  for (int i = 0; i < 6; ++i) {
    result.derivative[static_cast<std::size_t>(i)] =
        lhs.derivative[static_cast<std::size_t>(i)]
        + rhs.derivative[static_cast<std::size_t>(i)];
  }
  return result;
}

Dual6 operator-(const Dual6& lhs, const Dual6& rhs) {
  Dual6 result;
  result.value = lhs.value - rhs.value;
  for (int i = 0; i < 6; ++i) {
    result.derivative[static_cast<std::size_t>(i)] =
        lhs.derivative[static_cast<std::size_t>(i)]
        - rhs.derivative[static_cast<std::size_t>(i)];
  }
  return result;
}

Dual6 operator*(const Dual6& lhs, const Dual6& rhs) {
  Dual6 result;
  result.value = lhs.value * rhs.value;
  for (int i = 0; i < 6; ++i) {
    result.derivative[static_cast<std::size_t>(i)] =
        lhs.derivative[static_cast<std::size_t>(i)] * rhs.value
        + lhs.value * rhs.derivative[static_cast<std::size_t>(i)];
  }
  return result;
}

Dual6 operator*(const Dual6& lhs, long double rhs) {
  return lhs * constant(rhs);
}

Dual6 operator*(long double lhs, const Dual6& rhs) {
  return constant(lhs) * rhs;
}

using DualVec3 = std::array<Dual6, 3>;

double component(const Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

Dual6 component(const DualVec3& value, int axis) {
  return value[static_cast<std::size_t>(axis)];
}

Vec3 derivative_vec(const Dual6& value, int offset) {
  return {static_cast<double>(value.derivative[
              static_cast<std::size_t>(offset)]),
          static_cast<double>(value.derivative[
              static_cast<std::size_t>(offset + 1)]),
          static_cast<double>(value.derivative[
              static_cast<std::size_t>(offset + 2)])};
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

Coord common_strict_cell(const Vec3& start, const Vec3& end) {
  return {static_cast<int>(std::floor(0.5 * (start.x + end.x))),
          static_cast<int>(std::floor(0.5 * (start.y + end.y))),
          static_cast<int>(std::floor(0.5 * (start.z + end.z)))};
}

bool strictly_inside(const Vec3& start, const Vec3& end,
                     const Coord& cell, int L) {
  constexpr double tolerance = 128.0
      * std::numeric_limits<double>::epsilon();
  const int lower[3] = {cell.x, cell.y, cell.z};
  for (int axis = 0; axis < 3; ++axis) {
    if (lower[axis] < 0 || lower[axis] >= L) return false;
    const double a = component(start, axis);
    const double b = component(end, axis);
    if (std::min(a, b) <= lower[axis] + tolerance
        || std::max(a, b) >= lower[axis] + 1.0 - tolerance) {
      return false;
    }
  }
  return true;
}

Dual6 interpolate_link(const MatchedFaceFlux& potential,
                       int link_axis,
                       const Coord& cell,
                       const DualVec3& position) {
  const int transverse_a = (link_axis + 1) % 3;
  const int transverse_b = (link_axis + 2) % 3;
  const int lower[3] = {cell.x, cell.y, cell.z};
  const Dual6 fa = component(position, transverse_a)
      - constant(lower[transverse_a]);
  const Dual6 fb = component(position, transverse_b)
      - constant(lower[transverse_b]);
  const std::vector<double>* field = link_axis == 0 ? &potential.x
      : (link_axis == 1 ? &potential.y : &potential.z);
  Dual6 result = constant(0.0L);
  for (int ba = 0; ba <= 1; ++ba) {
    const Dual6 wa = ba == 0 ? constant(1.0L) - fa : fa;
    for (int bb = 0; bb <= 1; ++bb) {
      const Dual6 wb = bb == 0 ? constant(1.0L) - fb : fb;
      int coordinate[3] = {cell.x, cell.y, cell.z};
      coordinate[transverse_a] += ba;
      coordinate[transverse_b] += bb;
      const int index = potential.index(
          coordinate[0], coordinate[1], coordinate[2]);
      result = result + wa * wb * static_cast<long double>(
          (*field)[static_cast<std::size_t>(index)]);
    }
  }
  return result;
}

Dual6 interpolate_scalar(const DualGaugePotentialSlab& slab,
                         const Coord& cell,
                         const DualVec3& position) {
  const int lower[3] = {cell.x, cell.y, cell.z};
  Dual6 f[3] = {
      component(position, 0) - constant(lower[0]),
      component(position, 1) - constant(lower[1]),
      component(position, 2) - constant(lower[2])};
  Dual6 result = constant(0.0L);
  for (int dx = 0; dx <= 1; ++dx) {
    const Dual6 wx = dx == 0 ? constant(1.0L) - f[0] : f[0];
    for (int dy = 0; dy <= 1; ++dy) {
      const Dual6 wy = dy == 0 ? constant(1.0L) - f[1] : f[1];
      for (int dz = 0; dz <= 1; ++dz) {
        const Dual6 wz = dz == 0 ? constant(1.0L) - f[2] : f[2];
        const int index = slab.index(
            cell.x + dx, cell.y + dy, cell.z + dz);
        result = result + wx * wy * wz * static_cast<long double>(
            slab.Phi[static_cast<std::size_t>(index)]);
      }
    }
  }
  return result;
}

Dual6 interaction_action(const DualGaugePotentialSlab& slab,
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
  Dual6 integral = constant(0.0L);
  for (long double tau : nodes) {
    DualVec3 position{};
    for (int axis = 0; axis < 3; ++axis) {
      position[static_cast<std::size_t>(axis)] =
          start[static_cast<std::size_t>(axis)]
          + tau * displacement[static_cast<std::size_t>(axis)];
    }
    Dual6 integrand = constant(0.0L);
    for (int axis = 0; axis < 3; ++axis) {
      const Dual6 a0 = interpolate_link(
          slab.A_start, axis, cell, position);
      const Dual6 a1 = interpolate_link(
          slab.A_end, axis, cell, position);
      integrand = integrand
          + ((1.0L - tau) * a0 + tau * a1)
          * displacement[static_cast<std::size_t>(axis)];
    }
    integrand = integrand
        - static_cast<long double>(slab.temporal_scale)
        * interpolate_scalar(slab, cell, position);
    integral = integral + 0.5L * integrand;
  }
  return static_cast<long double>(charge)
      * static_cast<long double>(coupling) * integral;
}

DualVec3 endpoint_variables(const Vec3& position, int offset) {
  return {variable(position.x, offset),
          variable(position.y, offset + 1),
          variable(position.z, offset + 2)};
}

Vec3 interpolate_connection(const MatchedFaceFlux& potential,
                            const Coord& cell,
                            const Vec3& position) {
  const DualVec3 fixed = {constant(position.x),
                          constant(position.y),
                          constant(position.z)};
  return {static_cast<double>(interpolate_link(
              potential, 0, cell, fixed).value),
          static_cast<double>(interpolate_link(
              potential, 1, cell, fixed).value),
          static_cast<double>(interpolate_link(
              potential, 2, cell, fixed).value)};
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

double squared_norm(const Vec3& value) {
  return value.x * value.x + value.y * value.y + value.z * value.z;
}

}  // namespace

DiscreteLegendreWorldlineResult evaluate_discrete_legendre_worldline(
    const Vec3& start_position,
    const Vec3& end_position,
    int charge,
    double rest_energy,
    double c_speed,
    const DualGaugePotentialSlab& slab,
    double coupling) {
  DiscreteLegendreWorldlineResult result;
  result.L = slab.L;
  result.charge = charge;
  result.rest_energy = rest_energy;
  result.c_speed = c_speed;
  result.temporal_scale = slab.temporal_scale;
  result.coupling = coupling;
  result.start_position = start_position;
  result.end_position = end_position;
  result.displacement = end_position - start_position;
  if ((charge != -1 && charge != 1) || !finite(start_position)
      || !finite(end_position) || !finite(slab)
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0
      || !std::isfinite(coupling)) {
    return result;
  }
  const Coord cell = common_strict_cell(start_position, end_position);
  if (!strictly_inside(start_position, end_position, cell, slab.L)) {
    return result;
  }
  const double lambda_squared = slab.temporal_scale * slab.temporal_scale;
  const double displacement_squared = squared_norm(result.displacement);
  if (!(displacement_squared < lambda_squared)) return result;
  const double gamma_denominator = std::sqrt(
      1.0 - displacement_squared / lambda_squared);
  const double momentum_scale = rest_energy
      / (c_speed * slab.temporal_scale * gamma_denominator);
  const Vec3 free_momentum = result.displacement * momentum_scale;
  result.matter_action = -rest_energy * slab.temporal_scale / c_speed
      * gamma_denominator;
  result.d1_matter = free_momentum * -1.0;
  result.d2_matter = free_momentum;

  const Dual6 interaction = interaction_action(
      slab, cell, endpoint_variables(start_position, 0),
      endpoint_variables(end_position, 3), charge, coupling);
  result.interaction_action = static_cast<double>(interaction.value);
  result.d1_interaction = derivative_vec(interaction, 0);
  result.d2_interaction = derivative_vec(interaction, 3);
  result.total_action = result.matter_action + result.interaction_action;
  result.canonical_start = (result.d1_matter
      + result.d1_interaction) * -1.0;
  result.canonical_end = result.d2_matter + result.d2_interaction;
  result.connection_start = interpolate_connection(
      slab.A_start, cell, start_position);
  result.connection_end = interpolate_connection(
      slab.A_end, cell, end_position);
  const double charge_coupling = coupling * charge;
  result.kinetic_start = result.canonical_start
      - result.connection_start * charge_coupling;
  result.kinetic_end = result.canonical_end
      - result.connection_end * charge_coupling;
  const double deposited = deposited_action(
      start_position, end_position, charge, slab, coupling);
  result.deposited_action_residual = std::abs(
      result.interaction_action - deposited);
  const double momentum_squared = squared_norm(free_momentum);
  const double energy = std::sqrt(
      rest_energy * rest_energy
      + c_speed * c_speed * momentum_squared);
  const Vec3 recovered = free_displacement_from_momentum(
      free_momentum, rest_energy, c_speed, slab.temporal_scale);
  result.dispersion_residual = std::max({
      std::abs(recovered.x - result.displacement.x),
      std::abs(recovered.y - result.displacement.y),
      std::abs(recovered.z - result.displacement.z),
      std::abs(energy * energy - rest_energy * rest_energy
          - c_speed * c_speed * momentum_squared)});
  result.valid = finite(result.d1_interaction)
      && finite(result.d2_interaction)
      && finite(result.canonical_start) && finite(result.canonical_end)
      && finite(result.kinetic_start) && finite(result.kinetic_end)
      && std::isfinite(result.matter_action)
      && std::isfinite(result.interaction_action)
      && std::isfinite(result.deposited_action_residual)
      && std::isfinite(result.dispersion_residual);
  return result;
}

Vec3 free_displacement_from_momentum(const Vec3& momentum,
                                     double rest_energy,
                                     double c_speed,
                                     double temporal_scale) {
  if (!finite(momentum) || !std::isfinite(rest_energy)
      || rest_energy <= 0.0 || !std::isfinite(c_speed)
      || c_speed <= 0.0 || !std::isfinite(temporal_scale)
      || temporal_scale <= 0.0) {
    return {};
  }
  const double energy = std::sqrt(rest_energy * rest_energy
      + c_speed * c_speed * squared_norm(momentum));
  return momentum * (temporal_scale * c_speed / energy);
}

}  // namespace ftd::eft
