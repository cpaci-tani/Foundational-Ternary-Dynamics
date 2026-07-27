#pragma once
/**
 * @file staggered_current_split_compatibility.h
 * @brief Exact FTD-0484 endpoint-current split versus frozen staggered field
 *        ordering (FTD-0535).
 */

#include "ftd/eft/single_slab_connection_compatibility.h"

namespace ftd::eft {

struct StaggeredCurrentSplitCompatibilityResult {
  bool valid = false;
  bool used_coupled_endpoint = false;
  bool frozen_staggered_transverse_compatible = false;
  int shell = 0;
  double total_current_l1 = 0.0;
  double start_current_l1 = 0.0;
  double end_current_l1 = 0.0;
  double start_current_curl_norm_squared = 0.0;
  double end_current_curl_norm_squared = 0.0;
  double faraday_mismatch_norm_squared = 0.0;
  double predicted_mismatch_norm_squared = 0.0;
  double split_recombination_residual = 0.0;
  double component_identity_residual = 0.0;
  double norm_identity_residual = 0.0;
  double split_continuity_residual = 0.0;
  double inherited_endpoint_residual = 0.0;
  SymmetricDiagonalCoupledEndpointResult coupled{};
  OvershootPreservingContactRebaseResult rebase{};
};

StaggeredCurrentSplitCompatibilityResult
analyze_staggered_current_split_compatibility(
    int L,
    const Vec3& contact_position,
    Coord moore_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft

