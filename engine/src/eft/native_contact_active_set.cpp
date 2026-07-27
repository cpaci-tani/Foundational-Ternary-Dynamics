#include "ftd/eft/native_contact_active_set.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

Vec3 coordinate(Coord value) {
  return {static_cast<double>(value.x),
          static_cast<double>(value.y),
          static_cast<double>(value.z)};
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double chart_excess(const Vec3& remainder) {
  return std::max(0.0, max_abs(remainder) - 1.0);
}

double hop_margin(const Vec3& remainder) {
  return 1.0 - max_abs(remainder);
}

double gap(const Coord& first_anchor,
           const Vec3& first_remainder,
           const Coord& second_anchor,
           const Vec3& second_remainder,
           const Vec3& normal) {
  const Vec3 first = coordinate(first_anchor) + first_remainder;
  const Vec3 second = coordinate(second_anchor) + second_remainder;
  return (second - first).dot(normal);
}

int threshold_tick(double time, double tolerance) {
  const double nearest = std::round(time);
  const double scale = std::max(1.0, std::abs(time));
  if (std::abs(time - nearest) <= tolerance * scale)
    return static_cast<int>(nearest);
  return static_cast<int>(std::ceil(time));
}

}  // namespace

NativeContactActiveSetGeometry analyze_native_contact_active_set_geometry(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double tolerance) {
  NativeContactActiveSetGeometry result;
  result.L = L;
  result.polarity = polarity;
  result.chart_direction = chart_direction;
  result.collision_position = collision_position;
  result.speed = speed;
  result.offset = speed;
  if (L < 3 || !finite(collision_position)
      || (chart_direction.x == 0 && chart_direction.y == 0
          && chart_direction.z == 0)
      || (polarity != -1 && polarity != +1)
      || !std::isfinite(speed) || speed <= 0.0 || speed >= 0.5
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;

  result.charts = analyze_boundary_chart_collision(
      L, collision_position, chart_direction, polarity,
      speed, tolerance);
  if (!result.charts.valid) return result;
  result.normal = result.charts.unit_direction;
  result.first_anchor = result.charts.first_chart.anchor;
  result.second_anchor = result.charts.second_chart.anchor;
  result.first_contact_remainder = collision_position
      - coordinate(result.first_anchor);
  result.second_contact_remainder = collision_position
      - coordinate(result.second_anchor);
  const Vec3 displacement = result.normal * speed;
  result.first_separated_remainder = result.first_contact_remainder
      - displacement;
  result.second_separated_remainder = result.second_contact_remainder
      + displacement;
  result.first_crossed_remainder = result.first_contact_remainder
      + displacement;
  result.second_crossed_remainder = result.second_contact_remainder
      - displacement;

  result.separated_gap = gap(
      result.first_anchor, result.first_separated_remainder,
      result.second_anchor, result.second_separated_remainder,
      result.normal);
  result.contact_gap = gap(
      result.first_anchor, result.first_contact_remainder,
      result.second_anchor, result.second_contact_remainder,
      result.normal);
  result.crossed_gap = gap(
      result.first_anchor, result.first_crossed_remainder,
      result.second_anchor, result.second_crossed_remainder,
      result.normal);
  result.expected_separated_gap = 2.0 * speed;
  result.expected_crossed_gap = -2.0 * speed;
  result.gap_residual = std::max({
      std::abs(result.separated_gap-result.expected_separated_gap),
      std::abs(result.contact_gap),
      std::abs(result.crossed_gap-result.expected_crossed_gap)});
  result.stable_chart_residual = std::max({
      chart_excess(result.first_separated_remainder),
      chart_excess(result.second_separated_remainder),
      chart_excess(result.first_contact_remainder),
      chart_excess(result.second_contact_remainder),
      chart_excess(result.first_crossed_remainder),
      chart_excess(result.second_crossed_remainder)});
  result.contact_hop_margin = std::min(
      hop_margin(result.first_contact_remainder),
      hop_margin(result.second_contact_remainder));
  result.crossed_hop_margin = std::min(
      hop_margin(result.first_crossed_remainder),
      hop_margin(result.second_crossed_remainder));
  const Vec3 direction_vector{
      static_cast<double>(chart_direction.x),
      static_cast<double>(chart_direction.y),
      static_cast<double>(chart_direction.z)};
  result.exact_hop_delay = 0.5 * direction_vector.mag() / speed;
  result.predicted_hop_delay_ticks = threshold_tick(
      result.exact_hop_delay, tolerance);
  result.minimum_missing_charge =
      result.charts.capacity.minimum_missing_charge;
  result.same_site_occupancy = result.first_anchor.x
          != result.second_anchor.x
      || result.first_anchor.y != result.second_anchor.y
      || result.first_anchor.z != result.second_anchor.z;
  result.full_phase_distinguishes_gap =
      result.separated_gap > tolerance
      && std::abs(result.contact_gap) <= tolerance
      && result.crossed_gap < -tolerance;
  result.valid = result.charts.valid
      && result.minimum_missing_charge == 0
      && result.same_site_occupancy
      && result.full_phase_distinguishes_gap
      && result.gap_residual <= tolerance
      && result.stable_chart_residual <= tolerance
      && result.contact_hop_margin > tolerance
      && result.crossed_hop_margin > tolerance
      && result.predicted_hop_delay_ticks > 0;
  return result;
}

}  // namespace ftd::eft
