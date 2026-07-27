#pragma once
/**
 * @file ftd/eft/wave_morphology_observer.h
 * @brief Read-only bound/bow/wake morphology observer (FTD-0475).
 *
 * The positive profile below is an activity density used only to locate and
 * compare a packet.  It is not substituted for the exact modified energy of
 * the source-free kick-drift map, which is reported independently through
 * NativeWaveEnergy::tick_invariant.
 */

#include "ftd/eft/native_energy_contract.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace ftd::eft {

struct WaveMorphologyObservation {
  bool valid = false;
  std::vector<double> x_profile;
  double activity = 0.0;
  double centroid_x = 0.0;
  double width_x = 0.0;
  double core_fraction = 0.0;
  double leading_fraction = 0.0;
  double trailing_fraction = 0.0;
  double normalized_divergence = 0.0;
  long double exact_tick_energy = 0.0L;
};

struct WaveProfileComparison {
  bool valid = false;
  int best_shift = 0;
  double overlap = 0.0;
  double explained_fraction = 0.0;
  double leading_excess_fraction = 0.0;
  double trailing_excess_fraction = 0.0;
};

namespace wave_morphology_detail {

inline double periodic_delta(double value, double center, int length) {
  double delta = value - center;
  while (delta > 0.5 * length) delta -= length;
  while (delta < -0.5 * length) delta += length;
  return delta;
}

inline double positive_activity_density(const RenderBridge& bridge,
                                        int index) {
  const auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
  return field_kinetic_term(voxel.wave_vel)
      - field_gradient_term(voxel.flux,
                            bridge.lattice().neighbors_6(index),
                            bridge.lattice().neighbors_12(index),
                            bridge.voxels());
}

}  // namespace wave_morphology_detail

inline WaveMorphologyObservation observe_wave_morphology(
    const RenderBridge& bridge, int propagation_sign,
    double core_half_width) {
  WaveMorphologyObservation out;
  const int length = bridge.lattice().size();
  out.x_profile.assign(static_cast<std::size_t>(length), 0.0);

  double divergence2 = 0.0;
  double flux2 = 0.0;
  for (int index = 0;
       index < static_cast<int>(bridge.lattice().total_sites()); ++index) {
    const auto coordinate = bridge.lattice().coord(index);
    const double activity =
        wave_morphology_detail::positive_activity_density(bridge, index);
    out.x_profile[static_cast<std::size_t>(coordinate.x)] += activity;
    out.activity += activity;
    const double divergence = bridge.divergence_flux(index);
    divergence2 += divergence * divergence;
    flux2 += bridge.voxels()[static_cast<std::size_t>(index)].flux.mag2();
  }

  if (!(out.activity > 0.0) || !std::isfinite(out.activity)) return out;

  double cosine = 0.0;
  double sine = 0.0;
  for (int x = 0; x < length; ++x) {
    const double angle = 2.0 * PI * static_cast<double>(x) / length;
    const double weight = out.x_profile[static_cast<std::size_t>(x)];
    cosine += weight * std::cos(angle);
    sine += weight * std::sin(angle);
  }
  double angle = std::atan2(sine, cosine);
  if (angle < 0.0) angle += 2.0 * PI;
  out.centroid_x = angle * length / (2.0 * PI);

  double core = 0.0;
  double leading = 0.0;
  double trailing = 0.0;
  const double sign = propagation_sign >= 0 ? 1.0 : -1.0;
  for (int x = 0; x < length; ++x) {
    const double weight = out.x_profile[static_cast<std::size_t>(x)];
    const double delta = wave_morphology_detail::periodic_delta(
        static_cast<double>(x), out.centroid_x, length);
    out.width_x += weight * delta * delta;
    const double directed = sign * delta;
    if (std::abs(directed) <= core_half_width) core += weight;
    else if (directed > core_half_width) leading += weight;
    else trailing += weight;
  }
  out.width_x = std::sqrt(out.width_x / out.activity);
  out.core_fraction = core / out.activity;
  out.leading_fraction = leading / out.activity;
  out.trailing_fraction = trailing / out.activity;
  out.normalized_divergence = std::sqrt(
      divergence2 / std::max(1e-30, flux2));
  const auto energy = measure_native_wave_energy(bridge);
  out.exact_tick_energy = energy.tick_invariant;
  out.valid = energy.finite && std::isfinite(out.centroid_x)
      && std::isfinite(out.width_x)
      && std::isfinite(out.normalized_divergence);
  return out;
}

inline WaveProfileComparison compare_wave_profiles(
    const WaveMorphologyObservation& reference,
    const WaveMorphologyObservation& current,
    int propagation_sign, double core_half_width) {
  WaveProfileComparison out;
  if (!reference.valid || !current.valid
      || reference.x_profile.size() != current.x_profile.size()) return out;
  const int length = static_cast<int>(reference.x_profile.size());
  double reference2 = 0.0;
  double current2 = 0.0;
  for (int x = 0; x < length; ++x) {
    reference2 += reference.x_profile[static_cast<std::size_t>(x)]
        * reference.x_profile[static_cast<std::size_t>(x)];
    current2 += current.x_profile[static_cast<std::size_t>(x)]
        * current.x_profile[static_cast<std::size_t>(x)];
  }
  const double denominator = std::sqrt(reference2 * current2);
  if (!(denominator > 0.0)) return out;

  out.overlap = -std::numeric_limits<double>::infinity();
  for (int shift = -length / 2; shift <= length / 2; ++shift) {
    double dot = 0.0;
    for (int x = 0; x < length; ++x) {
      int source = (x - shift) % length;
      if (source < 0) source += length;
      dot += current.x_profile[static_cast<std::size_t>(x)]
          * reference.x_profile[static_cast<std::size_t>(source)];
    }
    const double overlap = dot / denominator;
    if (overlap > out.overlap) {
      out.overlap = overlap;
      out.best_shift = shift;
    }
  }

  const double scale = current.activity / reference.activity;
  double absolute_residual = 0.0;
  double leading_excess = 0.0;
  double trailing_excess = 0.0;
  const double sign = propagation_sign >= 0 ? 1.0 : -1.0;
  for (int x = 0; x < length; ++x) {
    int source = (x - out.best_shift) % length;
    if (source < 0) source += length;
    const double residual = current.x_profile[static_cast<std::size_t>(x)]
        - scale * reference.x_profile[static_cast<std::size_t>(source)];
    absolute_residual += std::abs(residual);
    if (residual <= 0.0) continue;
    const double delta = wave_morphology_detail::periodic_delta(
        static_cast<double>(x), current.centroid_x, length);
    const double directed = sign * delta;
    if (directed > core_half_width) leading_excess += residual;
    if (directed < -core_half_width) trailing_excess += residual;
  }
  out.explained_fraction = std::max(
      0.0, 1.0 - 0.5 * absolute_residual / current.activity);
  out.leading_excess_fraction = leading_excess / current.activity;
  out.trailing_excess_fraction = trailing_excess / current.activity;
  out.valid = std::isfinite(out.overlap)
      && std::isfinite(out.explained_fraction)
      && std::isfinite(out.leading_excess_fraction)
      && std::isfinite(out.trailing_excess_fraction);
  return out;
}

}  // namespace ftd::eft
