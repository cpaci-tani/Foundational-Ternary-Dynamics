#pragma once
/**
 * @file contact_quotient_coupling_scope.h
 * @brief Native snapshot source versus exact history-current quotient audit
 *        (FTD-0528).
 */

#include "ftd/eft/overshoot_preserving_contact_rebase.h"

namespace ftd::eft {

struct ContactQuotientCouplingScopeResult {
  bool valid = false;
  bool axial = false;
  bool native_snapshot_factors = false;
  bool matched_history_factors = false;
  double coupling_formula_residual = 0.0;
  double gradient_source_difference = 0.0;
  double curl_source_difference = 0.0;
  double native_response_difference = 0.0;
  double curl_explanation_residual = 0.0;
  double matched_density_residual = 0.0;
  double matched_current_residual = 0.0;
  double matched_field_response_residual = 0.0;
  double continuity_residual = 0.0;
  double common_output_native_residual = 0.0;
  OvershootPreservingContactRebaseResult rebase{};
};

/// Compare the actual pre-movement native coupling source with the complete
/// exact face-current history on the FTD-0527 contact quotient.
ContactQuotientCouplingScopeResult analyze_contact_quotient_coupling_scope(
    int L,
    const Vec3& contact_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft
