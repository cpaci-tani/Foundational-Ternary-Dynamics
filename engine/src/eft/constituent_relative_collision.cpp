#include "ftd/eft/constituent_relative_collision.h"

#include "ftd/eft/discrete_legendre_worldline.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double max_difference(const Vec3& lhs, const Vec3& rhs) {
  return max_abs(lhs - rhs);
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

double energy(const Vec3& momentum,
              double rest_energy,
              double c_speed) {
  return std::sqrt(rest_energy * rest_energy
      + c_speed * c_speed * momentum.mag2());
}

double momentum_for_speed(double speed,
                          double rest_energy,
                          double c_speed) {
  const double beta_squared = speed * speed / (c_speed * c_speed);
  return rest_energy * speed
      / (c_speed * c_speed * std::sqrt(1.0 - beta_squared));
}

Vec3 reflect_relative(const Vec3& momentum_first,
                      const Vec3& momentum_second,
                      const Vec3& normal,
                      bool first) {
  const Vec3 total = momentum_first + momentum_second;
  const Vec3 relative = (momentum_first - momentum_second) * 0.5;
  const Vec3 reflected = relative
      - normal * (2.0 * relative.dot(normal));
  return total * 0.5 + reflected * (first ? 1.0 : -1.0);
}

Vec3 tangential(const Vec3& value, const Vec3& normal) {
  return value - normal * value.dot(normal);
}

int hamming(Coord direction) {
  return static_cast<int>(direction.x != 0)
      + static_cast<int>(direction.y != 0)
      + static_cast<int>(direction.z != 0);
}

}  // namespace

