#include "ftd/eft/hard_contact_corner_action.h"

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

double energy(const Vec3& momentum,
              double rest_energy,
              double c_speed) {
  return std::sqrt(rest_energy * rest_energy
      + c_speed * c_speed * momentum.mag2());
}

Vec3 velocity(const Vec3& momentum,
              double rest_energy,
              double c_speed) {
  return momentum * (c_speed * c_speed
      / energy(momentum, rest_energy, c_speed));
}

double lagrangian(const Vec3& carrier_velocity,
                  double rest_energy,
                  double c_speed) {
  const double beta_squared = carrier_velocity.mag2()
      / (c_speed * c_speed);
  return -rest_energy * std::sqrt(std::max(0.0, 1.0 - beta_squared));
}

Vec3 momentum_from_velocity(const Vec3& carrier_velocity,
                            double rest_energy,
                            double c_speed) {
  const double beta_squared = carrier_velocity.mag2()
      / (c_speed * c_speed);
  const double denominator = c_speed * c_speed
      * std::sqrt(std::max(0.0, 1.0 - beta_squared));
  return carrier_velocity * (rest_energy / denominator);
}

double momentum_for_speed(double speed,
                          double rest_energy,
                          double c_speed) {
  const double beta_squared = speed * speed / (c_speed * c_speed);
  return rest_energy * speed
      / (c_speed * c_speed * std::sqrt(1.0 - beta_squared));
}

Vec3 tangential(const Vec3& value, const Vec3& normal) {
  return value - normal * value.dot(normal);
}

double reference_difference(
    const HardContactCornerActionResult& result) {
  return std::max({
      max_difference(result.momentum_first_before,
          result.reference_collision.momentum_first_before),
      max_difference(result.momentum_second_before,
          result.reference_collision.momentum_second_before),
      max_difference(result.momentum_first_after,
          result.reference_collision.momentum_first_after),
      max_difference(result.momentum_second_after,
          result.reference_collision.momentum_second_after)});
}

}  // namespace

double selected_hard_contact_multiplier(double gap,
                                        double incoming_q_n,
                                        double tolerance) {
  if (!std::isfinite(gap) || !std::isfinite(incoming_q_n)
      || !std::isfinite(tolerance) || tolerance < 0.0) return NAN;
  if (gap > tolerance || incoming_q_n <= tolerance) return 0.0;
  if (gap < -tolerance) return NAN;
  return 2.0 * incoming_q_n;
}

