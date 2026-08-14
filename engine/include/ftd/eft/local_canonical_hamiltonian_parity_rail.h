#pragma once

/**
 * @file local_canonical_hamiltonian_parity_rail.h
 * @brief FTD-0875 local canonical Hamiltonian lift of the parity rail.
 *
 * Each rail site carries one imposed canonical pair (q,p). A common harmonic
 * reference clock drives the disjoint tick-parity bond generators
 * q_j p_k - q_k p_j through an exact quarter-turn. The endpoint reproduces
 * the FTD-0874 discrete rail and books local carrier-energy transfer plus the
 * transient clock-action ledger. This remains an isolated selected reference;
 * it is not production substrate coupling or a G* actuator.
 */

#include "ftd/eft/alternating_oriented_ternary_parity_rail.h"
#include "ftd/eft/oriented_ternary_quarter_turn.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace ftd::eft {

enum class LocalCanonicalHamiltonianParityRailStatus : std::uint8_t {
  Valid = 0,
  InvalidRailLength,
  InvalidSiteState,
  InvalidAmplitude,
  InvalidClockFrequency,
  InvalidReferenceAction,
  InvalidReferencePhase,
  InvalidTolerance,
  InsufficientReferenceReserve,
  NonFiniteOutput,
};

struct CanonicalParityRailSite {
  double q = 0.0;
  double p = 0.0;
};

struct CanonicalParityRailBondLedger {
  std::size_t left = 0;
  std::size_t right = 0;
  double oriented_generator = 0.0;
  double left_energy_before = 0.0;
  double right_energy_before = 0.0;
  double left_energy_after = 0.0;
  double right_energy_after = 0.0;
  double integrated_current_left_to_right = 0.0;
  bool bond_energy_conserved = false;
};

struct LocalCanonicalHamiltonianParityRailInput {
  std::vector<CanonicalParityRailSite> sites;
  std::uint64_t global_tick = 0;
  TernaryQuarterTurnOrientation orientation =
      TernaryQuarterTurnOrientation::Forward;
  double ternary_amplitude = 1.0;
  double clock_frequency = 1.0;
  double reference_action = 1.0;
  double reference_phase = 0.0;
  double tolerance = 1e-12;
};

struct LocalCanonicalHamiltonianParityRailResult {
  LocalCanonicalHamiltonianParityRailStatus status =
      LocalCanonicalHamiltonianParityRailStatus::InvalidRailLength;
  std::uint64_t global_tick = 0;
  TernaryQuarterTurnOrientation orientation =
      TernaryQuarterTurnOrientation::Forward;
  std::vector<CanonicalParityRailSite> before;
  std::vector<CanonicalParityRailSite> after;
  std::vector<CanonicalParityRailSite> recovered;
  std::vector<std::pair<std::size_t, std::size_t>> active_bonds;
  std::vector<CanonicalParityRailBondLedger> bond_ledgers;
  std::vector<std::int8_t> actual_labels_before;
  std::vector<std::int8_t> actual_labels_after;
  double cycle_duration = 0.0;
  double spatial_pulse_angle = 0.0;
  double carrier_norm_before = 0.0;
  double carrier_norm_after = 0.0;
  double oriented_bond_generator = 0.0;
  double imposed_record_energy_scale = 0.0;
  double carrier_energy_before = 0.0;
  double carrier_energy_after = 0.0;
  double carrier_plus_interaction_lower_bound = 0.0;
  double reference_action_before = 0.0;
  double minimum_reference_action = 0.0;
  double maximum_reference_action = 0.0;
  double reference_action_after = 0.0;
  double maximum_clock_action_excursion = 0.0;
  double maximum_reference_energy_exchange = 0.0;
  double maximum_interaction_energy_magnitude = 0.0;
  double gate_zero_switch_work = 0.0;
  double antiphase_switch_work_magnitude = 0.0;
  double endpoint_total_energy_before = 0.0;
  double endpoint_total_energy_after = 0.0;
  double endpoint_energy_residual = 0.0;
  bool disjoint_matching = false;
  bool exact_inverse_verified = false;
  bool carrier_norm_preserved = false;
  bool total_carrier_energy_conserved = false;
  bool local_bond_energy_conserved = false;
  bool local_antisymmetric_current_supplied = false;
  bool positive_carrier_interaction_bound = false;
  bool actual_ternary_section = false;
  bool actual_section_returns_to_section = false;
  bool actual_section_matches_discrete_rail = false;
  bool actual_section_clock_backreaction_zero = false;
  bool nontrivial_transport_with_zero_generator_value = false;
  bool scalar_common_form_boundary_global = true;
  std::size_t minimum_registered_local_carrier_dimension_per_site = 2;
  bool common_harmonic_clock_selected = true;
  bool continuous_subtick_ontology_claimed = false;
  bool native_doublet_formation_supplied = false;
  bool native_record_energy_scale_derived = false;
  bool production_coupling_supplied = false;
  bool native_gstar_synchronization_supplied = false;

  bool valid() const {
    return status == LocalCanonicalHamiltonianParityRailStatus::Valid;
  }
};

/** Signed anti-diagonal common form for an even scalar rail; zero otherwise. */
int scalar_boundary_global_symplectic_entry(
    std::size_t rail_length,
    std::size_t row,
    std::size_t column);

/** Evaluate one exact common-clock Hamiltonian cycle on one parity matching. */
LocalCanonicalHamiltonianParityRailResult
evolve_local_canonical_hamiltonian_parity_rail_cycle(
    const LocalCanonicalHamiltonianParityRailInput& input);

}  // namespace ftd::eft
