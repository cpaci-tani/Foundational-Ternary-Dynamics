#include "ftd/eft/canonical_source_centered_gauss_gate.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;

bool finite(double value) {
  return std::isfinite(value);
}

bool close(double first, double second, double tolerance) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

double raw_energy(const CanonicalSourceCenteredGaussMode& mode) {
  return 0.5 * (
      mode.field_normal * mode.field_normal + mode.port * mode.port
      + mode.field_conjugate * mode.field_conjugate
      + mode.port_conjugate * mode.port_conjugate);
}

double interaction_energy(const CanonicalSourceCenteredGaussMode& mode) {
  return -mode.source_offset * mode.field_normal
      + 0.5 * mode.source_offset * mode.source_offset;
}

double centered_norm(const CanonicalSourceCenteredGaussMode& mode) {
  const double residual = mode.field_normal - mode.source_offset;
  return 0.5 * (
      residual * residual + mode.port * mode.port
      + mode.field_conjugate * mode.field_conjugate
      + mode.port_conjugate * mode.port_conjugate);
}

double history_port_energy(const CanonicalHistoryPort& port) {
  return 0.5 * (
      port.coordinate * port.coordinate + port.conjugate * port.conjugate);
}

bool same_mode(
    const CanonicalSourceCenteredGaussMode& first,
    const CanonicalSourceCenteredGaussMode& second,
    double tolerance) {
  return close(first.field_normal, second.field_normal, tolerance)
      && close(first.port, second.port, tolerance)
      && close(first.field_conjugate, second.field_conjugate, tolerance)
      && close(first.port_conjugate, second.port_conjugate, tolerance)
      && close(first.source_offset, second.source_offset, tolerance);
}

bool same_port(
    const CanonicalHistoryPort& first,
    const CanonicalHistoryPort& second,
    double tolerance) {
  return close(first.coordinate, second.coordinate, tolerance)
      && close(first.conjugate, second.conjugate, tolerance);
}

}  // namespace