HardContactCornerActionResult analyze_hard_contact_corner_action(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double rest_energy,
    double c_speed,
    double segment_distance,
    double tolerance) {
  HardContactCornerActionResult result;
  result.L = L;
  result.polarity = polarity;
  result.chart_direction = chart_direction;
  result.collision_position = collision_position;
  result.rest_energy = rest_energy;
  result.c_speed = c_speed;
  result.speed = speed;
  if (L < 3 || !finite(collision_position)
      || (chart_direction.x == 0 && chart_direction.y == 0
          && chart_direction.z == 0)
      || (polarity != -1 && polarity != +1)
      || !std::isfinite(speed) || speed <= 0.0
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(c_speed) || c_speed <= 0.0
      || speed >= c_speed || !std::isfinite(segment_distance)
      || segment_distance <= 0.0 || !std::isfinite(tolerance)
      || tolerance < 0.0) return result;

  result.reference_collision = analyze_constituent_relative_collision(
      L, collision_position, chart_direction, polarity, speed,
      rest_energy, c_speed, segment_distance, tolerance);
  if (!result.reference_collision.valid) return result;
  result.normal = result.reference_collision.chart_normal;

  const double p = momentum_for_speed(speed, rest_energy, c_speed);
  result.momentum_first_before = result.normal * p;
  result.momentum_second_before = result.momentum_first_before * -1.0;
  const Vec3 relative_before = (result.momentum_first_before
      - result.momentum_second_before) * 0.5;
  result.relative_normal_momentum = relative_before.dot(result.normal);
  result.contact_gap = 0.0;
  result.impulse_multiplier = selected_hard_contact_multiplier(
      result.contact_gap, result.relative_normal_momentum, tolerance);
  result.inactive_control_multiplier = selected_hard_contact_multiplier(
      segment_distance, result.relative_normal_momentum, tolerance);
  if (!std::isfinite(result.impulse_multiplier)
      || !std::isfinite(result.inactive_control_multiplier)) return result;

  result.momentum_first_after = result.momentum_first_before
      - result.normal * result.impulse_multiplier;
  result.momentum_second_after = result.momentum_second_before
      + result.normal * result.impulse_multiplier;
  result.velocity_first_before = velocity(
      result.momentum_first_before, rest_energy, c_speed);
  result.velocity_second_before = velocity(
      result.momentum_second_before, rest_energy, c_speed);
  result.velocity_first_after = velocity(
      result.momentum_first_after, rest_energy, c_speed);
  result.velocity_second_after = velocity(
      result.momentum_second_after, rest_energy, c_speed);
  result.incoming_gap_rate = (result.velocity_second_before
      - result.velocity_first_before).dot(result.normal);
  result.outgoing_gap_rate = (result.velocity_second_after
      - result.velocity_first_after).dot(result.normal);

  const Vec3 impulse_first = result.momentum_first_after
      - result.momentum_first_before;
  const Vec3 impulse_second = result.momentum_second_after
      - result.momentum_second_before;
  result.reference_collision_residual = reference_difference(result);
  result.multiplier_match_residual = std::abs(result.impulse_multiplier
      - result.reference_collision.impulse_multiplier);
  result.normal_impulse_residual = std::max(
      max_difference(impulse_first,
          result.normal * -result.impulse_multiplier),
      max_difference(impulse_second,
          result.normal * result.impulse_multiplier));
  result.tangential_corner_residual = std::max(
      max_abs(tangential(impulse_first, result.normal)),
      max_abs(tangential(impulse_second, result.normal)));
  result.common_corner_gradient_residual = max_abs(
      impulse_first + impulse_second);
  result.total_momentum_residual = max_difference(
      result.momentum_first_after + result.momentum_second_after,
      result.momentum_first_before + result.momentum_second_before);
  const double energy_before = energy(
      result.momentum_first_before, rest_energy, c_speed)
      + energy(result.momentum_second_before, rest_energy, c_speed);
  const double energy_after = energy(
      result.momentum_first_after, rest_energy, c_speed)
      + energy(result.momentum_second_after, rest_energy, c_speed);
  result.total_energy_residual = std::abs(energy_after - energy_before);
  result.collision_time_gradient_residual = result.total_energy_residual;
  const double action_density_before = lagrangian(
      result.velocity_first_before, rest_energy, c_speed)
      + lagrangian(result.velocity_second_before, rest_energy, c_speed);
  const double action_density_after = lagrangian(
      result.velocity_first_after, rest_energy, c_speed)
      + lagrangian(result.velocity_second_after, rest_energy, c_speed);
  result.action_density_residual = std::abs(
      action_density_after - action_density_before);

  result.legendre_residual = 0.0;
  const Vec3 momenta[4]{
      result.momentum_first_before, result.momentum_second_before,
      result.momentum_first_after, result.momentum_second_after};
  const Vec3 velocities[4]{
      result.velocity_first_before, result.velocity_second_before,
      result.velocity_first_after, result.velocity_second_after};
  for (int i = 0; i < 4; ++i) {
    result.legendre_residual = std::max({
        result.legendre_residual,
        max_difference(momentum_from_velocity(
            velocities[i], rest_energy, c_speed), momenta[i]),
        std::abs(momenta[i].dot(velocities[i])
            - lagrangian(velocities[i], rest_energy, c_speed)
            - energy(momenta[i], rest_energy, c_speed))});
  }

  result.branch_polynomial_residual = std::abs(
      result.impulse_multiplier * (result.impulse_multiplier
          - 2.0 * result.relative_normal_momentum));
  result.nontrivial_branch_residual = std::abs(
      result.impulse_multiplier
      - 2.0 * result.relative_normal_momentum);
  result.kkt_dual_residual = std::max(0.0,
      -result.impulse_multiplier);
  result.complementarity_residual = std::abs(
      result.impulse_multiplier * result.contact_gap);
  result.incoming_gate_residual = std::max(
      0.0, result.incoming_gap_rate);
  result.outgoing_gate_residual = std::max(
      0.0, -result.outgoing_gap_rate);

  result.face_balance = analyze_collision_momentum_face_balance(
      L, collision_position, chart_direction, polarity, speed,
      rest_energy, c_speed, segment_distance, tolerance);
  result.face_balance_residual = std::max({
      result.face_balance.individual_segment_residual,
      result.face_balance.constituent_impulse_residual,
      result.face_balance.aggregate_impulse_source_l1,
      result.face_balance.aggregate_local_balance_residual,
      result.face_balance.aggregate_global_momentum_residual,
      result.face_balance.energy_residual,
      result.face_balance.tensor_moment_residual});

  const Vec3 reverse_first_before = result.momentum_first_after * -1.0;
  const Vec3 reverse_second_before = result.momentum_second_after * -1.0;
  const Vec3 reverse_relative = (reverse_first_before
      - reverse_second_before) * 0.5;
  const double reverse_q_n = reverse_relative.dot(result.normal);
  const double reverse_multiplier = selected_hard_contact_multiplier(
      0.0, reverse_q_n, tolerance);
  const Vec3 reverse_first_after = reverse_first_before
      - result.normal * reverse_multiplier;
  const Vec3 reverse_second_after = reverse_second_before
      + result.normal * reverse_multiplier;
  result.reversal_residual = std::max(
      max_difference(reverse_first_after,
          result.momentum_first_before * -1.0),
      max_difference(reverse_second_after,
          result.momentum_second_before * -1.0));
  result.reversal_multiplier_residual = std::abs(
      reverse_multiplier - result.impulse_multiplier);

  result.valid = result.reference_collision.valid
      && result.face_balance.valid
      && result.relative_normal_momentum > tolerance
      && result.impulse_multiplier > tolerance
      && result.inactive_control_multiplier == 0.0
      && result.incoming_gap_rate < -tolerance
      && result.outgoing_gap_rate > tolerance
      && result.reference_collision_residual <= tolerance
      && result.multiplier_match_residual <= tolerance
      && result.normal_impulse_residual <= tolerance
      && result.tangential_corner_residual <= tolerance
      && result.common_corner_gradient_residual <= tolerance
      && result.collision_time_gradient_residual <= tolerance
      && result.total_momentum_residual <= tolerance
      && result.total_energy_residual <= tolerance
      && result.action_density_residual <= tolerance
      && result.legendre_residual <= tolerance
      && result.branch_polynomial_residual <= tolerance
      && result.nontrivial_branch_residual <= tolerance
      && result.kkt_dual_residual <= tolerance
      && result.complementarity_residual <= tolerance
      && result.incoming_gate_residual <= tolerance
      && result.outgoing_gate_residual <= tolerance
      && result.face_balance_residual <= tolerance
      && result.reversal_residual <= tolerance
      && result.reversal_multiplier_residual <= tolerance;
  return result;
}

}  // namespace ftd::eft
