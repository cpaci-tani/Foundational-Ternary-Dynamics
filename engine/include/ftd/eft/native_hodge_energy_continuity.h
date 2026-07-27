#pragma once
/**
 * @file native_hodge_energy_continuity.h
 * @brief Observer-only native Hodge energy/continuity audit (FTD-0576).
 *
 * Derives the exact driven work coordinate of the frozen kick-drift field
 * map, verifies the conditional common-energy identity, and tests whether a
 * cardinal hop can supply the required central-difference current locally.
 * No RenderBridge state or production rule is read or changed.
 */

namespace ftd::eft {

struct NativeHodgeEnergyContinuityResult {
  bool valid = false;

  int mode_work_arms = 0;
  int full_field_work_arms = 0;
  int conditional_energy_arms = 0;
  int axial_cardinal_hop_arms = 0;
  int polarity_checks = 0;
  int proper_cubic_rotation_arms = 0;

  double maximum_mode_work_residual = 0.0;
  double maximum_full_field_work_residual = 0.0;
  double maximum_half_step_coordinate_residual = 0.0;
  double maximum_conditional_continuity_residual = 0.0;
  double maximum_conditional_field_work_residual = 0.0;
  double maximum_conditional_interaction_residual = 0.0;
  double maximum_conditional_total_energy_residual = 0.0;
  double maximum_odd_volume_current_residual = 0.0;
  double minimum_even_checkerboard_witness = 0.0;
  double minimum_odd_support_fraction = 0.0;
  int minimum_odd_support_sites = 0;
  int maximum_odd_support_sites = 0;
  int minimum_odd_support_radius = 0;
  int maximum_odd_support_radius = 0;
  double maximum_cubic_covariance_residual = 0.0;
  double face_to_site_checkerboard_defect = 0.0;

  bool driven_tick_work_identity_exact = false;
  bool half_step_coordinate_unique = false;
  bool constant_source_affine_invariant_exact = false;
  bool conditional_hodge_total_energy_exact = false;
  bool even_cardinal_hop_central_current_exists = false;
  bool odd_cardinal_hop_current_is_box_spanning = false;
  bool finite_range_cardinal_hop_current_exists = false;
  bool finite_range_face_to_site_projection_exists = false;
  bool additional_staggered_or_nonlocal_structure_required = false;
  bool production_changed = false;
};

NativeHodgeEnergyContinuityResult analyze_native_hodge_energy_continuity();

}  // namespace ftd::eft
