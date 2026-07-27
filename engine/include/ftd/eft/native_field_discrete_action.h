#pragma once
/**
 * @file native_field_discrete_action.h
 * @brief Observer-only native wave-action and source-operator audit (FTD-0574).
 *
 * The observer reconstructs the frozen source-free kick-drift map and the
 * prescribed state/velocity source from independent finite-dimensional and
 * periodic-lattice algebra.  It never reads or modifies RenderBridge state.
 */

namespace ftd::eft {

struct NativeFieldDiscreteActionResult {
  bool valid = false;

  int mode_arms = 0;
  int lattice_action_arms = 0;
  int source_operator_arms = 0;
  int uniform_counterexample_arms = 0;
  int proper_cubic_covariance_arms = 0;
  int invariant_constraint_rank = 0;
  int invariant_constraint_nullity = 0;

  double maximum_symplectic_residual = 0.0;
  double maximum_discrete_el_residual = 0.0;
  double maximum_legendre_momentum_residual = 0.0;
  double maximum_tick_invariant_residual = 0.0;
  double maximum_shadow_flow_residual = 0.0;
  double maximum_invariant_matrix_residual = 0.0;
  double maximum_electric_adjoint_residual = 0.0;
  double maximum_curl_adjoint_residual = 0.0;
  double maximum_correct_source_action_residual = 0.0;
  double maximum_documented_action_derivative_residual = 0.0;
  double maximum_affine_source_symplectic_residual = 0.0;
  double maximum_proper_cubic_covariance_residual = 0.0;
  double minimum_uniform_documented_source_mismatch = 0.0;
  double maximum_uniform_coded_source = 0.0;

  bool local_discrete_action_reproduces_tick = false;
  bool wave_velocity_is_legendre_momentum = false;
  bool standard_pairing_is_native = false;
  bool normalized_tick_invariant_is_unique = false;
  bool exact_continuous_shadow_generator_is_nonlocal = false;
  bool prescribed_source_action_reproduces_phase_read = false;
  bool prescribed_source_map_is_affine_symplectic = false;
  bool documented_velocity_interaction_generates_coded_source = false;
  bool full_dynamic_matter_field_action_derived = false;
  bool production_changed = false;
};

NativeFieldDiscreteActionResult analyze_native_field_discrete_action();

}  // namespace ftd::eft
