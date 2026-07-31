#pragma once
/**
 * @file connected_reservoir_decomposition.h
 * @brief Exact complete-state perturbation reservoir ledger (FTD-0673).
 */

#include "ftd/eft/connected_moore_block_action.h"

#include <cstddef>
#include <vector>

namespace ftd::eft {

struct ConnectedTangentMode {
  double omega = 0.0;
  std::vector<double> vector;
};

struct ConnectedReservoirDecomposition {
  int L = 0;
  std::size_t constituent_count = 0;
  std::size_t mode_count = 0;
  std::size_t target_mode_count = 0;
  double interaction_scale = 0.0;

  double kinetic_difference = 0.0;
  double binding_difference = 0.0;
  double exact_matter_difference = 0.0;
  double field_difference = 0.0;
  double total_difference = 0.0;

  double target_mode_energy = 0.0;
  double other_mode_energy = 0.0;
  double total_mode_energy = 0.0;
  double matter_nonlinear_remainder = 0.0;

  double dynamic_field_energy = 0.0;
  double field_interference = 0.0;

  double mode_orthonormality_residual = 0.0;
  double field_decomposition_residual = 0.0;
  double matter_decomposition_residual = 0.0;
  double complete_decomposition_residual = 0.0;

  std::vector<double> modal_positions;
  std::vector<double> modal_momenta;
  std::vector<double> modal_energies;
  bool valid = false;
};

/**
 * Decompose excited-minus-control energy without double-counting:
 *
 *   Delta E = E_target + E_other + R_matter
 *             + H(delta field) + I(control,delta field).
 *
 * The tangent modes must be a complete mass-orthonormal basis.  R_matter is
 * the exact nonlinear remainder between the full relativistic-plus-binding
 * matter difference and the tangent modal quadratic.  This is an observer;
 * it does not alter either state or the connected action.
 */
ConnectedReservoirDecomposition evaluate_connected_reservoir_decomposition(
    const ConnectedMooreBlockState& control,
    const ConnectedMooreBlockState& excited,
    const std::vector<ConnectedTangentMode>& modes,
    const std::vector<std::size_t>& target_modes,
    double interaction_scale,
    const ConnectedMooreBlockOptions& options = {},
    double tolerance = 1e-10);

}  // namespace ftd::eft
