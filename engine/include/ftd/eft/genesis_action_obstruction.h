#pragma once

/**
 * @file genesis_action_obstruction.h
 * @brief Exact observer for the genesis amplitude/common-action gate (FTD-0567).
 */

#include <vector>

namespace ftd::eft {

struct GenesisActionArm {
  bool valid = false;
  bool dual_substrate = false;
  double excess = 0.0;
  double wave_magnitude_squared = 0.0;
  double kinetic_drain = 0.0;
  int polarity = 0;
  double acceptance_probability = 0.0;
  double flux_magnitude_before = 0.0;
  double flux_magnitude_after = 0.0;
  double flux_energy_withdrawn = 0.0;
  double wave_energy_withdrawn = 0.0;
  double amplitude_residual = 0.0;
  double flux_energy_residual = 0.0;
  double wave_energy_residual = 0.0;
};

struct GenesisActionObstructionResult {
  bool valid = false;
  bool single_map_preserves_overshoot = false;
  bool no_post_genesis_amplitude_lock = false;
  bool no_fixed_ternary_energy_quantum = false;
  bool acceptance_conditioning_does_not_lock = false;
  bool dual_branch_has_no_latent_heat_payment = false;
  bool evaporation_signed_preimages_collapse = false;
  bool written_action_cannot_generate_magnitude_gate = false;
  bool written_action_zero_divergence_polarity_degenerate = false;
  bool frozen_common_action_route_closed = false;
  bool extended_reservoir_or_open_system_remains_open = false;
  int distinct_single_post_amplitudes = 0;
  int expected_distinct_single_post_amplitudes = 4;
  double maximum_amplitude_residual = 0.0;
  double maximum_flux_energy_residual = 0.0;
  double maximum_wave_energy_residual = 0.0;
  double maximum_polarity_scalar_residual = 0.0;
  double maximum_action_threshold_residual = 0.0;
  double fixed_quantum_energy_spread = 0.0;
  std::vector<GenesisActionArm> arms;
};

GenesisActionObstructionResult analyze_genesis_action_obstruction();

}  // namespace ftd::eft
