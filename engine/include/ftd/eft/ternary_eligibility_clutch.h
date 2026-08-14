#pragma once

/**
 * @file ternary_eligibility_clutch.h
 * @brief FTD-0867 isolated ternary clutch and one-shot handshake witness.
 *
 * The retained latch value s in {-1,0,+1} supplies the minimum even
 * hold/exchange clutch epsilon=s^2.  On the registered prepared domain, one
 * FTD-0865 harmonic cycle transfers a signed canonical matter mode into the
 * signal.  A separate FTD-0863 phase reference decodes the sign after a
 * gate-zero request to release the local latch.
 *
 * This reduced composition does not implement microscopic latch acquisition,
 * autonomous acknowledgement/reset, a bath state, clock synchronization,
 * cubic-lattice production coupling, or a G* cadence.
 */

#include "ftd/eft/catalytic_phase_reference.h"
#include "ftd/eft/clock_gated_hamiltonian_exchange.h"

#include <cstdint>

namespace ftd::eft {

enum class TernaryEligibilityClutchStatus : std::uint8_t {
  Valid = 0,
  InvalidLatch,
  InvalidOrientationReference,
  InvalidTolerance,
  InvalidHoldPreparation,
  InvalidActiveMatter,
  InvalidActiveSignal,
  InvalidMatterOrientation,
  ClockCycleRejected,
  SignalDecoderRejected,
  EventRecoveryFailed,
  NonFiniteOutput,
};

struct TernaryEligibilityClutchInput {
  std::int8_t latch = 0;
  CanonicalCarrierPair orientation_reference;
  ClockGatedHamiltonianState clock_state;
  ClockGatedHamiltonianParameters clock_parameters;
};

struct TernaryEligibilityClutchResult {
  TernaryEligibilityClutchStatus status =
      TernaryEligibilityClutchStatus::InvalidLatch;
  std::int8_t latch_before = 0;
  std::int8_t requested_latch_after = 0;
  std::int8_t decoded_latch = 0;
  RecordPortEligibility derived_eligibility = RecordPortEligibility::Hold;
  ClockGatedHamiltonianResult clock_cycle;
  CatalyticSignalReadout matter_readout_before;
  CatalyticSignalReadout signal_readout_after;
  double event_energy_before = 0.0;
  double decoded_event_energy = 0.0;
  double oriented_area_after = 0.0;
  double endpoint_gate_value = 0.0;
  double clutch_switch_work_at_release = 0.0;
  bool latch_square_is_eligibility = false;
  bool clutch_release_requested = false;
  bool clutch_release_at_gate_zero = false;
  bool declared_event_recoverable_after_requested_reset = false;
  bool second_active_cycle_would_undo_exchange = false;
  bool microscopic_latch_reset_supplied = false;
  bool autonomous_acknowledgement_supplied = false;
  bool clock_synchronization_supplied = false;
  bool cubic_production_coupling_supplied = false;

  bool valid() const { return status == TernaryEligibilityClutchStatus::Valid; }
};

/** Execute the registered reduced one-shot latch-to-signal handshake. */
TernaryEligibilityClutchResult execute_ternary_eligibility_handshake(
    const TernaryEligibilityClutchInput& input);

}  // namespace ftd::eft
