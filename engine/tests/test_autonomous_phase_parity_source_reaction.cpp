/** FTD-0887/0888 autonomous phase-parity/source-reaction EFT verifier. */

#include "ftd/eft/autonomous_phase_parity_source_reaction.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;

int checks = 0;
int failures = 0;

void check(const std::string& label, bool condition) {
  ++checks;
  if (!condition) {
    ++failures;
    std::cerr << "FAIL  " << label << '\n';
  }
}

bool close(double first, double second, double tolerance = 1e-11) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

}  // namespace

int main() {
  using namespace ftd::eft;

  AutonomousPhaseParityInput self_dual;
  self_dual.mode.residual = 2.0;
  self_dual.source_offset = 1.25;
  self_dual.clock_frequency = 3.0;
  self_dual.reference_action = 20.0;
  const auto result =
      evolve_autonomous_phase_parity_source_reaction_cycle(self_dual);
  check("self-dual source-reaction cycle is valid", result.valid());
  check("six windows compile color zero before color one",
      result.pulses[0].checkerboard_color == 0
      && result.pulses[2].checkerboard_color == 0
      && result.pulses[3].checkerboard_color == 1
      && result.pulses[5].checkerboard_color == 1
      && result.pulses[0].kind
          == AutonomousPhasePulseKind::ResidualHistory
      && result.pulses[1].kind
          == AutonomousPhasePulseKind::HistoryReaction
      && result.pulses[2].kind
          == AutonomousPhasePulseKind::ReactionPhase);
  check("window timing and integrated angles are exact",
      close(result.window_duration, kPi / 9.0)
      && close(result.cycle_duration, 2.0 * kPi / 3.0)
      && close(result.base_winding_per_window, 2.0 * kPi)
      && close(result.pulses[0].window_integral, kPi / 6.0)
      && close(result.pulses[0].pulse_coefficient, 3.0)
      && close(result.pulses[0].target_angle, kPi / 2.0));
  check("controller is autonomous positive and boundary closed",
      result.autonomous_hamiltonian
      && !result.external_integer_parity_switch_required
      && result.phase_windows_c1
      && result.phase_window_interiors_disjoint
      && result.action_returns_at_every_boundary
      && result.positive_carrier_hamiltonian
      && close(result.reference_action_before, result.reference_action_after)
      && close(
          result.carrier_hamiltonian_lower_bound,
          3.0 * self_dual.clock_frequency * result.common_norm_before));
  check("ready slice clears the residual and resets reaction displacement",
      result.ready_reaction_slice
      && result.gauss_residual_cleared
      && result.reaction_displacement_reset
      && close(result.after.residual, 0.0)
      && close(result.after.residual_conjugate, 0.0)
      && close(result.after.reaction, 0.0));
  check("source reaction receives the selected impulse",
      result.reaction_impulse_generated
      && close(result.source_reaction_impulse, std::sqrt(2.0))
      && close(result.after.history, -std::sqrt(2.0)));
  check("self-dual history and reaction energies are equal",
      result.self_dual_channel_symmetry_selected
      && result.equal_history_reaction_energy
      && result.exact_history_reaction_split
      && close(result.residual_energy_before, 2.0)
      && close(result.history_energy_after, 1.0)
      && close(result.reaction_energy_after, 1.0));
  check("completed source interaction and reaction ledger is exact",
      result.exact_completed_energy_ledger
      && close(result.completed_energy_residual, 0.0)
      && close(
          result.raw_energy_after - result.raw_energy_before,
          result.old_source_work - result.reaction_energy_after)
      && close(
          result.interaction_energy_after
              - result.interaction_energy_before,
          -result.old_source_work));
  check("exact inverse and common norm close",
      result.exact_inverse_verified
      && result.endpoint_symplectic
      && result.endpoint_orthogonal
      && result.endpoint_orientation_preserving
      && close(result.common_norm_before, result.common_norm_after));
  check("scope and minimum-pair boundary remain explicit",
      result.history_only_endpoint_energy_saturated
      && result.one_canonical_reaction_pair_minimum_in_registered_class
      && !result.spatial_ternary_source_recoil_supplied
      && !result.production_coupling_supplied
      && !result.native_gstar_synchronization_supplied
      && !result.born_target_used
      && !result.new_selected_type_added);

  AutonomousPhaseParityInput history_only = self_dual;
  history_only.split_angle = 0.0;
  const auto history_result =
      evolve_autonomous_phase_parity_source_reaction_cycle(history_only);
  check("eta zero reproduces the history-only FTD0886 endpoint",
      history_result.valid()
      && history_result.gauss_residual_cleared
      && close(history_result.after.history, -2.0)
      && close(history_result.after.reaction, 0.0)
      && close(history_result.after.reaction_conjugate, 0.0)
      && close(history_result.history_energy_after, 2.0)
      && close(history_result.reaction_energy_after, 0.0));

  AutonomousPhaseParityInput generic = self_dual;
  generic.mode.residual = 0.7;
  generic.mode.history = -0.4;
  generic.mode.reaction = 0.2;
  generic.mode.residual_conjugate = 0.3;
  generic.mode.history_conjugate = -0.6;
  generic.mode.reaction_conjugate = 0.5;
  generic.split_angle = kPi / 6.0;
  generic.reference_action = 30.0;
  const auto generic_result =
      evolve_autonomous_phase_parity_source_reaction_cycle(generic);
  check("generic continuous phase state reverses exactly",
      generic_result.valid()
      && generic_result.exact_inverse_verified
      && close(generic_result.recovered.residual, generic.mode.residual)
      && close(generic_result.recovered.history, generic.mode.history)
      && close(generic_result.recovered.reaction, generic.mode.reaction)
      && close(
          generic_result.recovered.reaction_conjugate,
          generic.mode.reaction_conjugate));

  auto bad_angle = self_dual;
  bad_angle.split_angle = kPi;
  check("out-of-range split angle fails closed",
      evolve_autonomous_phase_parity_source_reaction_cycle(bad_angle).status
          == AutonomousPhaseParityStatus::InvalidSplitAngle);
  auto bad_frequency = self_dual;
  bad_frequency.clock_frequency = 0.0;
  check("nonpositive frequency fails closed",
      evolve_autonomous_phase_parity_source_reaction_cycle(bad_frequency).status
          == AutonomousPhaseParityStatus::InvalidClockFrequency);
  auto off_phase = self_dual;
  off_phase.reference_phase = 0.25;
  check("off-origin phase fails closed",
      evolve_autonomous_phase_parity_source_reaction_cycle(off_phase).status
          == AutonomousPhaseParityStatus::InvalidReferencePhase);
  auto low_reserve = self_dual;
  low_reserve.reference_action = 1.0;
  check("insufficient clock reserve fails closed",
      evolve_autonomous_phase_parity_source_reaction_cycle(low_reserve).status
          == AutonomousPhaseParityStatus::InsufficientReferenceReserve);
  auto nonfinite = self_dual;
  nonfinite.mode.reaction = std::numeric_limits<double>::infinity();
  check("nonfinite mode fails closed",
      evolve_autonomous_phase_parity_source_reaction_cycle(nonfinite).status
          == AutonomousPhaseParityStatus::NonFiniteInput);

  std::cout << "FTD-0887/0888 autonomous phase parity/source reaction EFT: "
            << (checks - failures) << '/' << checks << " PASS\n";
  std::cout << "parity_controller=AUTONOMOUS_PHASE_REFERENCE\n";
  std::cout << "reaction_split=POSITIVE_COMPLETE_PAIR\n";
  std::cout << "spatial_source_gstar_production=OPEN\n";
  return failures == 0 ? 0 : 1;
}

