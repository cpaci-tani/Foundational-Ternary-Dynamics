#include "ftd/eft/boundary_collision_resolution.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

Vec3 normalized(const Vec3& value) {
  const double magnitude = value.mag();
  return magnitude > 0.0 ? value * (1.0 / magnitude) : Vec3{};
}

double maximum_difference(const std::vector<double>& lhs,
                          const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result = std::max(result, std::abs(lhs[i] - rhs[i]));
  }
  return result;
}

double reversal_residual(const PiecewiseCurrentSignature& forward,
                         const PiecewiseCurrentSignature& reverse) {
  if (!forward.valid || !reverse.valid) return INFINITY;
  double result = std::max(
      maximum_difference(forward.rho_before, reverse.rho_after),
      maximum_difference(forward.rho_after, reverse.rho_before));
  if (forward.current_x.size() != reverse.current_x.size()) return INFINITY;
  for (std::size_t i = 0; i < forward.current_x.size(); ++i) {
    result = std::max({result,
        std::abs(forward.current_x[i] + reverse.current_x[i]),
        std::abs(forward.current_y[i] + reverse.current_y[i]),
        std::abs(forward.current_z[i] + reverse.current_z[i])});
  }
  return result;
}

double pair_energy(double speed, double rest_energy, double c_speed) {
  const double beta_squared = speed * speed / (c_speed * c_speed);
  return 2.0 * rest_energy / std::sqrt(1.0 - beta_squared);
}

}  // namespace

