#pragma once
/**
 * @file ternary_block_bipole_peierls.h
 * @brief Exact integer-site extended-carrier spectral observer (FTD-0621).
 */

#include <array>
#include <cstdint>

namespace ftd::eft {

struct TernaryBlockBipolePeierlsResult {
  bool valid = false;
  bool support_does_not_wrap = false;
  bool exactly_neutral = false;
  int L = 0;
  int width = 0;
  int orientation_axis = -1;
  std::int64_t positive_sites = 0;
  std::int64_t negative_sites = 0;
  std::int64_t occupied_sites = 0;
  double beta = 0.0;
  double energy = 0.0;
  std::array<double, 3> peierls_coefficient{};
  std::array<double, 3> half_cell_barrier{};
  std::array<double, 3> pinning_index{};
  std::array<double, 3> spectral_average{};
  std::array<double, 3> spectral_identity_residual{};
  double structure_factor_relative_residual = 0.0;
  double maximum_identity_residual = 0.0;
};

/**
 * Evaluate the exact finite-volume quadratic-coat spectrum of two adjacent
 * w^3 ternary blocks with opposite polarity. This observer does not construct
 * fractional site states and never mutates production state.
 */
TernaryBlockBipolePeierlsResult evaluate_ternary_block_bipole_peierls(
    int L, int width, int orientation_axis, double beta);

}  // namespace ftd::eft

