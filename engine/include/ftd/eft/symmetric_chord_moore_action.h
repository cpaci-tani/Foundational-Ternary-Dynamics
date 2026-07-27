#pragma once
/**
 * @file symmetric_chord_moore_action.h
 * @brief Observer-only symmetric chord Moore action (FTD-0580).
 */

namespace ftd::eft {

struct SymmetricChordMooreActionResult {
  bool valid = false;
  int shape_samples = 0;
  int path_arms = 0;
  int peierls_coefficient_arms = 0;
  int peierls_potential_samples = 0;
  int cubic_rotation_arms = 0;
  double maximum_partition_residual = 0.0;
  double maximum_first_moment_residual = 0.0;
  double maximum_wrong_sign_residual = 0.0;
  double maximum_raw_continuity_residual = 0.0;
  double maximum_central_continuity_residual = 0.0;
  double maximum_temporal_centering_residual = 0.0;
  double maximum_split_continuity_residual = 0.0;
  double maximum_peierls_law_residual = 0.0;
  double maximum_polarity_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double minimum_peierls_coefficient = 0.0;
  double minimum_peierls_barrier = 0.0;
  bool positive_centered_shape_unique = false;
  bool democratic_shortest_route_exact = false;
  bool common_action_energy_centered = false;
  bool every_peierls_barrier_positive = false;
  bool gapless_mobile_law_derived = false;
  bool production_changed = false;
};

SymmetricChordMooreActionResult analyze_symmetric_chord_moore_action();

}  // namespace ftd::eft