CanonicalSourceCenteredGaussResult
evolve_canonical_source_centered_gauss_cycle(
    const CanonicalSourceCenteredGaussInput& input) {
  CanonicalSourceCenteredGaussResult result;
  result.orientation = input.orientation;
  result.before = input.mode;

  const auto& mode = input.mode;
  if (!finite(mode.field_normal) || !finite(mode.port)
      || !finite(mode.field_conjugate) || !finite(mode.port_conjugate)
      || !finite(mode.source_offset)) {
    result.status = CanonicalSourceCenteredGaussStatus::NonFiniteInput;
    return result;
  }
  if (!finite(input.clock_frequency) || input.clock_frequency <= 0.0) {
    result.status = CanonicalSourceCenteredGaussStatus::InvalidClockFrequency;
    return result;
  }
  if (!finite(input.reference_action) || input.reference_action <= 0.0) {
    result.status = CanonicalSourceCenteredGaussStatus::InvalidReferenceAction;
    return result;
  }
  if (!finite(input.tolerance) || input.tolerance <= 0.0) {
    result.status = CanonicalSourceCenteredGaussStatus::InvalidTolerance;
    return result;
  }
  if (!finite(input.reference_phase)
      || std::abs(std::remainder(input.reference_phase, 2.0 * kPi))
          > input.tolerance) {
    result.status = CanonicalSourceCenteredGaussStatus::InvalidReferencePhase;
    return result;
  }

  const double residual = mode.field_normal - mode.source_offset;
  const double generator = mode.port * mode.field_conjugate
      - residual * mode.port_conjugate;
  const double action_excursion = 0.5 * std::abs(generator);
  if (!(input.reference_action > action_excursion + input.tolerance)) {
    result.status =
        CanonicalSourceCenteredGaussStatus::InsufficientReferenceReserve;
    return result;
  }

  result.status = CanonicalSourceCenteredGaussStatus::Valid;
  result.residual_before = residual;
  result.cycle_duration = 2.0 * kPi / input.clock_frequency;
  const double orientation_sign =
      input.orientation == TernaryQuarterTurnOrientation::Forward ? 1.0 : -1.0;
  result.pulse_angle = orientation_sign * kPi / 2.0;
  result.angular_momentum = generator;
  result.carrier_norm_before = centered_norm(mode);
  result.carrier_hamiltonian_lower_bound =
      0.5 * input.clock_frequency * result.carrier_norm_before;
  result.reference_action_before = input.reference_action;
  result.minimum_reference_action = input.reference_action - action_excursion;
  result.maximum_reference_action = input.reference_action + action_excursion;
  result.reference_action_after = input.reference_action;
  result.maximum_clock_action_excursion = action_excursion;

  CanonicalSourceCenteredGaussMode after = mode;
  if (input.orientation == TernaryQuarterTurnOrientation::Forward) {
    after.field_normal = mode.port + mode.source_offset;
    after.port = -residual;
    after.field_conjugate = mode.port_conjugate;
    after.port_conjugate = -mode.field_conjugate;
  } else {
    after.field_normal = -mode.port + mode.source_offset;
    after.port = residual;
    after.field_conjugate = -mode.port_conjugate;
    after.port_conjugate = mode.field_conjugate;
  }
  result.after = after;
  result.residual_after = after.field_normal - after.source_offset;
  result.carrier_norm_after = centered_norm(after);

  CanonicalSourceCenteredGaussMode recovered = after;
  if (input.orientation == TernaryQuarterTurnOrientation::Forward) {
    const double after_residual = after.field_normal - after.source_offset;
    recovered.field_normal = -after.port + after.source_offset;
    recovered.port = after_residual;
    recovered.field_conjugate = -after.port_conjugate;
    recovered.port_conjugate = after.field_conjugate;
  } else {
    const double after_residual = after.field_normal - after.source_offset;
    recovered.field_normal = after.port + after.source_offset;
    recovered.port = -after_residual;
    recovered.field_conjugate = after.port_conjugate;
    recovered.port_conjugate = -after.field_conjugate;
  }
  result.recovered = recovered;
  result.exact_inverse_verified = same_mode(mode, recovered, input.tolerance);

  result.raw_energy_before = raw_energy(mode);
  result.raw_energy_after = raw_energy(after);
  result.interaction_energy_before = interaction_energy(mode);
  result.interaction_energy_after = interaction_energy(after);
  result.raw_source_work = result.raw_energy_after - result.raw_energy_before;
  result.interaction_work =
      result.interaction_energy_after - result.interaction_energy_before;
  result.source_work_residual = result.raw_source_work + result.interaction_work;
  result.endpoint_hamiltonian_before = input.clock_frequency
      * (input.reference_action + result.carrier_norm_before);
  result.endpoint_hamiltonian_after = input.clock_frequency
      * (input.reference_action + result.carrier_norm_after);
  result.endpoint_hamiltonian_residual =
      result.endpoint_hamiltonian_after - result.endpoint_hamiltonian_before;

  result.positive_source_centered_hamiltonian =
      result.carrier_norm_before >= -input.tolerance
      && result.carrier_hamiltonian_lower_bound >= -input.tolerance;
  result.zero_conjugate_section =
      std::abs(mode.field_conjugate) <= input.tolerance
      && std::abs(mode.port_conjugate) <= input.tolerance;
  result.zero_conjugate_section_preserved =
      !result.zero_conjugate_section
      || (std::abs(after.field_conjugate) <= input.tolerance
          && std::abs(after.port_conjugate) <= input.tolerance);
  result.frozen_gauss_configuration_gate_reproduced =
      input.orientation == TernaryQuarterTurnOrientation::Forward
      && close(result.residual_after, mode.port, input.tolerance)
      && close(after.port, -residual, input.tolerance);
  result.raw_work_is_interaction_energy_exchange =
      close(result.source_work_residual, 0.0, input.tolerance);
  return result;
}

