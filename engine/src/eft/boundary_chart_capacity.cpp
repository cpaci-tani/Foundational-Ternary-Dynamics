#include "ftd/eft/boundary_chart_capacity.h"

#include "ftd/eft/subcell_polarity_shape.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <tuple>

namespace ftd::eft {
namespace {

using SiteKey = std::tuple<int, int, int>;
using ShapeMap = std::map<SiteKey, long double>;

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double integer_residual(double value) {
  return std::abs(value - std::round(value));
}

int integer_coordinate_count(const Vec3& value) {
  const double scale = std::max({1.0, std::abs(value.x),
                                std::abs(value.y), std::abs(value.z)});
  const double tolerance = 64.0 * std::numeric_limits<double>::epsilon()
      * scale;
  return static_cast<int>(integer_residual(value.x) <= tolerance)
      + static_cast<int>(integer_residual(value.y) <= tolerance)
      + static_cast<int>(integer_residual(value.z) <= tolerance);
}

SiteKey site_key(Coord site) {
  return {site.x, site.y, site.z};
}

ShapeMap shape_map(const SubcellPolarityShape& shape) {
  ShapeMap result;
  if (!shape.valid) return result;
  for (std::size_t i = 0; i < shape.weight_count; ++i) {
    result[site_key(shape.weights[i].site)] += shape.weights[i].weight;
  }
  return result;
}

double map_difference(const ShapeMap& lhs, const ShapeMap& rhs) {
  std::set<SiteKey> keys;
  for (const auto& entry : lhs) keys.insert(entry.first);
  for (const auto& entry : rhs) keys.insert(entry.first);
  long double result = 0.0L;
  for (const auto& key : keys) {
    const auto l = lhs.find(key);
    const auto r = rhs.find(key);
    const long double lv = l == lhs.end() ? 0.0L : l->second;
    const long double rv = r == rhs.end() ? 0.0L : r->second;
    result = std::max(result, std::abs(lv - rv));
  }
  return static_cast<double>(result);
}

double vector_difference(const std::vector<double>& lhs,
                         const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i] - rhs[i]));
  return result;
}

double signature_difference(const PiecewiseCurrentSignature& lhs,
                            const PiecewiseCurrentSignature& rhs) {
  if (!lhs.valid || !rhs.valid) return INFINITY;
  return std::max({
      vector_difference(lhs.rho_before, rhs.rho_before),
      vector_difference(lhs.rho_after, rhs.rho_after),
      vector_difference(lhs.current_x, rhs.current_x),
      vector_difference(lhs.current_y, rhs.current_y),
      vector_difference(lhs.current_z, rhs.current_z)});
}

Vec3 normalized(Coord direction) {
  const Vec3 vector{static_cast<double>(direction.x),
                    static_cast<double>(direction.y),
                    static_cast<double>(direction.z)};
  const double magnitude = vector.mag();
  return magnitude > 0.0 ? vector * (1.0 / magnitude) : Vec3{};
}

Coord subtract(Coord lhs, Coord rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

double coord_residual(Coord lhs, Coord rhs) {
  return static_cast<double>(std::max({std::abs(lhs.x - rhs.x),
                                      std::abs(lhs.y - rhs.y),
                                      std::abs(lhs.z - rhs.z)}));
}

}  // namespace

BoundaryChartCapacityResult analyze_boundary_chart_capacity(
    const Vec3& effective_position,
    int multiplicity,
    int polarity,
    double tolerance) {
  BoundaryChartCapacityResult result;
  result.effective_position = effective_position;
  result.multiplicity = multiplicity;
  result.polarity = polarity;
  if (!finite(effective_position) || multiplicity <= 0
      || (polarity != -1 && polarity != +1)
      || !std::isfinite(tolerance) || tolerance < 0.0) {
    return result;
  }

  result.integer_coordinate_count = integer_coordinate_count(
      effective_position);
  result.expected_chart_count = 1 << (3 - result.integer_coordinate_count);
  result.charts = enumerate_subcell_charts(effective_position);
  result.chart_count = static_cast<int>(result.charts.size());
  std::set<SiteKey> anchors;
  for (const auto& chart : result.charts) {
    anchors.insert(site_key(chart.anchor));
    result.chart_position_residual = std::max(
        result.chart_position_residual,
        max_abs(subcell_chart_position(chart) - effective_position));
  }
  result.distinct_anchor_count = static_cast<int>(anchors.size());
  result.stored_carriers = std::min(multiplicity, result.chart_count);
  result.minimum_missing_charge = std::max(
      0, multiplicity - result.chart_count);
  result.canonical_single_anchor_defect = multiplicity - 1;
  result.minimum_per_anchor_occupancy =
      (multiplicity + result.chart_count - 1) / result.chart_count;
  result.minimum_chart_aware_alphabet_symbols =
      2 * result.minimum_per_anchor_occupancy + 1;
  result.canonical_single_anchor_alphabet_symbols = 2 * multiplicity + 1;

  ShapeMap reference;
  ShapeMap aggregate;
  bool shapes_valid = result.chart_count > 0;
  long double total_charge = 0.0L;
  long double first_x = 0.0L;
  long double first_y = 0.0L;
  long double first_z = 0.0L;
  for (int i = 0; i < result.stored_carriers; ++i) {
    const auto shape = make_subcell_polarity_shape(
        result.charts[static_cast<std::size_t>(i)].anchor,
        result.charts[static_cast<std::size_t>(i)].remainder,
        polarity);
    shapes_valid = shapes_valid && shape.valid;
    const ShapeMap current = shape_map(shape);
    if (i == 0) reference = current;
    result.chart_shape_residual = std::max(
        result.chart_shape_residual, map_difference(reference, current));
    for (const auto& entry : current) {
      aggregate[entry.first] += entry.second;
      const auto [x, y, z] = entry.first;
      total_charge += entry.second;
      first_x += entry.second * static_cast<long double>(x);
      first_y += entry.second * static_cast<long double>(y);
      first_z += entry.second * static_cast<long double>(z);
    }
  }

  ShapeMap expected;
  for (const auto& entry : reference)
    expected[entry.first] = entry.second * result.stored_carriers;
  result.aggregate_shape_residual = map_difference(aggregate, expected);
  result.aggregate_charge_residual = static_cast<double>(
      total_charge - static_cast<long double>(polarity)
          * result.stored_carriers);
  result.aggregate_first_moment_residual = {
      static_cast<double>(first_x - static_cast<long double>(polarity)
          * result.stored_carriers * effective_position.x),
      static_cast<double>(first_y - static_cast<long double>(polarity)
          * result.stored_carriers * effective_position.y),
      static_cast<double>(first_z - static_cast<long double>(polarity)
          * result.stored_carriers * effective_position.z)};

  result.valid = shapes_valid
      && result.chart_count == result.expected_chart_count
      && result.distinct_anchor_count == result.chart_count
      && result.chart_position_residual <= tolerance
      && result.chart_shape_residual <= tolerance
      && result.aggregate_shape_residual <= tolerance
      && std::abs(result.aggregate_charge_residual) <= tolerance
      && max_abs(result.aggregate_first_moment_residual) <= tolerance;
  return result;
}

