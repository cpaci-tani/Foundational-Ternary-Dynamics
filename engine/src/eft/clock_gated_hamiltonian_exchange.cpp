#include "ftd/eft/clock_gated_hamiltonian_exchange.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

bool finite_pair(const CanonicalCarrierPair& pair) {
  return std::isfinite(pair.q) && std::isfinite(pair.p);
}

double pair_action(const CanonicalCarrierPair& pair) {
  const double radius = std::hypot(pair.q, pair.p);
  return 0.5 * radius * radius;
}

CanonicalCarrierPair add(
    const CanonicalCarrierPair& first,
    const CanonicalCarrierPair& second,
    double scale) {
  return {
      scale * (first.q + second.q),
      scale * (first.p + second.p),
  };
}

CanonicalCarrierPair subtract(
    const CanonicalCarrierPair& first,
    const CanonicalCarrierPair& second,
    double scale) {
  return {
      scale * (first.q - second.q),
      scale * (first.p - second.p),
  };
}

CanonicalCarrierPair rotate(
    const CanonicalCarrierPair& pair,
    double phase) {
  const double cosine = std::cos(phase);
  const double sine = std::sin(phase);
  return {
      cosine * pair.q + sine * pair.p,
      -sine * pair.q + cosine * pair.p,
  };
}

bool close_phase(double phase, double target, double tolerance) {
  const double two_pi = 2.0 * std::acos(-1.0);
  return std::abs(std::remainder(phase - target, two_pi)) <= tolerance;
}

bool finite_result(const ClockGatedHamiltonianResult& result) {
  return finite_pair(result.after.matter)
      && finite_pair(result.after.signal)
      && finite_pair(result.common_after)
      && finite_pair(result.relative_after)
      && std::isfinite(result.after.reference_phase)
      && std::isfinite(result.after.reference_action)
      && std::isfinite(result.common_action)
      && std::isfinite(result.relative_action)
      && std::isfinite(result.mode_action_before)
      && std::isfinite(result.mode_action_after)
      && std::isfinite(result.minimum_reference_action)
      && std::isfinite(result.maximum_interaction_energy)
      && std::isfinite(result.maximum_reference_energy_loan)
      && std::isfinite(result.endpoint_energy_before)
      && std::isfinite(result.endpoint_energy_after)
      && std::isfinite(result.endpoint_energy_residual);
}

}  // namespace

