#pragma once
/**
 * @file native_hodge_reciprocity.h
 * @brief Observer-only reciprocal-force/static-pole audit (FTD-0575).
 *
 * This observer derives the matter-side Hodge potentials of the exact
 * FTD-0574 prescribed-source action and measures their static lattice exchange
 * kernel.  It does not read or modify RenderBridge state.
 */

namespace ftd::eft {

struct NativeHodgeReciprocityResult {
  bool valid = false;

  int infrared_symbol_arms = 0;
  int proper_cubic_rotation_arms = 0;
  int static_charge_arms = 0;
  int static_transverse_current_arms = 0;
  int brillouin_corner_controls = 0;
  int periodic_operator_identity_arms = 0;
  int smooth_path_variation_arms = 0;

  double minimum_static_kernel = 0.0;
  double maximum_static_kernel = 0.0;
  double maximum_kernel_bound_excess = 0.0;
  double maximum_kernel_identity_residual = 0.0;
  double maximum_infrared_error = 0.0;
  double maximum_charge_response_residual = 0.0;
  double maximum_current_response_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double maximum_corner_response = 0.0;
  double maximum_divergence_of_b_residual = 0.0;
  double maximum_faraday_residual = 0.0;
  double maximum_interaction_rewrite_residual = 0.0;
  double maximum_path_variation_residual = 0.0;
  double maximum_magnetic_scalar_work = 0.0;
  double largest_same_polarity_cross_energy = 0.0;
  double smallest_opposite_polarity_cross_energy = 0.0;
  double maximum_soft_residue = 0.0;
  double minimum_soft_residue = 0.0;

  bool hodge_potentials_rewrite_interaction = false;
  bool lorentz_form_path_variation = false;
  bool homogeneous_identities_exact = false;
  bool static_charge_pole_canceled = false;
  bool static_current_pole_canceled = false;
  bool same_polarity_static_interaction_attractive = false;
  bool soft_radiative_residue_quadratic = false;
  bool reciprocal_force_is_coulomb_electromagnetism = false;
  bool exact_finite_step_total_energy_derived = false;
  bool mobile_manifested_solution_derived = false;
  bool production_changed = false;
};

NativeHodgeReciprocityResult analyze_native_hodge_reciprocity();

}  // namespace ftd::eft
