#pragma once
/**
 * @file minimal_moore_compatibility_coat.h
 * @brief Observer-only local bridge from face to central continuity (FTD-0577).
 *
 * Primitive manifestation remains site-valued and ternary.  This record
 * applies the unique normalized symmetric radius-one checkerboard-cancelling
 * filter in each axis to the FTD-0478 coupling representation, then converts
 * its oriented face current to a finite-range site-centered current.
 */

#include "ftd/eft/face_current_segment.h"

#include <vector>

namespace ftd::eft {

struct MooreCoatedCurrent {
  int L = 0;
  int charge = 0;
  std::vector<double> rho_before;
  std::vector<double> rho_after;
  std::vector<Vec3> central_current;

  int rho_before_support = 0;
  int rho_after_support = 0;
  int current_support = 0;
  double partition_residual = 0.0;
  double first_moment_residual = 0.0;
  double wrong_sign_weight_residual = 0.0;
  double central_continuity_residual = 0.0;
  bool finite_range = false;
  bool valid = false;

  int total_sites() const { return L * L * L; }
  int index(int x, int y, int z) const;
};

/// Apply B=(T^-1+2+T)/4 in all three axes to the endpoint density and
/// Q_i=(1+T_i^-1)/2 product_(j!=i) B_j K_i to each face-current component.
MooreCoatedCurrent make_minimal_moore_compatibility_coat(
    const FaceCurrentSegment& segment);

double central_current_divergence_at(
    const MooreCoatedCurrent& coated, int x, int y, int z);

double coated_continuity_at(
    const MooreCoatedCurrent& coated, int x, int y, int z);

struct MinimalMooreCompatibilityCoatResult {
  bool valid = false;

  int path_arms = 0;
  int polarity_arms = 0;
  int volume_arms = 0;
  int translation_arms = 0;
  int proper_cubic_rotation_arms = 0;
  int conditional_energy_arms = 0;
  int integer_coat_sites = 0;
  int minimum_local_rho_support = 0;
  int maximum_local_rho_support = 0;
  int minimum_local_current_support = 0;
  int maximum_local_current_support = 0;

  double radius_one_a = 0.0;
  double radius_one_b = 0.0;
  double center_weight = 0.0;
  double face_weight = 0.0;
  double edge_weight = 0.0;
  double corner_weight = 0.0;
  double maximum_filter_equation_residual = 0.0;
  double maximum_partition_residual = 0.0;
  double maximum_first_moment_residual = 0.0;
  double maximum_wrong_sign_weight_residual = 0.0;
  double maximum_central_continuity_residual = 0.0;
  double maximum_translation_covariance_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double maximum_zero_mode_residual = 0.0;
  double maximum_checkerboard_response = 0.0;
  double maximum_conditional_field_work_residual = 0.0;
  double maximum_conditional_interaction_residual = 0.0;
  double maximum_conditional_total_energy_residual = 0.0;
  double integer_center_cardinality_defect = 0.0;

  bool scoped_radius_one_filter_unique = false;
  bool integer_coat_positive_and_normalized = false;
  bool trilinear_moments_preserved = false;
  bool local_central_continuity_exact = false;
  bool local_support_volume_independent = false;
  bool integer_translation_covariant = false;
  bool proper_cubic_covariant = false;
  bool checkerboard_nulls_removed_from_source = false;
  bool conditional_hodge_energy_compatible = false;
  bool coupling_representation_is_cardinal = false;
  bool reciprocal_force_derived = false;
  bool static_coulomb_pole_recovered = false;
  bool mobile_manifested_solution_derived = false;
  bool production_changed = false;
};

MinimalMooreCompatibilityCoatResult
analyze_minimal_moore_compatibility_coat();

}  // namespace ftd::eft