BoundaryCollisionResolution analyze_boundary_collision_resolution(
    const Vec3& center,
    const Vec3& direction,
    double half_separation,
    double speed,
    double dt,
    int charge,
    double tolerance) {
  BoundaryCollisionResolution result;
  result.center = center;
  result.half_separation = half_separation;
  result.speed = speed;
  result.dt = dt;
  if (!finite(center) || !finite(direction) || direction.mag() == 0.0
      || !std::isfinite(half_separation) || half_separation <= 0.0
      || !std::isfinite(speed) || speed <= 0.0
      || !std::isfinite(dt) || dt <= 0.0
      || (charge != -1 && charge != +1)
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  result.unit_direction = normalized(direction);
  result.collision_time = half_separation / speed;
  result.collision_time_residual = std::abs(result.collision_time - dt);
  result.endpoint_capacity = analyze_ternary_same_sign_capacity(2, charge);
  result.minimum_charge_alphabet_symbols = 5;
  result.minimum_auxiliary_occupancy_bits = 1;
  result.valid = result.endpoint_capacity.valid
      && result.collision_time_residual <= tolerance;
  return result;
}

SameTickSeparatedOutputAttempt analyze_same_tick_separated_output(
    double half_separation,
    double incoming_speed,
    double dt,
    double output_distance,
    double output_speed,
    double c_speed,
    double tolerance) {
  SameTickSeparatedOutputAttempt result;
  result.output_distance = output_distance;
  result.output_speed = output_speed;
  if (!std::isfinite(half_separation) || half_separation <= 0.0
      || !std::isfinite(incoming_speed) || incoming_speed <= 0.0
      || !std::isfinite(dt) || dt <= 0.0
      || !std::isfinite(output_distance) || output_distance <= 0.0
      || !std::isfinite(output_speed) || output_speed <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0
      || output_speed > c_speed + tolerance
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  result.collision_time = half_separation / incoming_speed;
  result.required_total_time = result.collision_time
      + output_distance / output_speed;
  result.temporal_causal_defect = std::max(
      0.0, result.required_total_time - dt);
  result.same_tick_causal = result.temporal_causal_defect <= tolerance;
  result.valid = std::abs(result.collision_time - dt) <= tolerance;
  return result;
}

PrecontactExclusionResult analyze_precontact_exclusion(
    int L,
    const Vec3& center,
    const Vec3& direction,
    double half_separation,
    double speed,
    double dt,
    double exclusion_radius,
    int charge,
    double rest_energy,
    double c_speed,
    double tolerance) {
  PrecontactExclusionResult result;
  result.L = L;
  result.charge = charge;
  result.exclusion_radius = exclusion_radius;
  if (L < 3 || !finite(center) || !finite(direction)
      || direction.mag() == 0.0
      || !std::isfinite(half_separation) || half_separation <= 0.0
      || !std::isfinite(speed) || speed <= 0.0
      || !std::isfinite(dt) || dt <= 0.0
      || !std::isfinite(exclusion_radius) || exclusion_radius <= 0.0
      || exclusion_radius >= half_separation
      || (charge != -1 && charge != +1)
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0
      || speed > c_speed + tolerance
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  const Vec3 axis = normalized(direction);
  const Vec3 left_start = center - axis * half_separation;
  const Vec3 right_start = center + axis * half_separation;
  const Vec3 left_contact = center - axis * exclusion_radius;
  const Vec3 right_contact = center + axis * exclusion_radius;
  result.left_endpoint = center - axis * (2.0 * exclusion_radius);
  result.right_endpoint = center + axis * (2.0 * exclusion_radius);
  result.endpoint_separation = (
      result.right_endpoint - result.left_endpoint).mag();
  result.contact_time = (half_separation - exclusion_radius) / speed;
  result.remaining_time = dt - result.contact_time;

  const std::vector<PiecewiseWorldline> paths{{
      charge, {left_start, left_contact, result.left_endpoint}},
      {charge, {right_start, right_contact, result.right_endpoint}}};
  auto reverse_paths = paths;
  for (auto& path : reverse_paths) {
    std::reverse(path.vertices.begin(), path.vertices.end());
  }
  result.current = make_piecewise_current_signature(L, paths);
  const auto reverse_current = make_piecewise_current_signature(
      L, reverse_paths);
  result.reversal_residual = reversal_residual(
      result.current, reverse_current);

  const double expected_remaining = exclusion_radius / speed;
  const double incoming_energy = pair_energy(speed, rest_energy, c_speed);
  const double outgoing_energy = pair_energy(speed, rest_energy, c_speed);
  result.energy_residual = std::abs(outgoing_energy - incoming_energy);
  result.momentum_residual = 0.0;
  result.charge_residual = 0.0;
  const double travelled = (half_separation - exclusion_radius)
      + exclusion_radius;
  result.causal_residual = std::max({
      0.0, speed - c_speed,
      std::abs(result.remaining_time - expected_remaining),
      std::abs(travelled - speed * dt)});
  result.continuity_residual = result.current.continuity_residual;
  result.valid = result.current.valid
      && result.endpoint_separation > 0.0
      && result.energy_residual <= tolerance
      && result.momentum_residual <= tolerance
      && result.charge_residual <= tolerance
      && result.causal_residual <= tolerance
      && result.continuity_residual <= tolerance
      && result.reversal_residual <= tolerance;
  return result;
}

double collision_signature_difference(
    const PiecewiseCurrentSignature& lhs,
    const PiecewiseCurrentSignature& rhs) {
  if (!lhs.valid || !rhs.valid) return INFINITY;
  return std::max({
      maximum_difference(lhs.rho_before, rhs.rho_before),
      maximum_difference(lhs.rho_after, rhs.rho_after),
      maximum_difference(lhs.current_x, rhs.current_x),
      maximum_difference(lhs.current_y, rhs.current_y),
      maximum_difference(lhs.current_z, rhs.current_z)});
}

CollisionTimingShiftResult analyze_collision_timing_shift(
    double half_separation,
    double dt,
    double delta,
    double rest_energy,
    double c_speed) {
  CollisionTimingShiftResult result;
  result.delta = delta;
  if (!std::isfinite(half_separation) || half_separation <= 0.0
      || !std::isfinite(dt) || dt <= 0.0
      || !std::isfinite(delta) || delta <= 0.0 || delta >= dt
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0) return result;
  result.baseline_speed = half_separation / dt;
  result.early_speed = half_separation / (dt - delta);
  result.late_speed = half_separation / (dt + delta);
  if (result.baseline_speed >= c_speed || result.late_speed >= c_speed) {
    return result;
  }
  result.baseline_pair_energy = pair_energy(
      result.baseline_speed, rest_energy, c_speed);
  if (result.early_speed < c_speed) {
    result.early_pair_energy = pair_energy(
        result.early_speed, rest_energy, c_speed);
  } else {
    result.early_pair_energy = INFINITY;
  }
  result.late_pair_energy = pair_energy(
      result.late_speed, rest_energy, c_speed);
  result.early_energy_shift = result.early_pair_energy
      - result.baseline_pair_energy;
  result.late_energy_shift = result.late_pair_energy
      - result.baseline_pair_energy;
  result.minimum_absolute_energy_shift = std::min(
      std::abs(result.early_energy_shift),
      std::abs(result.late_energy_shift));
  result.early_causal_residual = std::max(
      0.0, result.early_speed - c_speed);
  result.valid = std::isfinite(result.early_pair_energy)
      && result.early_causal_residual == 0.0
      && result.minimum_absolute_energy_shift > 0.0;
  return result;
}

}  // namespace ftd::eft
