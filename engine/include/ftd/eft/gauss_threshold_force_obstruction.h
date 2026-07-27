#pragma once
/**
 * @file gauss_threshold_force_obstruction.h
 * @brief Local Gauss lower bound on compact point-force jumps (FTD-0487).
 */

#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft {

struct GaussThresholdForceObstruction {
  bool valid = false;
  int L = 0;
  double coupling = 0.0;
  double temporal_scale = 0.0;
  std::vector<double> jump_x;
  std::vector<double> jump_y;
  std::vector<double> jump_z;
  double divergence_identity_residual = 0.0;
  double gauss_residual = 0.0;
  double pointwise_bound_violation = 0.0;
  double maximum_component_jump = 0.0;
  double maximum_source_magnitude = 0.0;
  double minimum_nonzero_source_ratio = 0.0;
  double normalized_impulse_lower_bound = 0.0;
};

inline GaussThresholdForceObstruction
analyze_gauss_threshold_force_obstruction(
    const MatchedFaceFlux& electric,
    const std::vector<double>& source,
    double coupling,
    double temporal_scale) {
  GaussThresholdForceObstruction result;
  result.L = electric.L;
  result.coupling = coupling;
  result.temporal_scale = temporal_scale;
  const std::size_t count = electric.L > 0
      ? static_cast<std::size_t>(electric.L * electric.L * electric.L) : 0;
  if (electric.L <= 0 || source.size() != count
      || electric.x.size() != count || electric.y.size() != count
      || electric.z.size() != count || !std::isfinite(coupling)
      || !std::isfinite(temporal_scale) || temporal_scale <= 0.0) {
    return result;
  }
  const auto finite = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(),
        [](double value) { return std::isfinite(value); });
  };
  if (!finite(source) || !finite(electric.x)
      || !finite(electric.y) || !finite(electric.z)) {
    return result;
  }

  result.jump_x.assign(count, 0.0);
  result.jump_y.assign(count, 0.0);
  result.jump_z.assign(count, 0.0);
  result.minimum_nonzero_source_ratio = INFINITY;
  for (int x = 0; x < electric.L; ++x) {
    for (int y = 0; y < electric.L; ++y) {
      for (int z = 0; z < electric.L; ++z) {
        const int index = electric.index(x, y, z);
        const std::size_t i = static_cast<std::size_t>(index);
        const double jump_x = electric.x[i]
            - electric.x[static_cast<std::size_t>(
                electric.index(x - 1, y, z))];
        const double jump_y = electric.y[i]
            - electric.y[static_cast<std::size_t>(
                electric.index(x, y - 1, z))];
        const double jump_z = electric.z[i]
            - electric.z[static_cast<std::size_t>(
                electric.index(x, y, z - 1))];
        result.jump_x[i] = jump_x;
        result.jump_y[i] = jump_y;
        result.jump_z[i] = jump_z;
        const double sum = jump_x + jump_y + jump_z;
        const double max_jump = std::max({
            std::abs(jump_x), std::abs(jump_y), std::abs(jump_z)});
        const double source_magnitude = std::abs(source[i]);
        result.divergence_identity_residual = std::max(
            result.divergence_identity_residual,
            std::abs(sum - divergence_at(electric, x, y, z)));
        result.gauss_residual = std::max(
            result.gauss_residual, std::abs(sum - source[i]));
        result.pointwise_bound_violation = std::max(
            result.pointwise_bound_violation,
            std::max(0.0, source_magnitude / 3.0 - max_jump));
        result.maximum_component_jump = std::max(
            result.maximum_component_jump, max_jump);
        result.maximum_source_magnitude = std::max(
            result.maximum_source_magnitude, source_magnitude);
        if (source_magnitude > 0.0) {
          result.minimum_nonzero_source_ratio = std::min(
              result.minimum_nonzero_source_ratio,
              3.0 * max_jump / source_magnitude);
        }
      }
    }
  }
  if (!std::isfinite(result.minimum_nonzero_source_ratio)) {
    result.minimum_nonzero_source_ratio = 0.0;
  }
  result.normalized_impulse_lower_bound =
      std::abs(coupling * temporal_scale)
      * result.maximum_source_magnitude / 3.0;
  result.valid = std::isfinite(result.divergence_identity_residual)
      && std::isfinite(result.gauss_residual)
      && std::isfinite(result.pointwise_bound_violation)
      && std::isfinite(result.normalized_impulse_lower_bound);
  return result;
}

}  // namespace ftd::eft
