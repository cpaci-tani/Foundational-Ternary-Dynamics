#include "ftd/eft/signal_acknowledged_two_stroke_reset.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

bool finite_pair(const CanonicalCarrierPair& pair) {
  return std::isfinite(pair.q) && std::isfinite(pair.p);
}

double pair_action(const CanonicalCarrierPair& pair) {
  const double radius = std::hypot(pair.q, pair.p);
  return 0.5 * radius * radius;
}

bool near_zero_pair(const CanonicalCarrierPair& pair, double tolerance) {
  return std::hypot(pair.q, pair.p) <= tolerance;
}

bool close(double first, double second, double tolerance) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

bool gate_zero_phase(double phase, double tolerance) {
  const double two_pi = 2.0 * std::acos(-1.0);
  return std::abs(std::remainder(phase, two_pi)) <= tolerance;
}

std::int8_t sign_with_tolerance(double value, double tolerance) {
  if (value > tolerance) return 1;
  if (value < -tolerance) return -1;
  return 0;
}

}  // namespace

SignalAcknowledgedResetResult execute_signal_acknowledged_two_stroke_reset(
    const SignalAcknowledgedResetInput& input) {
  SignalAcknowledgedResetResult result;
  result.latch_before = input.latch;
  result.latch_coordinate_before = input.latch_coordinate;
  result.matter_before = input.matter;
  result.signal_before = input.signal;

  if (input.latch < -1 || input.latch > 1) {
    result.status = SignalAcknowledgedResetStatus::InvalidLatch;
    return result;
  }
  if (!std::isfinite(input.tolerance) || input.tolerance < 0.0) {
    result.status = SignalAcknowledgedResetStatus::InvalidTolerance;
    return result;
  }
  if (!finite_pair(input.orientation_reference)
      || !(pair_action(input.orientation_reference) > 0.0)) {
    result.status = SignalAcknowledgedResetStatus::InvalidReference;
    return result;
  }
  if (!std::isfinite(input.reference_phase)
      || !gate_zero_phase(input.reference_phase, input.tolerance)) {
    result.status = SignalAcknowledgedResetStatus::InvalidReferencePhase;
    return result;
  }
  if (!std::isfinite(input.reference_action)
      || !(input.reference_action > 0.0)) {
    result.status = SignalAcknowledgedResetStatus::InvalidReferenceAction;
    return result;
  }
  if (!std::isfinite(input.clock_frequency)
      || !(input.clock_frequency > 0.0)) {
    result.status = SignalAcknowledgedResetStatus::InvalidClockFrequency;
    return result;
  }
  if (!std::isfinite(input.latch_amplitude)
      || !(input.latch_amplitude > 0.0)
      || !std::isfinite(input.reset_drag)
      || !(input.reset_drag > 0.0)
      || !std::isfinite(input.reset_force)
      || !(input.reset_force > 0.0)) {
    result.status = SignalAcknowledgedResetStatus::InvalidResetParameters;
    return result;
  }
  if (!finite_pair(input.matter) || !finite_pair(input.signal)) {
    result.status = SignalAcknowledgedResetStatus::InvalidActiveMatter;
    return result;
  }
  if (!finite_pair(input.output_port)
      || !near_zero_pair(input.output_port, input.tolerance)) {
    result.status = SignalAcknowledgedResetStatus::NonemptyOutputPort;
    return result;
  }

  const double pi = std::acos(-1.0);
  const int eligibility = static_cast<int>(input.latch)
      * static_cast<int>(input.latch);
  const double expected_latch_coordinate =
      static_cast<double>(input.latch) * input.latch_amplitude;
  if (!std::isfinite(input.latch_coordinate)
      || !close(
          input.latch_coordinate,
          expected_latch_coordinate,
          input.tolerance)) {
    result.status = SignalAcknowledgedResetStatus::InvalidLatchCoordinate;
    return result;
  }

  result.reference_phase_midpoint = input.reference_phase + pi;
  result.reference_phase_after = input.reference_phase + 2.0 * pi;
  result.reference_action_after = input.reference_action;
  result.reset_window = pi / input.clock_frequency;
  result.reset_time = input.reset_drag * input.latch_amplitude
      / input.reset_force;
  result.reset_time_margin = result.reset_window - result.reset_time;
  result.reset_window_compliant =
      result.reset_time_margin >= -input.tolerance;
  result.acknowledgement_is_sign_even = true;
  result.no_extra_acknowledgement_bit = true;
  result.smooth_finite_time_reset_established = false;
  result.microscopic_bath_state_supplied = false;
  result.protected_cubic_transport_supplied = false;
  result.native_gstar_synchronization_supplied = false;
  result.production_latch_coupling_supplied = false;

  if (eligibility == 0) {
    if (!near_zero_pair(input.matter, input.tolerance)
        || !near_zero_pair(input.signal, input.tolerance)) {
      result.status = SignalAcknowledgedResetStatus::InvalidHoldPreparation;
      return result;
    }
    result.midpoint_matter = input.matter;
    result.midpoint_signal = input.signal;
    result.matter_after = input.matter;
    result.signal_after = input.signal;
    result.exported_signal = input.output_port;
    result.minimum_reference_action = input.reference_action;
    result.reserve_margin = input.reference_action;
    result.latch_after = 0;
    result.latch_coordinate_after = 0.0;
    result.exchange_stroke_exact = true;
    result.local_signal_acknowledged = false;
    result.nonsmooth_finite_time_reset_selected = false;
    result.scalar_bath_ledger_closed = true;
    result.local_state_ready = true;
    result.status = SignalAcknowledgedResetStatus::Valid;
    return result;
  }

  if (!near_zero_pair(input.signal, input.tolerance)) {
    result.status = SignalAcknowledgedResetStatus::InvalidActiveSignal;
    return result;
  }
  const auto matter_readout = read_catalytic_phase_signal(
      input.orientation_reference,
      input.matter);
  if (!matter_readout.valid()) {
    result.status = SignalAcknowledgedResetStatus::InvalidActiveMatter;
    return result;
  }
  result.event_energy = pair_action(input.matter);
  const double matter_scale = std::max(
      1.0,
      std::sqrt(2.0 * result.event_energy));
  if (!(result.event_energy > input.tolerance * input.tolerance)) {
    result.status = SignalAcknowledgedResetStatus::InvalidActiveMatter;
    return result;
  }
  if (std::abs(matter_readout.parallel_amplitude)
          > input.tolerance * matter_scale
      || sign_with_tolerance(
             matter_readout.oriented_area,
             input.tolerance * matter_scale)
          != input.latch) {
    result.status = SignalAcknowledgedResetStatus::InvalidMatterOrientation;
    return result;
  }

  result.relative_action = 0.5 * result.event_energy;
  result.minimum_reference_action = input.reference_action - result.event_energy;
  result.reserve_margin = result.minimum_reference_action;
  if (!(result.minimum_reference_action > input.tolerance)) {
    result.status = SignalAcknowledgedResetStatus::InsufficientReferenceReserve;
    return result;
  }
  result.maximum_interaction_energy =
      input.clock_frequency * result.event_energy;
  result.maximum_reference_energy_loan =
      input.clock_frequency * result.event_energy;

  result.midpoint_matter = input.signal;
  result.midpoint_signal = input.matter;
  result.exchange_stroke_exact =
      near_zero_pair(result.midpoint_matter, input.tolerance)
      && close(
          pair_action(result.midpoint_signal),
          result.event_energy,
          input.tolerance);
  result.local_signal_acknowledged = result.exchange_stroke_exact
      && pair_action(result.midpoint_signal) > input.tolerance * input.tolerance;
  if (!result.local_signal_acknowledged) {
    result.status = SignalAcknowledgedResetStatus::AcknowledgementFailed;
    return result;
  }
  if (!result.reset_window_compliant) {
    result.status = SignalAcknowledgedResetStatus::ResetWindowViolation;
    return result;
  }

  result.nonsmooth_finite_time_reset_selected = true;
  result.latch_after = 0;
  result.latch_coordinate_after = 0.0;
  result.controller_energy_supplied =
      input.reset_force * input.latch_amplitude;
  result.scalar_bath_energy_exported = result.controller_energy_supplied;
  result.reset_ledger_residual = result.scalar_bath_energy_exported
      - result.controller_energy_supplied;
  result.scalar_bath_ledger_closed = close(
      result.reset_ledger_residual,
      0.0,
      input.tolerance);

  result.matter_after = {0.0, 0.0};
  result.signal_after = {0.0, 0.0};
  result.exported_signal = result.midpoint_signal;
  const auto output_readout = read_catalytic_phase_signal(
      input.orientation_reference,
      result.exported_signal);
  if (!output_readout.valid()) {
    result.status = SignalAcknowledgedResetStatus::OutputDecoderFailed;
    return result;
  }
  result.decoded_output_energy = output_readout.signal_energy;
  result.decoded_output_sign = sign_with_tolerance(
      output_readout.oriented_area,
      input.tolerance * matter_scale);
  result.event_energy_residual =
      result.decoded_output_energy - result.event_energy;
  result.local_state_ready = result.latch_after == 0
      && close(result.latch_coordinate_after, 0.0, input.tolerance)
      && near_zero_pair(result.matter_after, input.tolerance)
      && near_zero_pair(result.signal_after, input.tolerance)
      && result.decoded_output_sign == input.latch
      && close(result.event_energy_residual, 0.0, input.tolerance);
  if (!result.local_state_ready || !result.scalar_bath_ledger_closed) {
    result.status = SignalAcknowledgedResetStatus::OutputDecoderFailed;
    return result;
  }

  if (!std::isfinite(result.reference_phase_after)
      || !std::isfinite(result.minimum_reference_action)
      || !std::isfinite(result.reset_time)
      || !std::isfinite(result.reset_time_margin)
      || !std::isfinite(result.controller_energy_supplied)
      || !std::isfinite(result.scalar_bath_energy_exported)
      || !std::isfinite(result.event_energy_residual)) {
    result.status = SignalAcknowledgedResetStatus::NonFiniteOutput;
    return result;
  }
  result.status = SignalAcknowledgedResetStatus::Valid;
  return result;
}

}  // namespace ftd::eft
