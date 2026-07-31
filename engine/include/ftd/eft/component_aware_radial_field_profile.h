#pragma once
/**
 * @file component_aware_radial_field_profile.h
 * @brief Fixed-origin component-aware radial field morphology (FTD-0683).
 *
 * Face and edge components are binned at their actual staggered geometric
 * locations.  Doubled Chebyshev radii are integers, avoiding floating shell
 * membership and storage-cell-centre approximations.
 */

#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/voxel.h"

#include <vector>

namespace ftd::eft {

struct ComponentAwareRadialFieldProfile {
  bool valid = false;
  bool finite = false;
  bool zero_profile = false;
  int L = 0;
  Vec3 origin{};
  double field_energy_scale = 0.0;
  double wave_speed = 0.0;

  // Index k is doubled periodic Chebyshev radius rho2=k.
  std::vector<double> shell_norm_by_doubled_radius;
  std::vector<double> cumulative_norm_by_doubled_radius;

  double total_norm = 0.0;
  double direct_total_norm = 0.0;
  double partition_residual = 0.0;
  double cumulative_residual = 0.0;
  double monotonicity_residual = 0.0;
  double mean_radius = 0.0;
  double rms_radius = 0.0;
  int doubled_radius_50 = 0;
  int doubled_radius_90 = 0;
  int doubled_radius_99 = 0;
};

ComponentAwareRadialFieldProfile observe_component_aware_radial_field_profile(
    const MatchedFaceFlux& reference_electric,
    const MatchedEdgeField& reference_magnetic,
    const MatchedFaceFlux& candidate_electric,
    const MatchedEdgeField& candidate_magnetic,
    const Vec3& integer_origin,
    double field_energy_scale,
    double wave_speed = C_SPEED,
    double tolerance = 1e-12);

}  // namespace ftd::eft
