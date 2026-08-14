#pragma once

/**
 * @file signal_acknowledged_two_stroke_reset.h
 * @brief FTD-0869 isolated signal-acknowledged recursive reset witness.
 *
 * One imposed harmonic supercycle is divided into an exact exchange stroke
 * and a selected nonsmooth reset stroke.  The completed local signal is the
 * acknowledgement token; after finite-time cusp reset it is handed to an
 * initially empty output port, returning the local reference state to ready.
 *
 * This interface evaluates exact endpoint maps and ledgers.  It does not
 * implement a microscopic bath, protected cubic transport, production latch,
 * quartic compensation, or native G* synchronization.
 */

#include "ftd/eft/ternary_eligibility_clutch.h"

#include <cstdint>

namespace ftd::eft {

enum class SignalAcknowledgedResetStatus : std::uint8_t {
  Valid = 0,
  InvalidLatch,
  InvalidLatchCoordinate,
  InvalidReference,
  InvalidReferencePhase,
  InvalidReferenceAction,
  InvalidClockFrequency,
  InvalidResetParameters,
  InvalidTolerance,
  InvalidHoldPreparation,
  InvalidActiveMatter,
  InvalidActiveSignal,
  InvalidMatterOrientation,
  NonemptyOutputPort,
  InsufficientReferenceReserve,
  AcknowledgementFailed,
  ResetWindowViolation,
  OutputDecoderFailed,
  NonFiniteOutput,
};

struct SignalAcknowledgedResetInput {
  std::int8_t latch = 0;
  double latch_coordinate = 0.0;
  CanonicalCarrierPair orientation_reference;
  double reference_phase = 0.0;
  double reference_action = 0.0;
  CanonicalCarrierPair matter;
  CanonicalCarrierPair signal;
  CanonicalCarrierPair output_port;
  double clock_frequency = 0.0;
  double latch_amplitude = 0.0;
  double reset_drag = 0.0;
  double reset_force = 0.0;
  double tolerance = 1e-12;
};

struct SignalAcknowledgedResetResult {
  SignalAcknowledgedResetStatus status =
      SignalAcknowledgedResetStatus::InvalidLatch;
  std::int8_t latch_before = 0;
  std::int8_t latch_after = 0;
  std::int8_t decoded_output_sign = 0;
  double latch_coordinate_before = 0.0;
  double latch_coordinate_after = 0.0;
  CanonicalCarrierPair matter_before;
  CanonicalCarrierPair signal_before;
  CanonicalCarrierPair midpoint_matter;
  CanonicalCarrierPair midpoint_signal;
  CanonicalCarrierPair matter_after;
  CanonicalCarrierPair signal_after;
  CanonicalCarrierPair exported_signal;
  double event_energy = 0.0;
  double decoded_output_energy = 0.0;
  double relative_action = 0.0;
  double reference_phase_midpoint = 0.0;
  double reference_phase_after = 0.0;
  double reference_action_after = 0.0;
  double minimum_reference_action = 0.0;
  double reserve_margin = 0.0;
  double maximum_interaction_energy = 0.0;
  double maximum_reference_energy_loan = 0.0;
  double reset_time = 0.0;
  double reset_window = 0.0;
  double reset_time_margin = 0.0;
  double controller_energy_supplied = 0.0;
  double scalar_bath_energy_exported = 0.0;
  double reset_ledger_residual = 0.0;
  double event_energy_residual = 0.0;
  bool exchange_stroke_exact = false;
  bool local_signal_acknowledged = false;
  bool acknowledgement_is_sign_even = false;
  bool no_extra_acknowledgement_bit = false;
  bool reset_window_compliant = false;
  bool nonsmooth_finite_time_reset_selected = false;
  bool smooth_finite_time_reset_established = false;
  bool scalar_bath_ledger_closed = false;
  bool local_state_ready = false;
  bool microscopic_bath_state_supplied = false;
  bool protected_cubic_transport_supplied = false;
  bool native_gstar_synchronization_supplied = false;
  bool production_latch_coupling_supplied = false;

  bool valid() const { return status == SignalAcknowledgedResetStatus::Valid; }
};

/** Execute one exact exchange/reset/export supercycle on the registered domain. */
SignalAcknowledgedResetResult execute_signal_acknowledged_two_stroke_reset(
    const SignalAcknowledgedResetInput& input);

}  // namespace ftd::eft
