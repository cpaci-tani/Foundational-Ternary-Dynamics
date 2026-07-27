#include "ftd/eft/production_same_sign_bounce.h"

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

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result = std::max(result, std::abs(lhs[i] - rhs[i]));
  }
  return result;
}

Vec3 coordinate(Coord value) {
  return {static_cast<double>(value.x),
          static_cast<double>(value.y),
          static_cast<double>(value.z)};
}

double attribute_residual(const Voxel& lhs, const Voxel& rhs) {
  return std::max({
      std::abs(static_cast<double>(lhs.state - rhs.state)),
      max_difference(lhs.velocity, rhs.velocity),
      max_difference(lhs.remainder, rhs.remainder),
      std::abs(static_cast<double>(lhs.particle_id - rhs.particle_id)),
      std::abs(static_cast<double>(lhs.pair_id - rhs.pair_id)),
      std::abs(static_cast<double>(lhs.spin - rhs.spin)),
      std::abs(static_cast<double>(lhs.color - rhs.color)),
      std::abs(static_cast<double>(lhs.flavor - rhs.flavor))});
}

double field_residual(const Voxel& before, const Voxel& after) {
  return std::max({
      max_difference(before.flux, after.flux),
      max_difference(before.wave_vel, after.wave_vel),
      max_difference(before.flux_L, after.flux_L),
      max_difference(before.flux_R, after.flux_R),
      max_difference(before.wave_vel_L, after.wave_vel_L),
      max_difference(before.wave_vel_R, after.wave_vel_R),
      max_difference(before.flux_strong, after.flux_strong),
      max_difference(before.wave_vel_strong, after.wave_vel_strong),
      max_difference(before.flux_weak, after.flux_weak),
      max_difference(before.wave_vel_weak, after.wave_vel_weak)});
}

Vec3 reflected_velocity(const Vec3& velocity, Coord hop) {
  Vec3 result = velocity;
  if (hop.x != 0) result.x *= -1.0;
  if (hop.y != 0) result.y *= -1.0;
  if (hop.z != 0) result.z *= -1.0;
  return result;
}

Vec3 specular_remainder(const Vec3& proposed, Coord hop) {
  Vec3 result = proposed;
  if (hop.x > 0) result.x = 2.0 - proposed.x;
  if (hop.x < 0) result.x = -2.0 - proposed.x;
  if (hop.y > 0) result.y = 2.0 - proposed.y;
  if (hop.y < 0) result.y = -2.0 - proposed.y;
  if (hop.z > 0) result.z = 2.0 - proposed.z;
  if (hop.z < 0) result.z = -2.0 - proposed.z;
  return result;
}

}  // namespace

