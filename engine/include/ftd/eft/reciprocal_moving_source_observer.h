#pragma once
/**
 * @file reciprocal_moving_source_observer.h
 * @brief Read-only matter-conditioned field morphology for FTD-0477.
 *
 * The observer subtracts a matched driver-only history from a combined
 * source+driver history.  The resulting field is never fed back into either
 * bridge.  Streamline, dressing, wake, and detached-field labels remain
 * classifications of the frozen production histories.
 */

#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace ftd::eft {

struct ReciprocalMovingSourceObservation {
  bool valid = false;
  double activity = 0.0;
  double near_activity = 0.0;
  double detached_activity = 0.0;
  double near_fraction = 0.0;
  double leading_activity = 0.0;
  double trailing_activity = 0.0;
  double transverse_activity = 0.0;
  double leading_fraction = 0.0;
  double trailing_fraction = 0.0;
  double transverse_fraction = 0.0;
  double trailing_to_leading = 0.0;
  double mean_radius = 0.0;
  double shifted_source_correlation = 0.0;
  double field_norm2 = 0.0;
  double wave_norm2 = 0.0;
  Vec3 selected_field_momentum{};
};

namespace reciprocal_moving_source_detail {

inline double periodic_delta(double value, double centre, int length) {
  double delta = value - centre;
  while (delta > 0.5 * length) delta -= length;
  while (delta < -0.5 * length) delta += length;
  return delta;
}

inline bool finite_vec(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

}  // namespace reciprocal_moving_source_detail

inline ReciprocalMovingSourceObservation
observe_reciprocal_moving_source(
    const RenderBridge& combined, const RenderBridge& driver_only,
    const RenderBridge& source_only, int source_index,
    const Vec3& integer_displacement, const Vec3& direction,
    double near_radius = 4.0) {
  ReciprocalMovingSourceObservation out;
  if (combined.lattice().size() != driver_only.lattice().size()
      || combined.lattice().size() != source_only.lattice().size()
      || combined.voxels().size() != driver_only.voxels().size()
      || combined.voxels().size() != source_only.voxels().size()
      || source_index < 0
      || source_index >= static_cast<int>(combined.voxels().size()))
    return out;

  const int length = combined.lattice().size();
  const auto source = combined.lattice().coord(source_index);
  std::vector<Voxel> difference(combined.voxels().size());
  for (std::size_t index = 0; index < difference.size(); ++index) {
    difference[index].flux = combined.voxels()[index].flux
        - driver_only.voxels()[index].flux;
    difference[index].wave_vel = combined.voxels()[index].wave_vel
        - driver_only.voxels()[index].wave_vel;
  }

  const double direction_magnitude = direction.mag();
  const Vec3 direction_hat = direction_magnitude > 0.0
      ? direction * (1.0 / direction_magnitude) : Vec3{};
  double radius_weight = 0.0;
  long double correlation_numerator = 0.0L;
  long double correlation_left = 0.0L;
  long double correlation_right = 0.0L;

  for (int index = 0;
       index < static_cast<int>(difference.size()); ++index) {
    const auto& voxel = difference[static_cast<std::size_t>(index)];
    const double activity = field_kinetic_term(voxel.wave_vel)
        - field_gradient_term(
            voxel.flux, combined.lattice().neighbors_6(index),
            combined.lattice().neighbors_12(index), difference);
    const auto coordinate = combined.lattice().coord(index);
    const Vec3 offset{
        reciprocal_moving_source_detail::periodic_delta(
            coordinate.x, source.x, length),
        reciprocal_moving_source_detail::periodic_delta(
            coordinate.y, source.y, length),
        reciprocal_moving_source_detail::periodic_delta(
            coordinate.z, source.z, length)};
    const double radius = offset.mag();

    out.activity += activity;
    out.field_norm2 += voxel.flux.mag2();
    out.wave_norm2 += voxel.wave_vel.mag2();
    radius_weight += radius * activity;
    if (radius <= near_radius) {
      out.near_activity += activity;
    } else if (direction_magnitude > 0.0) {
      const double directed = offset.dot(direction_hat);
      if (directed > near_radius) out.leading_activity += activity;
      else if (directed < -near_radius) out.trailing_activity += activity;
      else out.transverse_activity += activity;
    } else {
      out.transverse_activity += activity;
    }

    const int reference_index = source_only.lattice().index(
        coordinate.x - static_cast<int>(std::llround(integer_displacement.x)),
        coordinate.y - static_cast<int>(std::llround(integer_displacement.y)),
        coordinate.z - static_cast<int>(std::llround(integer_displacement.z)));
    const auto& reference = source_only.voxels()[
        static_cast<std::size_t>(reference_index)];
    correlation_numerator += static_cast<long double>(
        voxel.flux.dot(reference.flux)
        + voxel.wave_vel.dot(reference.wave_vel));
    correlation_left += static_cast<long double>(
        voxel.flux.mag2() + voxel.wave_vel.mag2());
    correlation_right += static_cast<long double>(
        reference.flux.mag2() + reference.wave_vel.mag2());
  }

  out.detached_activity = std::max(0.0, out.activity - out.near_activity);
  if (out.activity > 0.0) {
    out.near_fraction = out.near_activity / out.activity;
    out.leading_fraction = out.leading_activity / out.activity;
    out.trailing_fraction = out.trailing_activity / out.activity;
    out.transverse_fraction = out.transverse_activity / out.activity;
    out.mean_radius = radius_weight / out.activity;
  }
  out.trailing_to_leading = out.trailing_activity
      / std::max(1e-30, out.leading_activity);
  if (correlation_left > 0.0L && correlation_right > 0.0L) {
    out.shifted_source_correlation = static_cast<double>(
        correlation_numerator
        / std::sqrt(correlation_left * correlation_right));
  }

  for (int x = 0; x < length; ++x) {
    for (int y = 0; y < length; ++y) {
      for (int z = 0; z < length; ++z) {
        const int index = combined.lattice().index(x, y, z);
        const auto& wave = difference[static_cast<std::size_t>(index)].wave_vel;
        const Vec3 derivative_x = (
            difference[static_cast<std::size_t>(
                combined.lattice().index(x + 1, y, z))].flux
            - difference[static_cast<std::size_t>(
                combined.lattice().index(x - 1, y, z))].flux) * 0.5;
        const Vec3 derivative_y = (
            difference[static_cast<std::size_t>(
                combined.lattice().index(x, y + 1, z))].flux
            - difference[static_cast<std::size_t>(
                combined.lattice().index(x, y - 1, z))].flux) * 0.5;
        const Vec3 derivative_z = (
            difference[static_cast<std::size_t>(
                combined.lattice().index(x, y, z + 1))].flux
            - difference[static_cast<std::size_t>(
                combined.lattice().index(x, y, z - 1))].flux) * 0.5;
        out.selected_field_momentum.x -= wave.dot(derivative_x);
        out.selected_field_momentum.y -= wave.dot(derivative_y);
        out.selected_field_momentum.z -= wave.dot(derivative_z);
      }
    }
  }

  out.valid = out.activity > 0.0
      && std::isfinite(out.activity) && std::isfinite(out.near_fraction)
      && std::isfinite(out.trailing_to_leading)
      && std::isfinite(out.mean_radius)
      && std::isfinite(out.shifted_source_correlation)
      && std::isfinite(out.field_norm2) && std::isfinite(out.wave_norm2)
      && reciprocal_moving_source_detail::finite_vec(
          out.selected_field_momentum);
  return out;
}

}  // namespace ftd::eft

