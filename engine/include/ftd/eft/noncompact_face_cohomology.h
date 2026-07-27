#pragma once
/**
 * @file noncompact_face_cohomology.h
 * @brief Observer-only cohomology/local-defect gate for the matched complex
 *        (FTD-0583).
 */

namespace ftd::eft {

struct NoncompactFaceCohomologyResult {
  bool valid = false;
  int fourier_mode_arms = 0;
  int zero_momentum_mode_arms = 0;
  int nonzero_momentum_mode_arms = 0;
  int fourier_rank_mismatches = 0;
  int betti_volume_mismatches = 0;
  int harmonic_arms = 0;
  int localized_curl_arms = 0;
  int contraction_samples = 0;
  int charge_scaling_arms = 0;
  int cubic_rotation_arms = 0;
  int betti_0 = 0;
  int betti_1 = 0;
  int betti_2 = 0;
  int betti_3 = 0;
  int minimum_localized_support = 0;
  int maximum_localized_support = 0;
  double maximum_symbol_complex_residual = 0.0;
  double maximum_divergence_of_curl = 0.0;
  double maximum_curl_plane_flux = 0.0;
  double maximum_harmonic_plane_residual = 0.0;
  double maximum_harmonic_flux_change_under_curl = 0.0;
  double maximum_contraction_divergence = 0.0;
  double maximum_contraction_harmonic_flux = 0.0;
  double maximum_contraction_energy_residual = 0.0;
  double maximum_contraction_curl_residual = 0.0;
  int maximum_contraction_support_excess = 0;
  double minimum_nonzero_localized_energy = 0.0;
  double maximum_periodic_charge_sum = 0.0;
  double maximum_charge_scaling_residual = 0.0;
  double maximum_off_source_divergence = 0.0;
  double maximum_surface_telescope_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double maximum_curl_covariance_residual = 0.0;
  double maximum_rotated_divergence = 0.0;
  double maximum_rotation_energy_residual = 0.0;
  double maximum_rotated_harmonic_plane_residual = 0.0;
  double maximum_harmonic_rotation_residual = 0.0;
  bool periodic_complex_exact_off_zero_mode = false;
  bool face_cohomology_is_three_global_real_fluxes = false;
  bool localized_zero_harmonic_fields_contractible = false;
  bool real_gauss_charge_continuously_scalable = false;
  bool localized_protected_carrier_in_current_variables = false;
  bool compact_u1_structure_derived = false;
  bool production_changed = false;
};

NoncompactFaceCohomologyResult analyze_noncompact_face_cohomology();

}  // namespace ftd::eft
