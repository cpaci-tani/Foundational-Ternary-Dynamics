#pragma once

/**
 * @file orientation_gauss_independence.h
 * @brief Observer-only orientation-degree/Gauss-flux independence (FTD-0564).
 */

#include <string>
#include <vector>

namespace ftd::eft {

struct OrientationGaussArm {
  bool valid = false;
  std::string family;
  double amplitude = 0.0;
  int polarity = 0;
  int cyclic_rotation = 0;
  double minimum_field_magnitude = 0.0;
  double orientation_degree = 0.0;
  double gauss_flux = 0.0;
  double expected_degree = 0.0;
  double expected_flux = 0.0;
  double degree_residual = 0.0;
  double flux_residual = 0.0;
};

struct OrientationGaussIndependenceResult {
  bool valid = false;
  bool degree_does_not_determine_flux = false;
  bool flux_does_not_determine_degree = false;
  bool amplitude_rescaling_separates_observables = false;
  bool polarity_mirror_exact = false;
  bool cubic_covariance_exact = false;
  bool periodic_divergence_image_is_zero_sum = false;
  bool topology_alone_charge_magnitude_closed = false;
  bool topological_core_with_action_remains_open = false;
  int rank_witnesses = 0;
  int expected_rank_witnesses = 2;
  double maximum_degree_residual = 0.0;
  double maximum_flux_residual = 0.0;
  double maximum_equal_flux_residual = 0.0;
  double maximum_scale_linearity_residual = 0.0;
  double maximum_polarity_mirror_residual = 0.0;
  double maximum_cyclic_covariance_residual = 0.0;
  double maximum_tree_routing_residual = 0.0;
  std::vector<OrientationGaussArm> arms;
};

OrientationGaussIndependenceResult
analyze_orientation_gauss_independence();

}  // namespace ftd::eft
