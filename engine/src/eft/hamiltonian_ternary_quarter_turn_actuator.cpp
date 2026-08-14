#include "ftd/eft/hamiltonian_ternary_quarter_turn_actuator.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

bool ternary(std::int8_t value) {
  return value >= -1 && value <= 1;
}

bool close(double first, double second, double tolerance) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

bool gate_zero(double phase, double tolerance) {
  const double two_pi = 2.0 * std::acos(-1.0);
  return std::abs(std::remainder(phase, two_pi)) <= tolerance;
}

bool finite_result(const HamiltonianTernaryActuatorResult& result) {
  return std::isfinite(result.carrier_p_before)
      && std::isfinite(result.carrier_q_before)
      && std::isfinite(result.carrier_p_after)
      && std::isfinite(result.carrier_q_after)
      && std::isfinite(result.carrier_action_before)
      && std::isfinite(result.carrier_action_after)
      && std::isfinite(result.cycle_duration)
      && std::isfinite(result.base_phase)
      && std::isfinite(result.gated_phase)
      && std::isfinite(result.total_phase)
      && std::isfinite(result.imposed_record_energy_scale)
      && std::isfinite(result.record_energy_before)
      && std::isfinite(result.record_energy_after)
      && std::isfinite(result.minimum_reference_action)
      && std::isfinite(result.maximum_reference_action)
      && std::isfinite(result.maximum_reference_energy_exchange)
      && std::isfinite(result.maximum_interaction_energy_magnitude)
      && std::isfinite(result.endpoint_total_energy_before)
      && std::isfinite(result.endpoint_total_energy_after)
      && std::isfinite(result.endpoint_energy_residual);
}

}  // namespace

