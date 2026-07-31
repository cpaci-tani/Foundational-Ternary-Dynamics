#pragma once
/** @file batched_regional_energy_profile.h
 *  @brief Algebraically equivalent multi-radius FTD-0671 observer (FTD-0686).
 */

#include "ftd/eft/matched_regional_energy_transport.h"

#include <vector>

namespace ftd::eft {

struct BatchedRegionalEnergyProfile {
  bool valid = false;
  int L = 0;
  Vec3 center{};
  double lambda = 0.0;
  double energy_before = 0.0;
  double energy_pre_current = 0.0;
  double energy_after = 0.0;
  double maximum_scalar_equivalence_residual = 0.0;
  std::vector<MatchedRegionalEnergyTransportResult> regions;
};

BatchedRegionalEnergyProfile evaluate_batched_regional_energy_profile(
    const MatchedFaceFlux& electric_before,
    const MatchedEdgeField& magnetic_before,
    const MatchedFaceFlux& electric_pre_current,
    const MatchedEdgeField& magnetic_after,
    const MatchedFaceFlux& electric_after,
    double lambda,
    const Vec3& integer_center,
    const std::vector<int>& chebyshev_radii,
    double tolerance = 1e-12);

}  // namespace ftd::eft
