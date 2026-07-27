#pragma once
/**
 * @file genesis_environment_feedback.h
 * @brief Observer-only block-symplectic and existing-spectator audit for the
 *        accepted production genesis event (FTD-0571).
 */

namespace ftd::eft {

struct GenesisEnvironmentFeedbackResult {
  bool valid = false;
  int matrix_arms = 0;
  int rank_four_arms = 0;
  int rank_six_arms = 0;
  int continuous_spectator_components = 0;

  double maximum_defect_formula_residual = 0.0;
  double maximum_determinant_formula_residual = 0.0;
  double minimum_nonzero_symplectic_defect = 0.0;
  double maximum_raw_volume_jacobian = 0.0;

  bool block_triangular_symplectic_theorem = false;
  bool environment_independent_projection_requires_native_symplecticity = false;
  bool raw_genesis_defect_has_registered_rank = false;
  bool existing_continuous_spectators_are_unchanged = false;
  bool stateless_rng_is_not_dynamical_bath_state = false;
  bool prepared_bath_requires_feedback_or_reset = false;
  bool existing_spectators_close_native_action = false;
  bool environment_feedback_or_reset_required = false;
};

GenesisEnvironmentFeedbackResult analyze_genesis_environment_feedback();

}  // namespace ftd::eft