SquareRootBatteryPhaseAudit audit_square_root_battery_phase_completion(
    double amplitude,
    double conjugate,
    double work,
    double tolerance) {
  SquareRootBatteryPhaseAudit result;
  result.amplitude_before = amplitude;
  result.conjugate_before = conjugate;
  result.work = work;
  result.target_energy_change = -work;
  result.phase_circle_flux = -2.0 * kPi * work;
  result.constant_action_translation_globally_hamiltonian =
      finite(work) && std::abs(work) <= tolerance;

  if (!finite(amplitude) || !finite(conjugate) || !finite(work)) {
    result.status = SquareRootBatteryPhaseStatus::NonFiniteInput;
    return result;
  }
  if (!finite(tolerance) || tolerance <= 0.0) {
    result.status = SquareRootBatteryPhaseStatus::InvalidTolerance;
    return result;
  }
  if (std::abs(amplitude) <= tolerance) {
    result.status = SquareRootBatteryPhaseStatus::EmptyAmplitude;
    return result;
  }
  const double radicand = amplitude * amplitude - 2.0 * work;
  if (!(radicand > tolerance * tolerance)) {
    result.status = SquareRootBatteryPhaseStatus::ReserveDepleted;
    return result;
  }

  result.status = SquareRootBatteryPhaseStatus::Valid;
  result.amplitude_after = std::copysign(std::sqrt(radicand), amplitude);
  result.conjugate_after =
      conjugate * result.amplitude_after / amplitude;
  result.oscillator_energy_before =
      0.5 * (amplitude * amplitude + conjugate * conjugate);
  result.oscillator_energy_after = 0.5 * (
      result.amplitude_after * result.amplitude_after
      + result.conjugate_after * result.conjugate_after);
  result.oscillator_energy_change =
      result.oscillator_energy_after - result.oscillator_energy_before;
  result.energy_change_residual =
      result.oscillator_energy_change - result.target_energy_change;
  result.cotangent_jacobian = 1.0;
  result.sign_preserved = std::signbit(result.amplitude_after)
      == std::signbit(amplitude);
  result.cotangent_lift_symplectic = true;
  result.lagrangian_section = std::abs(conjugate) <= tolerance;
  result.exact_work_ledger =
      close(result.energy_change_residual, 0.0, tolerance);
  return result;
}

OpenCanonicalHistoryShiftResult shift_open_canonical_history_right(
    const std::vector<CanonicalHistoryPort>& rail,
    CanonicalHistoryPort incoming,
    double tolerance) {
  OpenCanonicalHistoryShiftResult result;
  result.before = rail;
  result.incoming = incoming;
  if (rail.empty()) {
    result.status = OpenCanonicalHistoryStatus::EmptyRail;
    return result;
  }
  if (!finite(tolerance) || tolerance <= 0.0) {
    result.status = OpenCanonicalHistoryStatus::InvalidTolerance;
    return result;
  }
  const auto finite_port = [](const CanonicalHistoryPort& port) {
    return finite(port.coordinate) && finite(port.conjugate);
  };
  if (!finite_port(incoming)
      || !std::all_of(rail.begin(), rail.end(), finite_port)) {
    result.status = OpenCanonicalHistoryStatus::NonFiniteInput;
    return result;
  }

  result.status = OpenCanonicalHistoryStatus::Valid;
  result.outgoing = rail.back();
  result.after.resize(rail.size());
  result.after.front() = incoming;
  for (std::size_t index = 1; index < rail.size(); ++index) {
    result.after[index] = rail[index - 1];
  }
  result.recovered.resize(rail.size());
  for (std::size_t index = 0; index + 1 < rail.size(); ++index) {
    result.recovered[index] = result.after[index + 1];
  }
  result.recovered.back() = result.outgoing;

  for (const auto& port : rail) {
    result.rail_energy_before += history_port_energy(port);
  }
  for (const auto& port : result.after) {
    result.rail_energy_after += history_port_energy(port);
  }
  result.incoming_energy = history_port_energy(incoming);
  result.outgoing_energy = history_port_energy(result.outgoing);
  result.open_energy_residual = result.rail_energy_after
      - result.rail_energy_before - result.incoming_energy
      + result.outgoing_energy;
  result.complete_pair_shifted = true;
  result.symplectic_with_boundaries = true;
  result.exact_inverse_verified = true;
  for (std::size_t index = 0; index < rail.size(); ++index) {
    result.exact_inverse_verified &=
        same_port(result.recovered[index], rail[index], tolerance);
  }
  return result;
}

}  // namespace ftd::eft
