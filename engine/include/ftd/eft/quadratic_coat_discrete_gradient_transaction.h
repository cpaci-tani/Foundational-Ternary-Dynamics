#pragma once
/**
 * @file quadratic_coat_discrete_gradient_transaction.h
 * @brief Selected reciprocal quadratic-coat matter/field step (FTD-0551).
 *
 * Observer only.  This does not alter the production tick or install a force.
 */

#include "ftd/eft/coupled_matched_face_transaction.h"
#include "ftd/eft/quadratic_coat_orbit_gather.h"

namespace ftd::eft {

using QuadraticCoatDGOptions = CoupledMatchedFaceOptions;

struct QuadraticCoatDGTransaction {
  bool valid = false;
  bool gates_pass = false;
  int charge = 0;
  FaceFluxNormalization normalization{};
  double interaction_scale = 0.0;
  CoupledMatchedFaceState before;
  CoupledMatchedFaceState after;
  QuadraticCoatFaceCurrent segment{};
  QuadraticCoatOrbitGatherResult gather{};
  LocalImplicitSolveDiagnostics solve{};
  Vec3 displacement{};
  Vec3 discrete_gradient_velocity{};
  Vec3 electric_impulse{};
  Vec3 magnetic_impulse{};
  Vec3 total_impulse{};
  double particle_energy_before = 0.0;
  double particle_energy_after = 0.0;
  double field_energy_before = 0.0;
  double field_energy_after = 0.0;
  double current_work = 0.0;
  double continuity_residual = 0.0;
  double gauss_before_residual = 0.0;
  double gauss_after_residual = 0.0;
  double solve_residual = 0.0;
  double force_residual = 0.0;
  double discrete_gradient_residual = 0.0;
  double electric_work_residual = 0.0;
  double field_work_residual = 0.0;
  double total_energy_residual = 0.0;
  double magnetic_work_residual = 0.0;
  double kinematic_residual = 0.0;
  double causal_speed_excess = 0.0;
  double inverse_residual = 0.0;
};

QuadraticCoatDGTransaction solve_quadratic_coat_dg_transaction(
    const CoupledMatchedFaceState& before,
    int charge,
    const std::vector<double>& stationary_density,
    const QuadraticCoatDGOptions& options = {});

}  // namespace ftd::eft