BoundaryChartCollisionResult analyze_boundary_chart_collision(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double half_step_distance,
    double tolerance) {
  BoundaryChartCollisionResult result;
  result.L = L;
  result.polarity = polarity;
  result.chart_direction = chart_direction;
  result.collision_position = collision_position;
  result.unit_direction = normalized(chart_direction);
  if (L < 3 || !finite(collision_position)
      || (chart_direction.x == 0 && chart_direction.y == 0
          && chart_direction.z == 0)
      || (polarity != -1 && polarity != +1)
      || !std::isfinite(half_step_distance) || half_step_distance <= 0.0
      || !std::isfinite(tolerance) || tolerance < 0.0) {
    return result;
  }

  result.capacity = analyze_boundary_chart_capacity(
      collision_position, 2, polarity, tolerance);
  bool pair_found = false;
  for (const auto& first : result.capacity.charts) {
    for (const auto& second : result.capacity.charts) {
      if (subtract(second.anchor, first.anchor).x == chart_direction.x
          && subtract(second.anchor, first.anchor).y == chart_direction.y
          && subtract(second.anchor, first.anchor).z == chart_direction.z) {
        result.first_chart = first;
        result.second_chart = second;
        pair_found = true;
        break;
      }
    }
    if (pair_found) break;
  }
  if (!pair_found) return result;

  result.anchor_direction_residual = coord_residual(
      subtract(result.second_chart.anchor, result.first_chart.anchor),
      chart_direction);
  result.collision_position_residual = std::max(
      max_abs(subcell_chart_position(result.first_chart) - collision_position),
      max_abs(subcell_chart_position(result.second_chart) - collision_position));

  const Vec3 offset = result.unit_direction * half_step_distance;
  const Vec3 first_endpoint = collision_position - offset;
  const Vec3 second_endpoint = collision_position + offset;
  // On the outgoing slab, bounce and pass-through differ only in which
  // identical incoming label is assigned to each branch.  The physical set of
  // worldline segments is the same and must therefore give the same nonzero
  // aggregate current.
  const std::vector<PiecewiseWorldline> bounce_paths{{
      {polarity, {collision_position, first_endpoint}},
      {polarity, {collision_position, second_endpoint}}}};
  const std::vector<PiecewiseWorldline> pass_paths{{
      {polarity, {collision_position, second_endpoint}},
      {polarity, {collision_position, first_endpoint}}}};
  result.bounce = make_piecewise_current_signature(L, bounce_paths);
  result.pass_through = make_piecewise_current_signature(L, pass_paths);
  result.endpoint_density_residual = std::max(
      vector_difference(result.bounce.rho_before,
                        result.pass_through.rho_before),
      vector_difference(result.bounce.rho_after,
                        result.pass_through.rho_after));
  result.current_quotient_residual = signature_difference(
      result.bounce, result.pass_through);
  result.continuity_residual = std::max(
      result.bounce.continuity_residual,
      result.pass_through.continuity_residual);
  result.valid = result.capacity.valid
      && result.capacity.minimum_missing_charge == 0
      && result.anchor_direction_residual <= tolerance
      && result.collision_position_residual <= tolerance
      && result.bounce.valid && result.pass_through.valid
      && result.endpoint_density_residual <= tolerance
      && result.current_quotient_residual <= tolerance
      && result.continuity_residual <= tolerance;
  return result;
}

}  // namespace ftd::eft