ClockGatedHamiltonianResult evolve_clock_gated_hamiltonian_cycle(
    const ClockGatedHamiltonianState& state,
    const ClockGatedHamiltonianParameters& parameters) {
  ClockGatedHamiltonianResult result;
  result.before = state;

  if (!std::isfinite(parameters.clock_frequency)
      || !(parameters.clock_frequency > 0.0)) {
    result.status = ClockGatedHamiltonianStatus::InvalidClockFrequency;
    return result;
  }
  if (!std::isfinite(parameters.common_frequency)
      || !(parameters.common_frequency > 0.0)) {
    result.status = ClockGatedHamiltonianStatus::InvalidCommonFrequency;
    return result;
  }
  if (!std::isfinite(parameters.coupling) || parameters.coupling < 0.0) {
    result.status = ClockGatedHamiltonianStatus::InvalidCoupling;
    return result;
  }
  if (!std::isfinite(parameters.tolerance) || parameters.tolerance < 0.0) {
    result.status = ClockGatedHamiltonianStatus::InvalidTolerance;
    return result;
  }
  if (!std::isfinite(state.reference_phase)
      || !close_phase(
          state.reference_phase,
          0.0,
          parameters.tolerance)) {
    result.status = ClockGatedHamiltonianStatus::InvalidReferencePhase;
    return result;
  }
  if (!std::isfinite(state.reference_action)
      || !(state.reference_action > 0.0)) {
    result.status = ClockGatedHamiltonianStatus::InvalidReferenceAction;
    return result;
  }
  if (!finite_pair(state.matter)) {
    result.status = ClockGatedHamiltonianStatus::InvalidMatterMode;
    return result;
  }
  if (!finite_pair(state.signal)) {
    result.status = ClockGatedHamiltonianStatus::InvalidSignalMode;
    return result;
  }

  double eligibility = 0.0;
  switch (parameters.eligibility) {
    case RecordPortEligibility::Hold:
      eligibility = 0.0;
      break;
    case RecordPortEligibility::Exchange:
      eligibility = 1.0;
      break;
    default:
      result.status = ClockGatedHamiltonianStatus::InvalidEligibility;
      return result;
  }

  const double pi = std::acos(-1.0);
  const double two_pi = 2.0 * pi;
  const double inverse_sqrt_two = 1.0 / std::sqrt(2.0);
  result.common_before = add(
      state.matter,
      state.signal,
      inverse_sqrt_two);
  result.relative_before = subtract(
      state.matter,
      state.signal,
      inverse_sqrt_two);
  result.common_action = pair_action(result.common_before);
  result.relative_action = pair_action(result.relative_before);
  result.mode_action_before =
      pair_action(state.matter) + pair_action(state.signal);

  const double frequency_ratio =
      parameters.common_frequency / parameters.clock_frequency;
  const double coupling_ratio =
      parameters.coupling / parameters.clock_frequency;
  result.common_phase = two_pi * frequency_ratio;
  result.relative_extra_phase =
      eligibility * two_pi * coupling_ratio;
  result.minimum_reference_action = state.reference_action
      - eligibility * 2.0 * coupling_ratio * result.relative_action;
  result.reserve_margin = result.minimum_reference_action;
  if (!(result.minimum_reference_action > parameters.tolerance)) {
    result.status = ClockGatedHamiltonianStatus::InsufficientReferenceReserve;
    return result;
  }

  result.maximum_interaction_energy = eligibility
      * 2.0 * parameters.coupling * result.relative_action;
  result.maximum_reference_energy_loan = parameters.clock_frequency
      * (state.reference_action - result.minimum_reference_action);

  result.common_after = rotate(result.common_before, result.common_phase);
  result.relative_after = rotate(
      result.relative_before,
      result.common_phase + result.relative_extra_phase);
  result.after.matter = add(
      result.common_after,
      result.relative_after,
      inverse_sqrt_two);
  result.after.signal = subtract(
      result.common_after,
      result.relative_after,
      inverse_sqrt_two);
  result.after.reference_phase = state.reference_phase + two_pi;
  result.after.reference_action = state.reference_action;
  result.mode_action_after =
      pair_action(result.after.matter) + pair_action(result.after.signal);

  result.endpoint_energy_before =
      parameters.clock_frequency * state.reference_action
      + parameters.common_frequency * result.mode_action_before;
  result.endpoint_energy_after =
      parameters.clock_frequency * result.after.reference_action
      + parameters.common_frequency * result.mode_action_after;
  result.endpoint_energy_residual =
      result.endpoint_energy_after - result.endpoint_energy_before;

  const double phase_tolerance = std::max(
      parameters.tolerance,
      32.0 * std::numeric_limits<double>::epsilon());
  result.common_winding_compliant =
      close_phase(result.common_phase, 0.0, phase_tolerance);
  result.branch_winding_compliant = eligibility == 0.0
      ? close_phase(result.relative_extra_phase, 0.0, phase_tolerance)
      : close_phase(result.relative_extra_phase, pi, phase_tolerance);
  result.exact_hold = eligibility == 0.0
      && result.common_winding_compliant
      && result.branch_winding_compliant;
  result.exact_swap = eligibility == 1.0
      && result.common_winding_compliant
      && result.branch_winding_compliant;
  result.dynamic_eligibility_supplied = false;
  result.quartic_load_blind_controller_established = false;

  if (!finite_result(result)) {
    result.status = ClockGatedHamiltonianStatus::NonFiniteOutput;
    return result;
  }
  result.status = ClockGatedHamiltonianStatus::Valid;
  return result;
}

}  // namespace ftd::eft