HamiltonianTernaryActuatorResult
evolve_hamiltonian_ternary_quarter_turn_cycle(
    const HamiltonianTernaryActuatorInput& input) {
  HamiltonianTernaryActuatorResult result;
  result.native_record_energy_scale_derived = false;
  result.dynamic_one_shot_scheduler_supplied = false;
  result.protected_cubic_transport_supplied = false;
  result.production_coupling_supplied = false;
  result.native_gstar_synchronization_supplied = false;
  result.repeated_active_cycle_is_one_shot = false;

  if (!ternary(input.latch)) {
    result.status = HamiltonianTernaryActuatorStatus::InvalidLatch;
    return result;
  }
  if (!ternary(input.port)) {
    result.status = HamiltonianTernaryActuatorStatus::InvalidPort;
    return result;
  }
  if (!std::isfinite(input.amplitude) || !(input.amplitude > 0.0)) {
    result.status = HamiltonianTernaryActuatorStatus::InvalidAmplitude;
    return result;
  }
  if (!std::isfinite(input.clock_frequency)
      || !(input.clock_frequency > 0.0)) {
    result.status = HamiltonianTernaryActuatorStatus::InvalidClockFrequency;
    return result;
  }
  if (!std::isfinite(input.reference_action)
      || !(input.reference_action > 0.0)) {
    result.status = HamiltonianTernaryActuatorStatus::InvalidReferenceAction;
    return result;
  }
  if (!std::isfinite(input.tolerance) || input.tolerance < 0.0) {
    result.status = HamiltonianTernaryActuatorStatus::InvalidTolerance;
    return result;
  }
  if (!std::isfinite(input.reference_phase)
      || !gate_zero(input.reference_phase, input.tolerance)) {
    result.status = HamiltonianTernaryActuatorStatus::InvalidReferencePhase;
    return result;
  }

  result.carrier_p_before = input.amplitude * input.latch;
  result.carrier_q_before = input.amplitude * input.port;
  result.carrier_action_before = 0.5 * (
      result.carrier_p_before * result.carrier_p_before
      + result.carrier_q_before * result.carrier_q_before);
  result.reference_action_before = input.reference_action;
  result.maximum_clock_action_excursion = input.eligible
      ? 0.5 * result.carrier_action_before
      : 0.0;
  result.conservative_reserve_margin = input.reference_action
      - result.maximum_clock_action_excursion;
  if (input.eligible
      && !(result.conservative_reserve_margin > input.tolerance)) {
    result.status =
        HamiltonianTernaryActuatorStatus::InsufficientReferenceReserve;
    return result;
  }

  const double pi = std::acos(-1.0);
  const double orientation_sign =
      input.orientation == TernaryQuarterTurnOrientation::Forward ? 1.0 : -1.0;
  result.cycle_duration = 2.0 * pi / input.clock_frequency;
  result.base_phase = 2.0 * pi;
  result.gated_phase = input.eligible
      ? orientation_sign * 0.5 * pi
      : 0.0;
  result.total_phase = result.base_phase + result.gated_phase;

  const double cosine = std::cos(result.total_phase);
  const double sine = std::sin(result.total_phase);
  result.carrier_p_after = cosine * result.carrier_p_before
      - sine * result.carrier_q_before;
  result.carrier_q_after = sine * result.carrier_p_before
      + cosine * result.carrier_q_before;
  result.carrier_action_after = 0.5 * (
      result.carrier_p_after * result.carrier_p_after
      + result.carrier_q_after * result.carrier_q_after);

  result.logical_transfer = apply_oriented_ternary_quarter_turn({
      input.latch,
      input.port,
      input.eligible,
      input.orientation,
  });
  if (!result.logical_transfer.valid()) {
    result.status = HamiltonianTernaryActuatorStatus::LogicalQuarterTurnRejected;
    return result;
  }

  const double expected_p = input.amplitude
      * result.logical_transfer.latch_after;
  const double expected_q = input.amplitude
      * result.logical_transfer.port_after;
  const double phase_tolerance = std::max(
      input.tolerance,
      64.0 * std::numeric_limits<double>::epsilon());
  result.continuous_flow_matches_ternary =
      close(result.carrier_p_after, expected_p, phase_tolerance)
      && close(result.carrier_q_after, expected_q, phase_tolerance);
  if (!result.continuous_flow_matches_ternary) {
    result.status = HamiltonianTernaryActuatorStatus::ContinuousLiftMismatch;
    return result;
  }

  result.imposed_record_energy_scale =
      0.5 * input.clock_frequency * input.amplitude * input.amplitude;
  result.imposed_record_energy_scale_supplied = true;
  result.record_energy_before =
      input.clock_frequency * result.carrier_action_before;
  result.record_energy_after =
      input.clock_frequency * result.carrier_action_after;

  if (!input.eligible) {
    result.minimum_reference_action = input.reference_action;
    result.maximum_reference_action = input.reference_action;
  } else if (input.orientation == TernaryQuarterTurnOrientation::Forward) {
    result.minimum_reference_action = input.reference_action
        - result.maximum_clock_action_excursion;
    result.maximum_reference_action = input.reference_action;
  } else {
    result.minimum_reference_action = input.reference_action;
    result.maximum_reference_action = input.reference_action
        + result.maximum_clock_action_excursion;
  }
  result.reference_action_after = input.reference_action;
  result.maximum_reference_energy_exchange = input.clock_frequency
      * result.maximum_clock_action_excursion;
  result.maximum_interaction_energy_magnitude =
      result.maximum_reference_energy_exchange;
  result.gate_zero_switch_work = 0.0;
  result.antiphase_switch_work_magnitude =
      0.5 * input.clock_frequency * result.carrier_action_before;

  result.endpoint_total_energy_before =
      input.clock_frequency * input.reference_action
      + result.record_energy_before;
  result.endpoint_total_energy_after =
      input.clock_frequency * result.reference_action_after
      + result.record_energy_after;
  result.endpoint_energy_residual =
      result.endpoint_total_energy_after - result.endpoint_total_energy_before;

  result.carrier_action_preserved = close(
      result.carrier_action_before,
      result.carrier_action_after,
      phase_tolerance);
  result.exact_hold = !input.eligible
      && result.logical_transfer.latch_after == input.latch
      && result.logical_transfer.port_after == input.port;
  result.exact_forward_quarter_turn = input.eligible
      && input.orientation == TernaryQuarterTurnOrientation::Forward
      && result.logical_transfer.latch_after == -input.port
      && result.logical_transfer.port_after == input.latch;
  result.exact_reverse_quarter_turn = input.eligible
      && input.orientation == TernaryQuarterTurnOrientation::Reverse
      && result.logical_transfer.latch_after == input.port
      && result.logical_transfer.port_after == -input.latch;
  result.exact_hamiltonian_lift =
      result.continuous_flow_matches_ternary
      && result.carrier_action_preserved
      && result.logical_transfer.exact_inverse_verified;
  result.controller_exchange_ledger_supplied = true;
  result.gate_zero_switching_booked = true;
  result.complete_cycle_net_work_zero = close(
      result.endpoint_energy_residual,
      0.0,
      phase_tolerance);

  if (!finite_result(result)) {
    result.status = HamiltonianTernaryActuatorStatus::NonFiniteOutput;
    return result;
  }
  result.status = HamiltonianTernaryActuatorStatus::Valid;
  return result;
}

}  // namespace ftd::eft

