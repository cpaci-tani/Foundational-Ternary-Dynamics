#pragma once
/**
 * @file symmetric_diagonal_coupled_endpoint.h
 * @brief Energy-coupled symmetric edge/corner endpoint observer (FTD-0531).
 */

#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/overshoot_preserving_contact_rebase.h"

namespace ftd::eft {

struct SymmetricDiagonalCoupledEndpointResult {
  bool valid = false;
  bool root_bracketed = false;
  bool converged = false;
  bool monotonic_on_locked_grid = false;
  int iterations = 0;
  int shell = 0;
  double interaction_scale = 0.0;
  double momentum_before = 0.0;
  double momentum_after = 0.0;
  double momentum_change = 0.0;
  double energy_before_per_carrier = 0.0;
  double energy_after_per_carrier = 0.0;
  double displacement_magnitude = 0.0;
  double reference_displacement_magnitude = 0.0;
  double endpoint_change = 0.0;
  double speed = 0.0;
  double minimum_monotonic_increment = 0.0;
  double root_residual = 0.0;
  double continuity_residual = 0.0;
  double gauss_before_residual = 0.0;
  double gauss_after_residual = 0.0;
  double staggered_embedding_residual = 0.0;
  double field_work_residual = 0.0;
  double matter_work_residual = 0.0;
  double total_energy_residual = 0.0;
  double displacement_residual = 0.0;
  double causal_excess = 0.0;
  double inverse_residual = 0.0;
  double reference_transverse_norm_squared = 0.0;
  FaceFluxNormalization normalization{};
  OvershootPreservingContactRebaseResult rebase{};
};

SymmetricDiagonalCoupledEndpointResult
solve_symmetric_diagonal_coupled_endpoint(
    int L,
    const Vec3& contact_position,
    Coord diagonal_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft

