#pragma once
/**
 * @file genesis_reservoir_dilation.h
 * @brief Observer-only reversible-dilation analysis for the frozen
 *        genesis/evaporation event kernel (FTD-0569).
 */

#include <cstdint>

#include "ftd/voxel.h"

namespace ftd::eft {

struct GenesisEventState {
  std::int8_t state = 0;
  Vec3 flux{};
  Vec3 wave{};
  std::int32_t particle_id = -1;
  std::int8_t spin = 0;
  std::int8_t color = 0;
};

struct GenesisMapResult {
  bool valid = false;
  GenesisEventState state{};
  double excess = 0.0;
  double acceptance_probability = 0.0;
  double field_withdrawal = 0.0;
};

/// Exact continuous drain/state assignment on a canonical void site. The
/// caller supplies the spin selected from the production pre-write snapshot;
/// this observer does not re-evaluate the nonlocal curl rule.
GenesisMapResult accepted_single_genesis(
    const GenesisEventState& before,
    std::int8_t polarity,
    std::int8_t manifested_spin,
    double genesis_threshold,
    double manifestation_scale,
    double kinetic_drain);

/// Conditional inverse of accepted_single_genesis for 0 <= drain < 1.
GenesisMapResult invert_accepted_single_genesis(
    const GenesisEventState& after,
    double genesis_threshold,
    double manifestation_scale,
    double kinetic_drain);

/// Frozen projected evaporation assignment. Continuous fields are untouched.
GenesisEventState projected_evaporation(const GenesisEventState& before);

/// Exact field/wave energy removed by accepted single genesis.
double genesis_field_withdrawal(
    double genesis_threshold,
    double excess,
    double wave_magnitude_squared,
    double kinetic_drain);

struct BernoulliDilationResult {
  bool valid = false;
  int branch = -1;  // 1=accepted, 0=rejected
  double probability = 0.0;
  double phase_before = 0.0;
  double phase_after = 0.0;
  double recovered_phase = 0.0;
};

/// Inverse-CDF/baker-map dilation of one Bernoulli draw.
BernoulliDilationResult dilate_bernoulli_phase(double phase, double probability);

/// Recover the preimage phase from the retained branch and future phase.
double recover_bernoulli_phase(
    int branch, double future_phase, double probability);

struct GenesisReservoirDilationResult {
  bool valid = false;
  int accepted_single_arms = 0;
  int bernoulli_arms = 0;
  int history_depth = 0;
  std::uint64_t erased_preimages_at_depth = 0;
  std::uint64_t minimum_history_bits_at_depth = 0;

  double maximum_genesis_inverse_residual = 0.0;
  double maximum_bernoulli_inverse_residual = 0.0;
  double maximum_history_inverse_residual = 0.0;
  double maximum_withdrawal_residual = 0.0;
  double maximum_withdrawal_slope_residual = 0.0;
  double maximum_evaporation_flux_distance_residual = 0.0;
  double maximum_evaporation_wave_distance_residual = 0.0;
  double minimum_evaporation_composition_flux_distance = 0.0;
  double withdrawal_span = 0.0;

  bool accepted_genesis_conditionally_invertible = false;
  bool unit_drain_has_wave_collision = false;
  bool one_step_bernoulli_dilation_exact = false;
  bool erased_trials_require_unbounded_history = false;
  bool evaporation_is_not_genesis_inverse = false;
  bool production_pair_violates_detailed_balance = false;
  bool continuous_energy_payload_required = false;
  bool dual_and_single_energy_exchange_differ = false;
  bool finite_local_reversible_production_dilation = false;
  bool one_event_dilation_open_system_only = false;
};

GenesisReservoirDilationResult analyze_genesis_reservoir_dilation();

}  // namespace ftd::eft
