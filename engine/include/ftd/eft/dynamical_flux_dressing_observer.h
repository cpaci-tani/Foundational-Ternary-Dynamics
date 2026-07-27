#pragma once
/**
 * @file dynamical_flux_dressing_observer.h
 * @brief Read-only source-centred flux morphology for FTD-0476.
 *
 * "Dressing" is a classification of the existing J/W history.  This observer
 * introduces no aura field, source, force, projection, or update rule.
 */

#include "ftd/eft/native_energy_contract.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace ftd::eft {

struct DynamicalDressingObservation {
  bool valid = false;
  double activity = 0.0;
  double field_norm2 = 0.0;
  double wave_norm2 = 0.0;
  double mean_radius = 0.0;
  double near_activity = 0.0;
  double near_fraction = 0.0;
  double leading_activity = 0.0;
  double trailing_activity = 0.0;
  double transverse_activity = 0.0;
  double leading_fraction = 0.0;
  double trailing_fraction = 0.0;
  double transverse_fraction = 0.0;
  double radial_alignment = 0.0;
  double signed_source_divergence = 0.0;
  int manifested_count = 0;
  int max_support_radius = -1;
  long double exact_tick_energy = 0.0L;
};

namespace dynamical_dressing_detail {

inline double periodic_delta(double value, double centre, int length) {
  double delta = value - centre;
  while (delta > 0.5 * length) delta -= length;
  while (delta < -0.5 * length) delta += length;
  return delta;
}

inline double activity_density(const RenderBridge& bridge, int index) {
  const auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
  return field_kinetic_term(voxel.wave_vel)
      - field_gradient_term(voxel.flux,
                            bridge.lattice().neighbors_6(index),
                            bridge.lattice().neighbors_12(index),
                            bridge.voxels());
}

inline void hash_bytes(std::uint64_t& hash, const void* data,
                       std::size_t size) {
  constexpr std::uint64_t prime = 1099511628211ull;
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (std::size_t i = 0; i < size; ++i) {
    hash ^= bytes[i];
    hash *= prime;
  }
}

}  // namespace dynamical_dressing_detail

inline DynamicalDressingObservation observe_dynamical_flux_dressing(
    const RenderBridge& bridge, int source_index, int polarity,
    double near_radius = 4.0, double alignment_radius = 6.0,
    int motion_sign = +1, double support_threshold = 1e-13) {
  DynamicalDressingObservation out;
  if (source_index < 0
      || source_index >= static_cast<int>(bridge.voxels().size())) return out;

  const int length = bridge.lattice().size();
  const auto source = bridge.lattice().coord(source_index);
  double radial_numerator = 0.0;
  double radial_denominator = 0.0;
  double radius_weight = 0.0;
  const double directed_sign = motion_sign >= 0 ? 1.0 : -1.0;

  for (int index = 0;
       index < static_cast<int>(bridge.voxels().size()); ++index) {
    const auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    if (voxel.state != 0) ++out.manifested_count;
    out.field_norm2 += voxel.flux.mag2();
    out.wave_norm2 += voxel.wave_vel.mag2();

    const auto coordinate = bridge.lattice().coord(index);
    const double dx = dynamical_dressing_detail::periodic_delta(
        coordinate.x, source.x, length);
    const double dy = dynamical_dressing_detail::periodic_delta(
        coordinate.y, source.y, length);
    const double dz = dynamical_dressing_detail::periodic_delta(
        coordinate.z, source.z, length);
    const double radius = std::sqrt(dx * dx + dy * dy + dz * dz);
    const int support_radius = static_cast<int>(std::max(
        {std::abs(dx), std::abs(dy), std::abs(dz)}));
    if (voxel.flux.mag2() + voxel.wave_vel.mag2()
        > support_threshold * support_threshold)
      out.max_support_radius = std::max(out.max_support_radius,
                                        support_radius);

    const double activity =
        dynamical_dressing_detail::activity_density(bridge, index);
    out.activity += activity;
    radius_weight += radius * activity;
    if (radius <= near_radius) {
      out.near_activity += activity;
    } else {
      const double directed = directed_sign * dx;
      if (directed > near_radius) out.leading_activity += activity;
      else if (directed < -near_radius) out.trailing_activity += activity;
      else out.transverse_activity += activity;
    }

    if (radius > 0.0 && radius <= alignment_radius) {
      const double magnitude = voxel.flux.mag();
      if (magnitude > 0.0) {
        radial_numerator += static_cast<double>(polarity)
            * (voxel.flux.x * dx + voxel.flux.y * dy
               + voxel.flux.z * dz) / radius;
        radial_denominator += magnitude;
      }
    }
  }

  if (out.activity > 0.0) {
    out.mean_radius = radius_weight / out.activity;
    out.near_fraction = out.near_activity / out.activity;
    out.leading_fraction = out.leading_activity / out.activity;
    out.trailing_fraction = out.trailing_activity / out.activity;
    out.transverse_fraction = out.transverse_activity / out.activity;
  }
  if (radial_denominator > 0.0)
    out.radial_alignment = radial_numerator / radial_denominator;
  out.signed_source_divergence = static_cast<double>(polarity)
      * bridge.divergence_flux(source_index);
  const auto energy = measure_native_wave_energy(bridge);
  out.exact_tick_energy = energy.tick_invariant;
  out.valid = energy.finite && std::isfinite(out.activity)
      && std::isfinite(out.field_norm2) && std::isfinite(out.wave_norm2)
      && std::isfinite(out.mean_radius) && std::isfinite(out.near_fraction)
      && std::isfinite(out.radial_alignment)
      && std::isfinite(out.signed_source_divergence);
  return out;
}

inline double flux_odd_mirror_residual(const RenderBridge& positive,
                                       const RenderBridge& negative) {
  if (positive.voxels().size() != negative.voxels().size())
    return std::numeric_limits<double>::infinity();
  long double residual2 = 0.0L;
  long double scale2 = 0.0L;
  for (std::size_t i = 0; i < positive.voxels().size(); ++i) {
    const Vec3 residual = positive.voxels()[i].flux
        + negative.voxels()[i].flux;
    residual2 += dot_long_double(residual, residual);
    scale2 += dot_long_double(positive.voxels()[i].flux,
                             positive.voxels()[i].flux);
  }
  return std::sqrt(static_cast<double>(residual2
      / std::max(1e-60L, scale2)));
}

inline std::uint64_t dynamical_dressing_state_hash(
    const RenderBridge& bridge) {
  std::uint64_t hash = 1469598103934665603ull;
  for (const auto& voxel : bridge.voxels()) {
    dynamical_dressing_detail::hash_bytes(hash, &voxel.state,
                                          sizeof(voxel.state));
    dynamical_dressing_detail::hash_bytes(hash, &voxel.flux,
                                          sizeof(voxel.flux));
    dynamical_dressing_detail::hash_bytes(hash, &voxel.wave_vel,
                                          sizeof(voxel.wave_vel));
    dynamical_dressing_detail::hash_bytes(hash, &voxel.velocity,
                                          sizeof(voxel.velocity));
    dynamical_dressing_detail::hash_bytes(hash, &voxel.remainder,
                                          sizeof(voxel.remainder));
    dynamical_dressing_detail::hash_bytes(hash, &voxel.locked,
                                          sizeof(voxel.locked));
  }
  return hash;
}

}  // namespace ftd::eft
