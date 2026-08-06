#include "ftd/eft/localized_basin_observer.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

double periodic_delta(double value, double reference, int L) {
  double delta = value - reference;
  if (L <= 0) return delta;
  const double period = static_cast<double>(L);
  delta -= period * std::round(delta / period);
  return delta;
}

Vec3 effective_position(const MatchedMatterPoint& point) {
  return {static_cast<double>(point.anchor.x) + point.remainder.x,
          static_cast<double>(point.anchor.y) + point.remainder.y,
          static_cast<double>(point.anchor.z) + point.remainder.z};
}

std::vector<Vec3> unwrapped_positions(
    const std::vector<MatchedMatterPoint>& points,
    int L) {
  std::vector<Vec3> result;
  if (points.empty()) return result;
  result.reserve(points.size());
  const Vec3 pivot = effective_position(points.front());
  for (const auto& point : points) {
    const Vec3 position = effective_position(point);
    result.push_back({
        pivot.x + periodic_delta(position.x, pivot.x, L),
        pivot.y + periodic_delta(position.y, pivot.y, L),
        pivot.z + periodic_delta(position.z, pivot.z, L)});
  }
  return result;
}

Vec3 mean(const std::vector<Vec3>& values) {
  Vec3 result{};
  if (values.empty()) return result;
  for (const auto& value : values) result += value;
  return result * (1.0 / static_cast<double>(values.size()));
}

bool same_topology(const ConnectedMooreBlockState& reference,
                   const ConnectedMooreBlockState& candidate,
                   double tolerance) {
  if (reference.electric.L != candidate.electric.L
      || reference.magnetic_half.L != candidate.magnetic_half.L
      || reference.constituents.size() != candidate.constituents.size()
      || reference.charges != candidate.charges
      || reference.edges.size() != candidate.edges.size()
      || reference.width != candidate.width
      || reference.orientation_axis != candidate.orientation_axis)
    return false;
  for (std::size_t index = 0; index < reference.edges.size(); ++index) {
    const auto& left = reference.edges[index];
    const auto& right = candidate.edges[index];
    if (left.first != right.first || left.second != right.second
        || left.reference_delta.x != right.reference_delta.x
        || left.reference_delta.y != right.reference_delta.y
        || left.reference_delta.z != right.reference_delta.z
        || std::abs(left.rest_length_squared - right.rest_length_squared)
               > tolerance)
      return false;
  }
  return true;
}

double periodic_radius(int x, int y, int z, const Vec3& origin, int L) {
  return std::max({
      std::abs(periodic_delta(static_cast<double>(x), origin.x, L)),
      std::abs(periodic_delta(static_cast<double>(y), origin.y, L)),
      std::abs(periodic_delta(static_cast<double>(z), origin.z, L))});
}

void add_shell_energy(double energy,
                      double radius,
                      int inner_radius,
                      int outer_radius,
                      LocalizedBasinObservation& result) {
  if (radius <= static_cast<double>(inner_radius))
    result.near_dynamic_field += energy;
  else if (radius <= static_cast<double>(outer_radius))
    result.intermediate_dynamic_field += energy;
  else
    result.far_dynamic_field += energy;
}

double edge_length_squared(const MooreBindingEdge& edge,
                           const std::vector<Vec3>& positions) {
  if (edge.first >= positions.size() || edge.second >= positions.size())
    return std::numeric_limits<double>::infinity();
  const Vec3 delta = positions[edge.second] - positions[edge.first];
  return delta.mag2();
}

}  // namespace