ProductionSameSignBounceResult analyze_production_same_sign_bounce(
    int L,
    Coord source_anchor,
    Coord hop,
    const Voxel& source_before,
    const Voxel& target_before,
    const Voxel& source_after,
    const Voxel& target_after,
    const Voxel& source_after_second_tick,
    const Voxel& target_after_second_tick,
    int journal_event_count,
    double dt,
    double tolerance) {
  ProductionSameSignBounceResult result;
  result.L = L;
  result.source_anchor = source_anchor;
  result.hop = hop;
  result.target_anchor = {
      source_anchor.x + hop.x,
      source_anchor.y + hop.y,
      source_anchor.z + hop.z};
  result.charge = source_before.state;
  result.dt = dt;
  result.journal_event_count = journal_event_count;
  const bool valid_hop = hop.x >= -1 && hop.x <= 1
      && hop.y >= -1 && hop.y <= 1
      && hop.z >= -1 && hop.z <= 1
      && (hop.x != 0 || hop.y != 0 || hop.z != 0);
  if (L < 3 || !valid_hop
      || (source_before.state != -1 && source_before.state != +1)
      || target_before.state != source_before.state
      || !finite(source_before.velocity)
      || !finite(source_before.remainder)
      || !std::isfinite(dt) || dt <= 0.0
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;

  const Vec3 anchor = coordinate(source_anchor);
  const Vec3 target = coordinate(result.target_anchor);
  result.effective_position_before = anchor + source_before.remainder;
  result.effective_position_after = anchor + source_after.remainder;
  const Vec3 proposed = source_before.remainder
      + source_before.velocity * dt;
  result.specular_remainder_after = specular_remainder(proposed, hop);
  result.specular_position_after = anchor
      + result.specular_remainder_after;

  result.source_velocity_reflection_residual = max_difference(
      source_after.velocity,
      reflected_velocity(source_before.velocity, hop));
  result.source_remainder_reset_residual = max_abs(source_after.remainder);
  result.target_unchanged_residual = attribute_residual(
      target_before, target_after);
  result.manifestation_residual = std::max(
      std::abs(static_cast<double>(source_after.state
                                   - source_before.state)),
      std::abs(static_cast<double>(target_after.state
                                   - target_before.state)));
  result.specular_remainder_residual = max_difference(
      source_after.remainder, result.specular_remainder_after);
  result.production_effective_causal_residual = std::max(
      0.0, (result.effective_position_after
            - result.effective_position_before).mag() / dt - C_SPEED);
  const double specular_arc = (target - result.effective_position_before).mag()
      + (result.specular_position_after - target).mag();
  result.specular_arc_causal_residual = std::max(
      0.0, specular_arc / dt - C_SPEED);

  const Vec3 p_source_before = production_flat_momentum(
      source_before.velocity);
  const Vec3 p_target_before = production_flat_momentum(
      target_before.velocity);
  const Vec3 p_source_after = production_flat_momentum(
      source_after.velocity);
  const Vec3 p_target_after = production_flat_momentum(
      target_after.velocity);
  const double energy_before = production_flat_energy_from_momentum(
      p_source_before) + production_flat_energy_from_momentum(p_target_before);
  const double energy_after = production_flat_energy_from_momentum(
      p_source_after) + production_flat_energy_from_momentum(p_target_after);
  result.pair_energy_residual = std::abs(energy_after - energy_before);
  result.pair_momentum_defect = (
      p_source_after + p_target_after
      - p_source_before - p_target_before).mag();
  result.field_state_change_residual = std::max(
      field_residual(source_before, source_after),
      field_residual(target_before, target_after));

  const std::vector<PiecewiseWorldline> production_paths{{
      result.charge,
      {result.effective_position_before, result.effective_position_after}},
      {result.charge, {target, target}}};
  const std::vector<PiecewiseWorldline> specular_paths{{
      result.charge,
      {result.effective_position_before, target,
       result.specular_position_after}},
      {result.charge, {target, target}}};
  result.production_endpoint_current = make_piecewise_current_signature(
      L, production_paths);
  result.specular_bounce_current = make_piecewise_current_signature(
      L, specular_paths);
  result.exact_current_difference = collision_signature_difference(
      result.production_endpoint_current, result.specular_bounce_current);
  result.exact_current_continuity_residual = std::max(
      result.production_endpoint_current.continuity_residual,
      result.specular_bounce_current.continuity_residual);
  result.missing_journal_current_residual = max_difference(
      result.production_endpoint_current.rho_before,
      result.production_endpoint_current.rho_after);

  result.inverse_phase_space_residual = std::max({
      max_difference(source_after_second_tick.velocity,
                     source_before.velocity),
      max_difference(source_after_second_tick.remainder,
                     source_before.remainder),
      attribute_residual(target_after_second_tick, target_before)});
  result.valid = result.production_endpoint_current.valid
      && result.specular_bounce_current.valid
      && std::isfinite(result.pair_energy_residual)
      && std::isfinite(result.pair_momentum_defect)
      && std::isfinite(result.exact_current_difference)
      && std::isfinite(result.inverse_phase_space_residual);
  return result;
}

}  // namespace ftd::eft
