#pragma once
/**
 * @file passive_dressing_depinning_obstruction.h
 * @brief Observer-only passive-dressing/depinning discriminator (FTD-0581).
 */

namespace ftd::eft {

struct PassiveDressingDepinningObstructionResult {
  bool valid = false;
  int depinning_arms = 0;
  int passive_fixture_arms = 0;
  int passive_samples = 0;
  int active_budget_samples = 0;
  int equality_nondifferentiable_arms = 0;
  int smooth_excited_arms = 0;
  int cubic_rotation_arms = 0;
  double maximum_threshold_energy_residual = 0.0;
  double maximum_inverse_momentum_residual = 0.0;
  double maximum_velocity_identity_residual = 0.0;
  double maximum_polarity_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double maximum_passive_linear_coefficient = 0.0;
  double maximum_passive_negative_excess = 0.0;
  double minimum_cusp_slope_gap = 0.0;
  double maximum_active_budget_residual = 0.0;
  double maximum_equality_midpoint_residual = 0.0;
  double minimum_equality_derivative_jump = 0.0;
  double minimum_peierls_coefficient = 0.0;
  double maximum_peierls_coefficient = 0.0;
  double minimum_barrier = 0.0;
  double maximum_barrier = 0.0;
  double minimum_depinning_momentum = 0.0;
  double maximum_depinning_momentum = 0.0;
  double minimum_depinning_speed = 0.0;
  double maximum_depinning_speed = 0.0;
  bool exact_relativistic_depinning = false;
  bool passive_completed_square_obstruction = false;
  bool passive_cusp_obstruction = false;
  bool active_excitation_lower_bound = false;
  bool active_common_action_derived = false;
  bool production_changed = false;
};

PassiveDressingDepinningObstructionResult
analyze_passive_dressing_depinning_obstruction();

}  // namespace ftd::eft
