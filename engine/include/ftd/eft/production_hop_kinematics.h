#pragma once
/**
 * @file production_hop_kinematics.h
 * @brief Analysis-only momentum form of the production flat kinematics.
 *
 * Production uses E = gamma E_REST, p = gamma M_INERTIAL v and
 * E_REST = M_INERTIAL C_SPEED^2.  Therefore
 * E(p)^2 = E_REST^2 + C_SPEED^2 |p|^2.
 */

#include "ftd/causal_kinematics.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {

inline Vec3 production_flat_momentum(const Vec3& velocity) {
  const double gamma = flat_gamma(velocity.mag2());
  if (!(gamma < CAUSAL_SENTINEL)) return {};
  return velocity * (gamma * M_INERTIAL);
}

inline double production_flat_energy_from_momentum(const Vec3& momentum) {
  return std::sqrt(E_REST * E_REST
      + C_SPEED * C_SPEED * momentum.mag2());
}

inline Vec3 production_flat_velocity_from_momentum(const Vec3& momentum) {
  const double energy = production_flat_energy_from_momentum(momentum);
  if (!(energy > 0.0)) return {};
  return momentum * (C_SPEED * C_SPEED / energy);
}

struct SelectedProductionHopUpdate {
  Vec3 momentum_before{};
  Vec3 momentum_after{};
  Vec3 required_field_recoil{};
  double energy_before = 0.0;
  double energy_after = 0.0;
  double requested_work = 0.0;
  double work_residual = 0.0;
  bool valid = false;
};

inline SelectedProductionHopUpdate selected_production_hop_update(
    const Vec3& momentum, const Vec3& displacement, double work) {
  SelectedProductionHopUpdate result;
  result.momentum_before = momentum;
  result.requested_work = work;
  if (!(displacement.mag2() > 0.0)) return result;

  const Vec3 direction = displacement * (1.0 / displacement.mag());
  const double parallel_before = momentum.dot(direction);
  const Vec3 perpendicular = momentum - direction * parallel_before;
  result.energy_before = production_flat_energy_from_momentum(momentum);
  const double target_energy = result.energy_before + work;
  if (target_energy < E_REST) return result;

  double parallel_sq =
      (target_energy * target_energy - E_REST * E_REST)
          / (C_SPEED * C_SPEED)
      - perpendicular.mag2();
  const double scale = std::max(1.0,
      target_energy * target_energy / (C_SPEED * C_SPEED));
  if (parallel_sq < -1e-14 * scale) return result;
  parallel_sq = std::max(0.0, parallel_sq);
  const double branch = parallel_before < 0.0 ? -1.0 : 1.0;
  result.momentum_after = perpendicular
      + direction * (branch * std::sqrt(parallel_sq));
  result.required_field_recoil = momentum - result.momentum_after;
  result.energy_after = production_flat_energy_from_momentum(
      result.momentum_after);
  result.work_residual =
      result.energy_after - result.energy_before - work;
  result.valid = std::isfinite(result.energy_before)
      && std::isfinite(result.energy_after)
      && std::isfinite(result.work_residual);
  return result;
}

}  // namespace ftd::eft
