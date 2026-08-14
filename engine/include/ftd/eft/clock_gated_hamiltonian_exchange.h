#pragma once

/**
 * @file clock_gated_hamiltonian_exchange.h
 * @brief FTD-0865 isolated autonomous Hamiltonian exchange witness.
 *
 * The FTD-0856 scalar swap is lifted to two full canonical modes.  A harmonic
 * action-angle reference supplies an autonomous phase pulse; over one complete
 * cycle the registered winding is exactly identity (hold) or a matter/signal
 * mode swap (exchange).  Reference action is borrowed transiently and returned
 * at the endpoint, with a strict reserve bound.
 *
 * This imposed reference Hamiltonian is not a dynamic eligibility mechanism,
 * a quartic G* controller, production coupling, or a Voxel tick phase.
 */

#include "ftd/eft/phase_referenced_action_rail.h"
#include "ftd/eft/reciprocal_record_port.h"

#include <cstdint>

namespace ftd::eft {

enum class ClockGatedHamiltonianStatus : std::uint8_t {
  Valid = 0,
  InvalidClockFrequency,
  InvalidCommonFrequency,
  InvalidCoupling,
  InvalidReferencePhase,
  InvalidReferenceAction,
  InvalidMatterMode,
  InvalidSignalMode,
  InvalidTolerance,
  InvalidEligibility,
  InsufficientReferenceReserve,
  NonFiniteOutput,
};

struct ClockGatedHamiltonianState {
  double reference_phase = 0.0;
  double reference_action = 0.0;
  CanonicalCarrierPair matter;
  CanonicalCarrierPair signal;
};

struct ClockGatedHamiltonianParameters {
  double clock_frequency = 0.0;
  double common_frequency = 0.0;
  double coupling = 0.0;
  RecordPortEligibility eligibility = RecordPortEligibility::Hold;
  double tolerance = 1e-12;
};

struct ClockGatedHamiltonianResult {
  ClockGatedHamiltonianStatus status =
      ClockGatedHamiltonianStatus::InvalidClockFrequency;
  ClockGatedHamiltonianState before;
  ClockGatedHamiltonianState after;
  CanonicalCarrierPair common_before;
  CanonicalCarrierPair common_after;
  CanonicalCarrierPair relative_before;
  CanonicalCarrierPair relative_after;
  double common_action = 0.0;
  double relative_action = 0.0;
  double mode_action_before = 0.0;
  double mode_action_after = 0.0;
  double common_phase = 0.0;
  double relative_extra_phase = 0.0;
  double minimum_reference_action = 0.0;
  double reserve_margin = 0.0;
  double maximum_interaction_energy = 0.0;
  double maximum_reference_energy_loan = 0.0;
  double endpoint_energy_before = 0.0;
  double endpoint_energy_after = 0.0;
  double endpoint_energy_residual = 0.0;
  bool common_winding_compliant = false;
  bool branch_winding_compliant = false;
  bool exact_hold = false;
  bool exact_swap = false;
  bool dynamic_eligibility_supplied = false;
  bool quartic_load_blind_controller_established = false;

  bool valid() const { return status == ClockGatedHamiltonianStatus::Valid; }
};

/**
 * Evolve one complete phase cycle of the imposed harmonic reference.
 *
 * The initial phase must lie at a zero of g(theta)=1-cos(theta).  The function
 * evaluates the exact Hamiltonian flow, not a numerical integrator.
 */
ClockGatedHamiltonianResult evolve_clock_gated_hamiltonian_cycle(
    const ClockGatedHamiltonianState& state,
    const ClockGatedHamiltonianParameters& parameters);

}  // namespace ftd::eft
