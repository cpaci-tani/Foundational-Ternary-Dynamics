#pragma once
/**
 * @file hop_source_multipole_hierarchy.h
 * @brief Observer-only slow-hop source multipole hierarchy (FTD-0561).
 */

#include <complex>
#include <string>
#include <vector>

namespace ftd::eft {

struct HopSourceMultipoleArm {
  bool valid = false;
  std::string profile;
  int period = 0;
  int axis = 0;
  int polarity = 0;
  int leading_moment_order = -1;
  long long leading_moment = 0;
  double root = 0.0;
  double phase = 0.0;
  double denominator_residual = 0.0;
  std::complex<double> form_factor{};
  std::complex<double> closed_form_factor{};
  double form_factor_residual = 0.0;
  double normalized_forcing = 0.0;
  double asymptotic_coefficient = 0.0;
  double normalized_asymptotic_ratio = 0.0;
};

struct HopSourceMultipoleHierarchyResult {
  bool valid = false;
  bool finite_source_multipole_theorem = false;
  bool charged_extension_retains_t2_forcing = false;
  bool neutrality_raises_suppression_order = false;
  bool axial_interval_cancellation_requires_plane_neutrality = false;
  bool axial_cancellation_is_not_full_surface_cancellation = false;
  double c2 = 0.0;
  double maximum_denominator_residual = 0.0;
  double maximum_form_factor_residual = 0.0;
  double minimum_normalized_forcing = 0.0;
  double maximum_polarity_mirror_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double point_t256_error = 0.0;
  double pair_t256_error = 0.0;
  double dipole_t256_error = 0.0;
  double quadrupole_t256_error = 0.0;
  double same_plane_axial_residual = 0.0;
  double same_plane_oblique_amplitude = 0.0;
  std::vector<HopSourceMultipoleArm> arms;
};

HopSourceMultipoleHierarchyResult analyze_hop_source_multipole_hierarchy(
    double c2);

}  // namespace ftd::eft
