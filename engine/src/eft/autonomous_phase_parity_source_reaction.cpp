#include "ftd/eft/autonomous_phase_parity_source_reaction.h"

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

bool finite_mode(const SourceReactionMode& mode) {
  return finite(mode.residual)
      && finite(mode.history)
      && finite(mode.reaction)
      && finite(mode.residual_conjugate)
      && finite(mode.history_conjugate)
      && finite(mode.reaction_conjugate);
}

double pair_energy(double coordinate, double conjugate) {
  return 0.5 * (coordinate * coordinate + conjugate * conjugate);
}

double common_norm(const SourceReactionMode& mode) {
  return pair_energy(mode.residual, mode.residual_conjugate)
      + pair_energy(mode.history, mode.history_conjugate)
      + pair_energy(mode.reaction, mode.reaction_conjugate);
}

bool same_mode(
    const SourceReactionMode& first,
    const SourceReactionMode& second,
    double tolerance) {
  return close(first.residual, second.residual, tolerance)
      && close(first.history, second.history, tolerance)
      && close(first.reaction, second.reaction, tolerance)
      && close(
          first.residual_conjugate,
          second.residual_conjugate,
          tolerance)
      && close(
          first.history_conjugate,
          second.history_conjugate,
          tolerance)
      && close(
          first.reaction_conjugate,
          second.reaction_conjugate,
          tolerance);
}

}  // namespace

