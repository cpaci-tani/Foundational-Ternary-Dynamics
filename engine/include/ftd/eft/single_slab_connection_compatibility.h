#pragma once
/**
 * @file single_slab_connection_compatibility.h
 * @brief Faraday compatibility of the FTD-0531 work field and staggered
 *        magnetic history (FTD-0534).
 */

#include "ftd/eft/symmetric_diagonal_coupled_endpoint.h"

namespace ftd::eft {

struct SingleSlabConnectionCompatibilityResult {
  bool valid = false;
  bool used_coupled_endpoint = false;
  bool single_slab_faraday_compatible = false;
  int shell = 0;
  double displacement_magnitude = 0.0;
  double current_l1 = 0.0;
  double current_curl_norm_squared = 0.0;
  double faraday_mismatch_norm_squared = 0.0;
  double predicted_mismatch_norm_squared = 0.0;
  double component_identity_residual = 0.0;
  double norm_identity_residual = 0.0;
  double continuity_residual = 0.0;
  double inherited_endpoint_residual = 0.0;
  SymmetricDiagonalCoupledEndpointResult coupled{};
  OvershootPreservingContactRebaseResult rebase{};
};

SingleSlabConnectionCompatibilityResult
analyze_single_slab_connection_compatibility(
    int L,
    const Vec3& contact_position,
    Coord moore_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft

