#pragma once
/**
 * @file constituent_complete_charged_trimer.h
 * @brief Observer-only lossless charged-trimer common-action transaction
 *        (FTD-0600).
 *
 * Matter is retained as three explicit ternary constituent phase-space
 * records.  Aggregate density and current are derived only for coupling to
 * the matched face/edge field.  This module does not modify RenderBridge or
 * production dynamics.
 */

#include "ftd/eft/matched_face_momentum_transaction.h"
#include "ftd/eft/quadratic_coat_discrete_gradient_transaction.h"

#include <array>
#include <string>
#include <vector>

namespace ftd::eft {

constexpr std::size_t CHARGED_TRIMER_SIZE = 3;

struct ChargedTrimerState {
  MatchedFaceFlux electric;
  MatchedEdgeField magnetic_half;
  std::array<MatchedMatterPoint, CHARGED_TRIMER_SIZE> constituents{};
  std::array<int, CHARGED_TRIMER_SIZE> charges{{-1, -1, +1}};

  explicit ChargedTrimerState(int size = 0)
      : electric(size), magnetic_half(size) {}
};

struct ChargedTrimerOptions {
  double wave_speed = C_SPEED;
  double dt = 1.0;
  double binding_stiffness = 1.0;
  double rest_length_squared = 2.0;
  double gate_tolerance = 1e-12;
  double solve_tolerance = 2e-13;
  double finite_difference_scale = 2e-7;
  int max_iterations = 64;
};

struct ChargedTrimerSolveDiagnostics {
  bool attempted = false;
  bool converged = false;
  int iterations = 0;
  int rejected_steps = 0;
  double residual = 0.0;
  double step_residual = 0.0;
  double minimum_abs_jacobian_determinant = 0.0;
};

struct ChargedTrimerStepResult {
  bool valid = false;
  bool gates_pass = false;
  bool forward = true;
  bool site_projection_valid = false;
  int net_charge = 0;
  FaceFluxNormalization normalization{};
  double interaction_scale = 0.0;
  ChargedTrimerState earlier;
  ChargedTrimerState later;
  ChargedTrimerSolveDiagnostics solve{};
  std::array<QuadraticCoatFaceCurrent, CHARGED_TRIMER_SIZE> segments{};
  std::array<QuadraticCoatOrbitGatherResult, CHARGED_TRIMER_SIZE> gathers{};
  std::array<Vec3, CHARGED_TRIMER_SIZE> velocities{};
  std::array<Vec3, CHARGED_TRIMER_SIZE> electric_impulses{};
  std::array<Vec3, CHARGED_TRIMER_SIZE> magnetic_impulses{};
  std::array<Vec3, CHARGED_TRIMER_SIZE> binding_impulses{};
  std::array<Vec3, CHARGED_TRIMER_SIZE> total_impulses{};

  double kinetic_energy_before = 0.0;
  double kinetic_energy_after = 0.0;
  double binding_energy_before = 0.0;
  double binding_energy_after = 0.0;
  double field_energy_before = 0.0;
  double field_energy_after = 0.0;
  double current_work = 0.0;
  Vec3 matter_momentum_before{};
  Vec3 matter_momentum_after{};
  Vec3 field_pseudomomentum_before{};
  Vec3 field_pseudomomentum_after{};
  Vec3 pseudomomentum_defect{};

  double root_residual = 0.0;
  double continuity_residual = 0.0;
  double gauss_before_residual = 0.0;
  double gauss_after_residual = 0.0;
  double force_residual = 0.0;
  double kinematic_residual = 0.0;
  double kinetic_discrete_gradient_residual = 0.0;
  double electric_adjoint_residual = 0.0;
  double magnetic_work_residual = 0.0;
  double binding_work_residual = 0.0;
  double binding_impulse_sum_residual = 0.0;
  double matter_work_residual = 0.0;
  double field_work_residual = 0.0;
  double total_energy_residual = 0.0;
  double causal_speed_excess = 0.0;
  double pseudomomentum_defect_norm = 0.0;
  double maximum_pair_distance = 0.0;
  double minimum_pair_distance = 0.0;

  explicit ChargedTrimerStepResult(int size = 0)
      : earlier(size), later(size) {}
};

/// Selected quartic pair binding energy from the FTD-0600 lock.
double charged_trimer_binding_energy(
    const ChargedTrimerState& state,
    const ChargedTrimerOptions& options = {});

/// Solve one forward common-action transaction from an earlier state.
ChargedTrimerStepResult solve_charged_trimer_forward(
    const ChargedTrimerState& earlier,
    const std::vector<double>& stationary_density,
    const ChargedTrimerOptions& options = {});

/// Solve one state-only reverse transaction from a later state.  No forward
/// current, endpoint, impulse, or branch record is accepted by this API.
ChargedTrimerStepResult solve_charged_trimer_reverse(
    const ChargedTrimerState& later,
    const std::vector<double>& stationary_density,
    const ChargedTrimerOptions& options = {});

/// Raw ordered-state comparison used only after applying the registered
/// equal-charge permutation to one side.
double charged_trimer_state_max_difference(
    const ChargedTrimerState& lhs,
    const ChargedTrimerState& rhs);

}  // namespace ftd::eft