ConstituentRelativeCollisionResult analyze_constituent_relative_collision(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double rest_energy,
    double c_speed,
    double observation_distance,
    double tolerance) {
  ConstituentRelativeCollisionResult result;
  result.L = L;
  result.polarity = polarity;
  result.chart_direction = chart_direction;
  result.collision_position = collision_position;
  result.speed = speed;
  result.face_direction = hamming(chart_direction) == 1;
  if (L < 3 || !finite(collision_position)
      || hamming(chart_direction) == 0
      || (polarity != -1 && polarity != +1)
      || !std::isfinite(speed) || speed <= 0.0
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0
      || speed >= c_speed
      || !std::isfinite(observation_distance)
      || observation_distance <= 0.0
      || !std::isfinite(tolerance) || tolerance < 0.0) {
    return result;
  }

  result.charts = analyze_boundary_chart_collision(
      L, collision_position, chart_direction, polarity,
      observation_distance, tolerance);
  if (!result.charts.valid) return result;
  result.chart_normal = result.charts.unit_direction;
  result.chart_position_residual =
      result.charts.collision_position_residual;

  result.momentum_magnitude = momentum_for_speed(
      speed, rest_energy, c_speed);
  result.momentum_first_before = result.chart_normal
      * result.momentum_magnitude;
  result.momentum_second_before = result.momentum_first_before * -1.0;
  const Vec3 total_before = result.momentum_first_before
      + result.momentum_second_before;
  const Vec3 relative_before = (result.momentum_first_before
      - result.momentum_second_before) * 0.5;
  result.incoming_normal_momentum = relative_before.dot(
      result.chart_normal);
  result.normal_com_momentum_residual = std::abs(
      total_before.dot(result.chart_normal));
  result.impulse_multiplier = 2.0
      * result.incoming_normal_momentum;

  result.momentum_first_after = reflect_relative(
      result.momentum_first_before, result.momentum_second_before,
      result.chart_normal, true);
  result.momentum_second_after = reflect_relative(
      result.momentum_first_before, result.momentum_second_before,
      result.chart_normal, false);
  result.impulse_first = result.momentum_first_after
      - result.momentum_first_before;
  result.impulse_second = result.momentum_second_after
      - result.momentum_second_before;
  result.selected_central_contact = true;

  const Vec3 total_after = result.momentum_first_after
      + result.momentum_second_after;
  const Vec3 relative_after = (result.momentum_first_after
      - result.momentum_second_after) * 0.5;
  result.outgoing_normal_momentum = relative_after.dot(
      result.chart_normal);
  result.impulse_sum_residual = max_abs(
      result.impulse_first + result.impulse_second);
  result.total_momentum_residual = max_difference(
      total_after, total_before);
  result.matter_energy_residual = std::abs(
      energy(result.momentum_first_after, rest_energy, c_speed)
          + energy(result.momentum_second_after, rest_energy, c_speed)
      - energy(result.momentum_first_before, rest_energy, c_speed)
          - energy(result.momentum_second_before, rest_energy, c_speed));
  result.central_impulse_residual = std::max(
      max_abs(tangential(result.impulse_first, result.chart_normal)),
      max_abs(tangential(result.impulse_second, result.chart_normal)));
  result.impulse_solution_residual = std::max(
      max_abs(result.impulse_first
          + result.chart_normal * result.impulse_multiplier),
      max_abs(result.impulse_second
          - result.chart_normal * result.impulse_multiplier));
  result.tangential_relative_residual = max_difference(
      tangential(relative_after, result.chart_normal),
      tangential(relative_before, result.chart_normal));
  result.outgoing_condition_residual = std::max(
      0.0, result.outgoing_normal_momentum);

  const Vec3 involution_first = reflect_relative(
      result.momentum_first_after, result.momentum_second_after,
      result.chart_normal, true);
  const Vec3 involution_second = reflect_relative(
      result.momentum_first_after, result.momentum_second_after,
      result.chart_normal, false);
  result.involution_residual = std::max(
      max_difference(involution_first, result.momentum_first_before),
      max_difference(involution_second, result.momentum_second_before));
  const Vec3 reverse_first = reflect_relative(
      result.momentum_first_after * -1.0,
      result.momentum_second_after * -1.0,
      result.chart_normal, true);
  const Vec3 reverse_second = reflect_relative(
      result.momentum_first_after * -1.0,
      result.momentum_second_after * -1.0,
      result.chart_normal, false);
  result.time_reversal_residual = std::max(
      max_difference(reverse_first,
                     result.momentum_first_before * -1.0),
      max_difference(reverse_second,
                     result.momentum_second_before * -1.0));

  const Vec3 first_displacement = free_displacement_from_momentum(
      result.momentum_first_after, rest_energy, c_speed, c_speed);
  const Vec3 second_displacement = free_displacement_from_momentum(
      result.momentum_second_after, rest_energy, c_speed, c_speed);
  result.causal_residual = std::max({
      0.0, first_displacement.mag() - c_speed,
      second_displacement.mag() - c_speed});

  const Vec3 first_endpoint = collision_position
      - result.chart_normal * observation_distance;
  const Vec3 second_endpoint = collision_position
      + result.chart_normal * observation_distance;
  const std::vector<PiecewiseWorldline> static_paths{{
      {polarity, {collision_position, collision_position}},
      {polarity, {collision_position, collision_position}}}};
  const std::vector<PiecewiseWorldline> separating_paths{{
      {polarity, {collision_position, first_endpoint}},
      {polarity, {collision_position, second_endpoint}}}};
  result.aggregate_static = make_piecewise_current_signature(
      L, static_paths);
  result.aggregate_separating = make_piecewise_current_signature(
      L, separating_paths);
  result.first_separating = make_piecewise_current_signature(
      L, {{polarity, {collision_position, first_endpoint}}});
  result.second_separating = make_piecewise_current_signature(
      L, {{polarity, {collision_position, second_endpoint}}});
  result.aggregate_static_separating_residual = signature_difference(
      result.aggregate_static, result.aggregate_separating);
  result.aggregate_current_l1 = result.aggregate_separating.current_l1;
  result.constituent_current_l1 = result.first_separating.current_l1
      + result.second_separating.current_l1;
  result.relative_mode_projection_gap = result.constituent_current_l1
      - result.aggregate_current_l1;
  result.matter_kinetic_energy_gap =
      energy(result.momentum_first_before, rest_energy, c_speed)
      + energy(result.momentum_second_before, rest_energy, c_speed)
      - 2.0 * rest_energy;
  result.continuity_residual = std::max({
      result.aggregate_static.continuity_residual,
      result.aggregate_separating.continuity_residual,
      result.first_separating.continuity_residual,
      result.second_separating.continuity_residual});
  result.aggregate_face_kernel = result.face_direction
      && result.aggregate_static_separating_residual <= tolerance
      && result.aggregate_current_l1 <= tolerance
      && result.constituent_current_l1 > tolerance
      && result.matter_kinetic_energy_gap > 1e-6;

  result.valid = result.charts.valid
      && result.selected_central_contact
      && result.chart_position_residual <= tolerance
      && result.incoming_normal_momentum > tolerance
      && result.outgoing_normal_momentum < -tolerance
      && result.normal_com_momentum_residual <= tolerance
      && result.impulse_sum_residual <= tolerance
      && result.total_momentum_residual <= tolerance
      && result.matter_energy_residual <= tolerance
      && result.central_impulse_residual <= tolerance
      && result.impulse_solution_residual <= tolerance
      && result.tangential_relative_residual <= tolerance
      && result.outgoing_condition_residual <= tolerance
      && result.involution_residual <= tolerance
      && result.time_reversal_residual <= tolerance
      && result.causal_residual <= tolerance
      && result.aggregate_static.valid
      && result.aggregate_separating.valid
      && result.first_separating.valid
      && result.second_separating.valid
      && result.continuity_residual <= tolerance;
  return result;
}

}  // namespace ftd::eft
