#include "ftd/eft/ternary_eligibility_clutch.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

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

std::int8_t sign_with_tolerance(double value, double tolerance) {
  if (value > tolerance) return 1;
  if (value < -tolerance) return -1;
  return 0;
}

}  // namespace

TernaryEligibilityClutchResult execute_ternary_eligibility_handshake(
    const TernaryEligibilityClutchInput& input) {
  TernaryEligibilityClutchResult result;
  result.latch_before = input.latch;
  result.requested_latch_after = 0;

  if (input.latch < -1 || input.latch > 1) {
    result.status = TernaryEligibilityClutchStatus::InvalidLatch;
    return result;
  }
  const double tolerance = input.clock_parameters.tolerance;
  if (!std::isfinite(tolerance) || tolerance < 0.0) {
    result.status = TernaryEligibilityClutchStatus::InvalidTolerance;
    return result;
  }

  const CatalyticSignalReadout reference_probe =
      read_catalytic_phase_signal(input.orientation_reference, {0.0, 0.0});
  if (!reference_probe.valid()) {
    result.status = TernaryEligibilityClutchStatus::InvalidOrientationReference;
    return result;
  }

  const int eligibility_integer = static_cast<int>(input.latch)
      * static_cast<int>(input.latch);
  result.latch_square_is_eligibility = eligibility_integer == 0
      || eligibility_integer == 1;
  result.derived_eligibility = eligibility_integer == 0
      ? RecordPortEligibility::Hold
      : RecordPortEligibility::Exchange;

  ClockGatedHamiltonianParameters parameters = input.clock_parameters;
  parameters.eligibility = result.derived_eligibility;

  if (eligibility_integer == 0) {
    if (!near_zero_pair(input.clock_state.matter, tolerance)
        || !near_zero_pair(input.clock_state.signal, tolerance)) {
      result.status = TernaryEligibilityClutchStatus::InvalidHoldPreparation;
      return result;
    }
  } else {
    if (!near_zero_pair(input.clock_state.signal, tolerance)) {
      result.status = TernaryEligibilityClutchStatus::InvalidActiveSignal;
      return result;
    }
    result.matter_readout_before = read_catalytic_phase_signal(
        input.orientation_reference,
        input.clock_state.matter);
    if (!result.matter_readout_before.valid()) {
      result.status = TernaryEligibilityClutchStatus::InvalidActiveMatter;
      return result;
    }
    result.event_energy_before = pair_action(input.clock_state.matter);
    const double matter_scale = std::max(
        1.0,
        std::sqrt(2.0 * result.event_energy_before));
    if (!(result.event_energy_before > tolerance * tolerance)) {
      result.status = TernaryEligibilityClutchStatus::InvalidActiveMatter;
      return result;
    }
    if (std::abs(result.matter_readout_before.parallel_amplitude)
            > tolerance * matter_scale
        || sign_with_tolerance(
               result.matter_readout_before.oriented_area,
               tolerance * matter_scale)
            != input.latch) {
      result.status = TernaryEligibilityClutchStatus::InvalidMatterOrientation;
      return result;
    }
  }

  result.clock_cycle = evolve_clock_gated_hamiltonian_cycle(
      input.clock_state,
      parameters);
  if (!result.clock_cycle.valid()
      || (eligibility_integer == 0 && !result.clock_cycle.exact_hold)
      || (eligibility_integer == 1 && !result.clock_cycle.exact_swap)) {
    result.status = TernaryEligibilityClutchStatus::ClockCycleRejected;
    return result;
  }

  result.signal_readout_after = read_catalytic_phase_signal(
      input.orientation_reference,
      result.clock_cycle.after.signal);
  if (!result.signal_readout_after.valid()) {
    result.status = TernaryEligibilityClutchStatus::SignalDecoderRejected;
    return result;
  }
  result.decoded_event_energy = result.signal_readout_after.signal_energy;
  result.oriented_area_after = result.signal_readout_after.oriented_area;
  result.decoded_latch = sign_with_tolerance(
      result.oriented_area_after,
      tolerance * std::max(
          1.0,
          std::sqrt(2.0 * result.decoded_event_energy)));

  const double phase = result.clock_cycle.after.reference_phase;
  result.endpoint_gate_value = 1.0 - std::cos(phase);
  result.clutch_switch_work_at_release = -static_cast<double>(eligibility_integer)
      * parameters.coupling
      * result.endpoint_gate_value
      * result.clock_cycle.relative_action;
  result.clutch_release_requested = eligibility_integer == 1;
  result.clutch_release_at_gate_zero =
      std::abs(result.endpoint_gate_value) <= tolerance;
  result.second_active_cycle_would_undo_exchange = eligibility_integer == 1;

  if (eligibility_integer == 0) {
    result.decoded_latch = 0;
    result.declared_event_recoverable_after_requested_reset =
        close(result.decoded_event_energy, 0.0, tolerance);
  } else {
    result.declared_event_recoverable_after_requested_reset =
        result.decoded_latch == input.latch
        && close(
            result.decoded_event_energy,
            result.event_energy_before,
            tolerance)
        && near_zero_pair(result.clock_cycle.after.matter, tolerance);
  }
  if (!result.clutch_release_at_gate_zero
      || !result.declared_event_recoverable_after_requested_reset) {
    result.status = TernaryEligibilityClutchStatus::EventRecoveryFailed;
    return result;
  }

  result.microscopic_latch_reset_supplied = false;
  result.autonomous_acknowledgement_supplied = false;
  result.clock_synchronization_supplied = false;
  result.cubic_production_coupling_supplied = false;

  if (!std::isfinite(result.event_energy_before)
      || !std::isfinite(result.decoded_event_energy)
      || !std::isfinite(result.oriented_area_after)
      || !std::isfinite(result.endpoint_gate_value)
      || !std::isfinite(result.clutch_switch_work_at_release)) {
    result.status = TernaryEligibilityClutchStatus::NonFiniteOutput;
    return result;
  }
  result.status = TernaryEligibilityClutchStatus::Valid;
  return result;
}

}  // namespace ftd::eft