AutonomousPhaseParityResult
evolve_autonomous_phase_parity_source_reaction_cycle(
    const AutonomousPhaseParityInput& input) {
  AutonomousPhaseParityResult result;
  result.before = input.mode;
  result.source_offset = input.source_offset;
  result.split_angle = input.split_angle;
  result.clock_frequency = input.clock_frequency;

  if (!finite_mode(input.mode) || !finite(input.source_offset)) {
    result.status = AutonomousPhaseParityStatus::NonFiniteInput;
    return result;
  }
  if (!finite(input.split_angle)
      || input.split_angle < 0.0
      || input.split_angle > kPi / 2.0) {
    result.status = AutonomousPhaseParityStatus::InvalidSplitAngle;
    return result;
  }
  if (!finite(input.clock_frequency) || input.clock_frequency <= 0.0) {
    result.status = AutonomousPhaseParityStatus::InvalidClockFrequency;
    return result;
  }
  if (!finite(input.reference_action) || input.reference_action <= 0.0) {
    result.status = AutonomousPhaseParityStatus::InvalidReferenceAction;
    return result;
  }
  if (!finite(input.tolerance) || input.tolerance <= 0.0) {
    result.status = AutonomousPhaseParityStatus::InvalidTolerance;
    return result;
  }
  if (!finite(input.reference_phase)
      || std::abs(std::remainder(input.reference_phase, 2.0 * kPi))
          > input.tolerance) {
    result.status = AutonomousPhaseParityStatus::InvalidReferencePhase;
    return result;
  }

  result.common_norm_before = common_norm(input.mode);
  result.maximum_clock_action_excursion_bound =
      3.0 * result.common_norm_before;
  if (!(input.reference_action
        > result.maximum_clock_action_excursion_bound + input.tolerance)) {
    result.status = AutonomousPhaseParityStatus::InsufficientReferenceReserve;
    return result;
  }

  result.status = AutonomousPhaseParityStatus::Valid;
  result.cycle_duration = 2.0 * kPi / input.clock_frequency;
  result.window_duration = kPi / (3.0 * input.clock_frequency);
  result.base_winding_per_window = 2.0 * kPi;
  result.carrier_hamiltonian_lower_bound =
      3.0 * input.clock_frequency * result.common_norm_before;
  result.reference_action_before = input.reference_action;
  result.reference_action_after = input.reference_action;
  result.positive_carrier_hamiltonian =
      result.carrier_hamiltonian_lower_bound >= -input.tolerance;

  const std::array<int, 6> colors = {0, 0, 0, 1, 1, 1};
  const std::array<AutonomousPhasePulseKind, 6> kinds = {
      AutonomousPhasePulseKind::ResidualHistory,
      AutonomousPhasePulseKind::HistoryReaction,
      AutonomousPhasePulseKind::ReactionPhase,
      AutonomousPhasePulseKind::ResidualHistory,
      AutonomousPhasePulseKind::HistoryReaction,
      AutonomousPhasePulseKind::ReactionPhase,
  };
  const std::array<double, 6> angles = {
      kPi / 2.0,
      input.split_angle,
      kPi / 2.0,
      kPi / 2.0,
      input.split_angle,
      kPi / 2.0,
  };
  for (std::size_t index = 0; index < result.pulses.size(); ++index) {
    auto& pulse = result.pulses[index];
    pulse.checkerboard_color = colors[index];
    pulse.kind = kinds[index];
    pulse.phase_start = static_cast<double>(index) * kPi / 3.0;
    pulse.phase_end = static_cast<double>(index + 1) * kPi / 3.0;
    pulse.window_integral = kPi / 6.0;
    pulse.target_angle = angles[index];
    pulse.pulse_coefficient = 6.0 * angles[index] / kPi;
  }

  const double cosine = std::cos(input.split_angle);
  const double sine = std::sin(input.split_angle);
  const auto& before = input.mode;
  SourceReactionMode after;
  after.residual = before.history;
  after.history = -cosine * before.residual - sine * before.reaction;
  after.reaction =
      -sine * before.residual_conjugate
      + cosine * before.reaction_conjugate;
  after.residual_conjugate = before.history_conjugate;
  after.history_conjugate =
      -cosine * before.residual_conjugate
      - sine * before.reaction_conjugate;
  after.reaction_conjugate =
      sine * before.residual - cosine * before.reaction;
  result.after = after;
  result.common_norm_after = common_norm(after);

  SourceReactionMode recovered;
  const double reaction_before_phase = -after.reaction_conjugate;
  const double reaction_conjugate_before_phase = after.reaction;
  const double history_before_split =
      cosine * after.history + sine * reaction_before_phase;
  const double reaction_before_split =
      -sine * after.history + cosine * reaction_before_phase;
  const double history_conjugate_before_split =
      cosine * after.history_conjugate
      + sine * reaction_conjugate_before_phase;
  const double reaction_conjugate_before_split =
      -sine * after.history_conjugate
      + cosine * reaction_conjugate_before_phase;
  recovered.residual = -history_before_split;
  recovered.history = after.residual;
  recovered.reaction = reaction_before_split;
  recovered.residual_conjugate = -history_conjugate_before_split;
  recovered.history_conjugate = after.residual_conjugate;
  recovered.reaction_conjugate = reaction_conjugate_before_split;
  result.recovered = recovered;
  result.exact_inverse_verified =
      same_mode(before, recovered, input.tolerance);

  result.ready_reaction_slice =
      std::abs(before.history) <= input.tolerance
      && std::abs(before.reaction) <= input.tolerance
      && std::abs(before.residual_conjugate) <= input.tolerance
      && std::abs(before.history_conjugate) <= input.tolerance
      && std::abs(before.reaction_conjugate) <= input.tolerance;
  result.gauss_residual_cleared =
      result.ready_reaction_slice
      && std::abs(after.residual) <= input.tolerance
      && std::abs(after.residual_conjugate) <= input.tolerance;
  result.reaction_displacement_reset =
      result.ready_reaction_slice
      && std::abs(after.reaction) <= input.tolerance;
  result.source_reaction_impulse = after.reaction_conjugate;
  result.reaction_impulse_generated =
      result.ready_reaction_slice
      && std::abs(before.residual) > input.tolerance
      && input.split_angle > input.tolerance
      && std::abs(result.source_reaction_impulse) > input.tolerance;

  result.residual_energy_before =
      pair_energy(before.residual, before.residual_conjugate);
  result.history_energy_after =
      pair_energy(after.history, after.history_conjugate);
  result.reaction_energy_after =
      pair_energy(after.reaction, after.reaction_conjugate);
  result.exact_history_reaction_split =
      result.ready_reaction_slice
      && close(
          result.history_energy_after + result.reaction_energy_after,
          result.residual_energy_before,
          input.tolerance);

  const double field_before = input.source_offset + before.residual;
  const double field_after = input.source_offset + after.residual;
  result.raw_energy_before =
      0.5 * (field_before * field_before
             + before.history * before.history);
  result.raw_energy_after =
      0.5 * (field_after * field_after
             + after.history * after.history);
  result.interaction_energy_before =
      -input.source_offset * field_before
      + 0.5 * input.source_offset * input.source_offset;
  result.interaction_energy_after =
      -input.source_offset * field_after
      + 0.5 * input.source_offset * input.source_offset;
  result.old_source_work = -input.source_offset * before.residual;
  result.completed_energy_residual =
      result.raw_energy_after - result.raw_energy_before
      + result.interaction_energy_after - result.interaction_energy_before
      + result.reaction_energy_after;
  result.exact_completed_energy_ledger =
      result.ready_reaction_slice
      && close(result.completed_energy_residual, 0.0, input.tolerance);

  result.self_dual_channel_symmetry_selected =
      close(input.split_angle, kPi / 4.0, input.tolerance);
  result.equal_history_reaction_energy =
      result.ready_reaction_slice
      && close(
          result.history_energy_after,
          result.reaction_energy_after,
          input.tolerance);
  return result;
}

}  // namespace ftd::eft

