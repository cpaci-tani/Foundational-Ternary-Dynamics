#pragma once
/**
 * @file finite_rigid_moore_carrier_obstruction.h
 * @brief Observer-only finite rigid-carrier obstruction (FTD-0579).
 */

#include "ftd/lattice.h"

namespace ftd::eft {

struct FiniteRigidMooreCarrierResult {
  bool valid = false;

  int profile_count = 0;
  int centering_arms = 0;
  int peierls_coefficient_arms = 0;
  int peierls_potential_samples = 0;
  int binomial_scaling_arms = 0;

  double maximum_direct_fourier_centering_residual = 0.0;
  double maximum_axial_centering_norm2 = 0.0;
  double minimum_diagonal_centering_norm2 = 0.0;
  double minimum_peierls_coefficient = 0.0;
  double minimum_peierls_barrier = 0.0;
  double maximum_peierls_law_residual = 0.0;
  double maximum_polarity_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double maximum_binomial_centering_residual = 0.0;
  double minimum_binomial_scaled_index_at_max_order = 0.0;
  double maximum_binomial_scaled_index_at_max_order = 0.0;

  bool laurent_factorization_exact = false;
  bool finite_diagonal_centering_cure_exists = false;
  bool finite_rigid_peierls_cure_exists = false;
  bool every_registered_diagonal_mismatch_positive = false;
  bool every_registered_peierls_barrier_positive = false;
  bool binomial_suppression_only = false;
  bool extended_native_carrier_derived = false;
  bool production_changed = false;
};

FiniteRigidMooreCarrierResult
analyze_finite_rigid_moore_carrier_obstruction();

}  // namespace ftd::eft