LocalizedBasinObservation observe_localized_basin(
    const ConnectedMooreBlockState& reference,
    const ConnectedMooreBlockState& candidate,
    const Vec3& origin,
    int inner_radius,
    int outer_radius,
    double reference_frequency,
    double field_energy_scale,
    double wave_speed,
    double constituent_mass,
    double tolerance) {
  LocalizedBasinObservation result;
  result.inner_radius = inner_radius;
  result.outer_radius = outer_radius;
  result.constituent_count = static_cast<int>(reference.constituents.size());
  result.mass = constituent_mass;
  result.reference_frequency = reference_frequency;
  result.field_energy_scale = field_energy_scale;
  result.wave_speed = wave_speed;
  result.topology_match = same_topology(reference, candidate, tolerance);
  if (!result.topology_match || reference.constituents.empty()
      || reference.electric.L <= 0 || inner_radius < 0
      || outer_radius < inner_radius || !(reference_frequency > 0.0)
      || !(field_energy_scale > 0.0) || !(wave_speed > 0.0)
      || !(constituent_mass > 0.0))
    return result;

  const int L = reference.electric.L;
  const auto reference_positions = unwrapped_positions(reference.constituents, L);
  const auto candidate_positions = unwrapped_positions(candidate.constituents, L);
  std::vector<Vec3> reference_momenta;
  std::vector<Vec3> candidate_momenta;
  reference_momenta.reserve(reference.constituents.size());
  candidate_momenta.reserve(candidate.constituents.size());
  for (std::size_t index = 0; index < reference.constituents.size(); ++index) {
    reference_momenta.push_back(reference.constituents[index].momentum);
    candidate_momenta.push_back(candidate.constituents[index].momentum);
  }
  const Vec3 reference_center = mean(reference_positions);
  const Vec3 candidate_center = mean(candidate_positions);
  const Vec3 reference_momentum = mean(reference_momenta);
  const Vec3 candidate_momentum = mean(candidate_momenta);
  result.center_offset = candidate_center - reference_center;
  result.mean_momentum_offset = candidate_momentum - reference_momentum;
  result.center_offset_norm = result.center_offset.mag();
  result.mean_momentum_offset_norm = result.mean_momentum_offset.mag();

  for (std::size_t index = 0; index < reference.constituents.size(); ++index) {
    const Vec3 position_offset =
        (candidate_positions[index] - candidate_center)
        - (reference_positions[index] - reference_center);
    const Vec3 momentum_offset =
        (candidate_momenta[index] - candidate_momentum)
        - (reference_momenta[index] - reference_momentum);
    result.internal_position_metric += constituent_mass * position_offset.mag2();
    result.internal_momentum_metric += momentum_offset.mag2() / constituent_mass;
    result.maximum_internal_position_offset = std::max(
        result.maximum_internal_position_offset, position_offset.mag());
    result.maximum_internal_momentum_offset = std::max(
        result.maximum_internal_momentum_offset, momentum_offset.mag());
  }
  result.core_phase_metric = reference_frequency * reference_frequency
          * result.internal_position_metric
      + result.internal_momentum_metric;

  for (std::size_t index = 0; index < reference.edges.size(); ++index) {
    const double left = edge_length_squared(
        reference.edges[index], reference_positions);
    const double right = edge_length_squared(
        candidate.edges[index], candidate_positions);
    result.maximum_edge_length_difference = std::max(
        result.maximum_edge_length_difference, std::abs(right - left));
  }

  const std::size_t count = reference.electric.x.size();
  if (candidate.electric.x.size() != count
      || reference.electric.y.size() != count
      || reference.electric.z.size() != count
      || candidate.electric.y.size() != count
      || candidate.electric.z.size() != count
      || reference.magnetic_half.x.size() != count
      || reference.magnetic_half.y.size() != count
      || reference.magnetic_half.z.size() != count
      || candidate.magnetic_half.x.size() != count
      || candidate.magnetic_half.y.size() != count
      || candidate.magnetic_half.z.size() != count)
    return result;

  const double electric_scale = 0.5 * field_energy_scale;
  const double magnetic_scale = electric_scale * wave_speed * wave_speed;
  double direct_total = 0.0;
  for (std::size_t index = 0; index < count; ++index) {
    const int z = static_cast<int>(index % static_cast<std::size_t>(L));
    const int y = static_cast<int>(
        (index / static_cast<std::size_t>(L)) % static_cast<std::size_t>(L));
    const int x = static_cast<int>(
        index / (static_cast<std::size_t>(L) * static_cast<std::size_t>(L)));
    const double radius = periodic_radius(x, y, z, origin, L);
    const double dex = candidate.electric.x[index] - reference.electric.x[index];
    const double dey = candidate.electric.y[index] - reference.electric.y[index];
    const double dez = candidate.electric.z[index] - reference.electric.z[index];
    const double dbx = candidate.magnetic_half.x[index]
        - reference.magnetic_half.x[index];
    const double dby = candidate.magnetic_half.y[index]
        - reference.magnetic_half.y[index];
    const double dbz = candidate.magnetic_half.z[index]
        - reference.magnetic_half.z[index];
    const double energy = electric_scale * (dex * dex + dey * dey + dez * dez)
        + magnetic_scale * (dbx * dbx + dby * dby + dbz * dbz);
    direct_total += energy;
    add_shell_energy(energy, radius, inner_radius, outer_radius, result);
  }
  result.total_dynamic_field = direct_total;
  result.field_partition_residual = std::abs(
      direct_total - (result.near_dynamic_field
                      + result.intermediate_dynamic_field
                      + result.far_dynamic_field));
  if (result.total_dynamic_field > 0.0) {
    result.near_fraction = result.near_dynamic_field / result.total_dynamic_field;
    result.far_fraction = result.far_dynamic_field / result.total_dynamic_field;
  }

  result.finite = std::isfinite(result.core_phase_metric)
      && std::isfinite(result.center_offset_norm)
      && std::isfinite(result.mean_momentum_offset_norm)
      && std::isfinite(result.total_dynamic_field)
      && std::isfinite(result.field_partition_residual);
  result.valid = result.finite && result.field_partition_residual <= tolerance
      && result.internal_position_metric >= 0.0
      && result.internal_momentum_metric >= 0.0
      && result.near_dynamic_field >= 0.0
      && result.intermediate_dynamic_field >= 0.0
      && result.far_dynamic_field >= 0.0;
  return result;
}

}  // namespace ftd::eft
