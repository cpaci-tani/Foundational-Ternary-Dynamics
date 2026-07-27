#pragma once
/**
 * @file axial_contact_longitudinal_work.h
 * @brief Gauss-fixed axial contact work audit (FTD-0530).
 */

#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/overshoot_preserving_contact_rebase.h"

namespace ftd::eft {

struct AxialContactLongitudinalWorkResult {
  bool valid = false;
  bool fixed_path_obstruction = false;
  int axis = -1;
  double interaction_scale = 0.0;
  double history_residual = 0.0;
  double continuity_residual = 0.0;
  double gauss_residual = 0.0;
  double curl_adjoint_norm_squared = 0.0;
  double harmonic_current_residual = 0.0;
  double current_norm_squared = 0.0;
  double endpoint_density_change_residual = 0.0;
  double transverse_work_difference = 0.0;
  double harmonic_work_difference = 0.0;
  double staggered_embedding_residual = 0.0;
  double field_energy_identity_residual = 0.0;
  double common_field_change = 0.0;
  double unchanged_total_energy_residual = 0.0;
  double initial_energy_per_carrier = 0.0;
  double required_energy_per_carrier = 0.0;
  double required_momentum_magnitude = 0.0;
  double required_speed = 0.0;
  double required_impulse_magnitude = 0.0;
  double frozen_path_correction_residual = 0.0;
  FaceFluxNormalization normalization{};
  OvershootPreservingContactRebaseResult rebase{};
};

AxialContactLongitudinalWorkResult analyze_axial_contact_longitudinal_work(
    int L,
    const Vec3& contact_position,
    Coord axial_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft
