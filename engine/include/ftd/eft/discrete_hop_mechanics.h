#pragma once
/**
 * @file discrete_hop_mechanics.h
 * @brief Selected reversible longitudinal map for finite-site hop work.
 *
 * This helper is analysis-only.  Scalar work does not uniquely determine a
 * force or local field recoil; the longitudinal/preserved-transverse branch
 * implemented here is an explicit selection used to expose that boundary.
 */

#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {

inline double flat_particle_energy_from_momentum(const Vec3& momentum,
                                                 double rest_energy,
                                                 double speed_limit) {
  if (!(rest_energy > 0.0) || !(speed_limit > 0.0)) return 0.0;
  return std::sqrt(rest_energy * rest_energy
      + momentum.mag2() / (speed_limit * speed_limit));
}

struct SelectedLongitudinalHopUpdate {
  Vec3 momentum_before{};
  Vec3 momentum_after{};
  Vec3 required_field_recoil{};
  double energy_before = 0.0;
  double energy_after = 0.0;
  double requested_work = 0.0;
  double work_residual = 0.0;
  bool valid = false;
};

inline SelectedLongitudinalHopUpdate selected_longitudinal_hop_update(
    const Vec3& momentum,
    const Vec3& displacement,
    double work,
    double rest_energy,
    double speed_limit) {
  SelectedLongitudinalHopUpdate result;
  result.momentum_before = momentum;
  result.requested_work = work;
  if (!(displacement.mag2() > 0.0) || !(rest_energy > 0.0)
      || !(speed_limit > 0.0))
    return result;

  const Vec3 direction = displacement * (1.0 / displacement.mag());
  const double parallel_before = momentum.dot(direction);
  const Vec3 perpendicular = momentum - direction * parallel_before;
  result.energy_before = flat_particle_energy_from_momentum(
      momentum, rest_energy, speed_limit);
  const double target_energy = result.energy_before + work;
  if (target_energy < rest_energy) return result;

  double parallel_sq = speed_limit * speed_limit
      * (target_energy * target_energy - rest_energy * rest_energy)
      - perpendicular.mag2();
  const double scale = std::max(1.0,
      speed_limit * speed_limit * target_energy * target_energy);
  if (parallel_sq < -1e-14 * scale) return result;
  parallel_sq = std::max(0.0, parallel_sq);
  const double branch = parallel_before < 0.0 ? -1.0 : 1.0;
  const double parallel_after = branch * std::sqrt(parallel_sq);
  result.momentum_after = perpendicular + direction * parallel_after;
  result.required_field_recoil = momentum - result.momentum_after;
  result.energy_after = flat_particle_energy_from_momentum(
      result.momentum_after, rest_energy, speed_limit);
  result.work_residual =
      result.energy_after - result.energy_before - work;
  result.valid = std::isfinite(result.energy_before)
      && std::isfinite(result.energy_after)
      && std::isfinite(result.work_residual);
  return result;
}

}  // namespace ftd::eft
