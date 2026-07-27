#pragma once

/**
 * @file native_gauss_monopole_dichotomy.h
 * @brief Observer-only Gauss-monopole/mobile-dressing dichotomy (FTD-0563).
 */

#include <array>
#include <complex>
#include <string>
#include <vector>

namespace ftd::eft {

struct GaussMonopoleArm {
  bool valid = false;
  bool leading_witness = false;
  std::string profile;
  int volume = 0;
  int direction_index = 0;
  int axis = 0;
  int polarity = 0;
  int total_polarity = 0;
  int leading_order = -1;
  std::array<double,3> momentum{};
  double kappa = 0.0;
  double face_laplacian = 0.0;
  std::complex<double> form_factor{};
  std::array<std::complex<double>,3> longitudinal_face_field{};
  double face_gauss_identity_residual = 0.0;
  double monopole_estimator = 0.0;
  std::complex<double> leading_polynomial{};
  double asymptotic_ratio = 0.0;
};

struct NativeGaussMonopoleDichotomyResult {
  bool valid = false;
  bool periodic_divergence_zero_sum = false;
  bool production_zero_mode_subtracted = false;
  bool matched_non_neutral_rejected = false;
  bool matched_neutral_accepted = false;
  bool infinite_volume_monopole_equals_net_polarity = false;
  bool neutral_finite_profile_has_no_monopole = false;
  bool solenoidal_dressing_cannot_change_monopole = false;
  bool native_ir_susceptibility_is_finite = false;
  bool fixed_finite_linear_charged_carrier_closed = false;
  bool nonlinear_topological_effective_charge_remains_open = false;
  int witness_groups = 0;
  int expected_witness_groups = 0;
  int monotone_neutral_witnesses = 0;
  long long maximum_zero_mode_numerator_sum = 0;
  double native_ir_susceptibility = 0.0;
  double periodic_telescope_residual = 0.0;
  double matched_neutral_gauss_residual = 0.0;
  double maximum_curl_divergence = 0.0;
  double maximum_closed_surface_flux_change = 0.0;
  double maximum_face_gauss_identity_residual = 0.0;
  double maximum_point_monopole_error = 0.0;
  double maximum_l256_neutral_monopole_estimator = 0.0;
  double maximum_l256_asymptotic_error = 0.0;
  double maximum_polarity_mirror_residual = 0.0;
  double maximum_cyclic_covariance_residual = 0.0;
  std::vector<GaussMonopoleArm> arms;
};

NativeGaussMonopoleDichotomyResult
analyze_native_gauss_monopole_dichotomy();

}  // namespace ftd::eft
