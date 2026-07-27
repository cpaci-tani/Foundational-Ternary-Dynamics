#pragma once
/**
 * @file full_surface_source_obstruction.h
 * @brief Observer-only finite-source full-resonance obstruction (FTD-0562).
 */

#include <array>
#include <complex>
#include <string>
#include <vector>

namespace ftd::eft {

struct FullSurfaceSourceArm {
  bool valid = false;
  bool root_bracketed = false;
  bool leading_witness = false;
  std::string profile;
  int period = 0;
  int direction_index = 0;
  int axis = 0;
  int polarity = 0;
  int leading_order = -1;
  std::array<double,3> direction{};
  std::array<double,3> momentum{};
  double root_radius = 0.0;
  double omega = 0.0;
  double denominator_residual = 0.0;
  double scaled_radial_derivative = 0.0;
  std::complex<double> form_factor{};
  std::complex<double> leading_polynomial{};
  std::complex<double> floquet_coefficient{};
  double source_forcing_over_gc = 0.0;
  double asymptotic_coefficient_over_gc = 0.0;
  double asymptotic_ratio = 0.0;
  double radius_first_correction_residual = 0.0;
};

struct FullSurfaceSourceObstructionResult {
  bool valid = false;
  bool full_direction_slow_branch_exists = false;
  bool finite_source_form_factor_is_analytic = false;
  bool lowest_homogeneous_moment_is_decisive = false;
  bool finite_rigid_universal_cancellation_closed = false;
  bool square_summable_linear_dressing_closed_for_slow_hops = false;
  bool nonlinear_deforming_carrier_remains_open = false;
  double c2 = 0.0;
  int witness_groups = 0;
  int expected_witness_groups = 0;
  double maximum_denominator_residual = 0.0;
  double minimum_scaled_radial_derivative = 0.0;
  double maximum_polarity_mirror_residual = 0.0;
  double maximum_cyclic_covariance_residual = 0.0;
  double maximum_t512_radius_correction_residual = 0.0;
  double maximum_t512_asymptotic_error = 0.0;
  double minimum_witness_scaled_forcing = 0.0;
  std::vector<FullSurfaceSourceArm> arms;
};

FullSurfaceSourceObstructionResult analyze_full_surface_source_obstruction(
    double c2);

}  // namespace ftd::eft
