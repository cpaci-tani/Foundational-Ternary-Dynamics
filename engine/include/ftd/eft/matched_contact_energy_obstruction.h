#pragma once
/**
 * @file matched_contact_energy_obstruction.h
 * @brief Field-independent elastic-contact energy obstruction (FTD-0529).
 */

#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/overshoot_preserving_contact_rebase.h"

namespace ftd::eft {

struct MatchedContactEnergyObstructionResult {
  bool valid = false;
  bool obstruction_present = false;
  double challenge_amplitude = 0.125;
  double interaction_scale = 0.0;
  double history_residual = 0.0;
  double continuity_residual = 0.0;
  double gauss_before_residual = 0.0;
  double gauss_after_residual = 0.0;
  double challenge_divergence_residual = 0.0;
  double adjoint_identity_residual = 0.0;
  double staggered_embedding_residual = 0.0;
  double baseline_field_identity_residual = 0.0;
  double challenge_field_identity_residual = 0.0;
  double energy_split_formula_residual = 0.0;
  double matter_energy_change = 0.0;
  double transverse_norm_squared = 0.0;
  double predicted_energy_split = 0.0;
  double measured_energy_split = 0.0;
  double baseline_total_energy_residual = 0.0;
  double challenge_total_energy_residual = 0.0;
  double elastic_incompatibility_margin = 0.0;
  FaceFluxNormalization normalization{};
  OvershootPreservingContactRebaseResult rebase{};
};

/** Compare the unchanged FTD-0527 elastic output on two face fields with the
 * same Gauss source. A positive C^T K norm proves that the field-blind map
 * cannot conserve matched field-plus-matter energy for both inputs. */
MatchedContactEnergyObstructionResult
analyze_matched_contact_energy_obstruction(
    int L,
    const Vec3& contact_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft

