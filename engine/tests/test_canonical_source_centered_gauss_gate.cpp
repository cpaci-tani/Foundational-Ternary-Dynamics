/** FTD-0885/0886 canonical source-centered Gauss-gate EFT verifier. */

#include "ftd/eft/canonical_source_centered_gauss_gate.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

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

  CanonicalSourceCenteredGaussInput actual;
  actual.mode.field_normal = 2.5;
  actual.mode.port = -0.75;
  actual.mode.field_conjugate = 0.0;
  actual.mode.port_conjugate = 0.0;
  actual.mode.source_offset = 1.25;
  actual.clock_frequency = 3.0;
  actual.reference_action = 5.0;
  const auto actual_result =
      evolve_canonical_source_centered_gauss_cycle(actual);
  check("actual-section canonical cycle is valid", actual_result.valid());
  check("forward endpoint is the frozen residual-port quarter-turn",
      close(actual_result.residual_after, actual.mode.port)
      && close(
          actual_result.after.port,
          -(actual.mode.field_normal - actual.mode.source_offset))
      && actual_result.frozen_gauss_configuration_gate_reproduced);
  check("zero-conjugate section is invariant",
      actual_result.zero_conjugate_section
      && actual_result.zero_conjugate_section_preserved
      && close(actual_result.after.field_conjugate, 0.0)
      && close(actual_result.after.port_conjugate, 0.0));
  check("positive centered Hamiltonian and exact inverse close",
      actual_result.positive_source_centered_hamiltonian
      && actual_result.exact_inverse_verified
      && close(
          actual_result.carrier_norm_before,
          actual_result.carrier_norm_after)
      && close(actual_result.endpoint_hamiltonian_residual, 0.0));
  const double expected_work = actual.mode.source_offset
      * (actual.mode.port
          - (actual.mode.field_normal - actual.mode.source_offset));
  check("raw source work is exactly interaction-energy exchange",
      close(actual_result.raw_source_work, expected_work)
      && close(actual_result.interaction_work, -expected_work)
      && close(actual_result.source_work_residual, 0.0)
      && actual_result.raw_work_is_interaction_energy_exchange);
  check("scope flags remain explicit",
      !actual_result.source_offset_dynamical
      && !actual_result.autonomous_parity_controller_supplied
      && !actual_result.production_coupling_supplied
      && !actual_result.native_gstar_synchronization_supplied
      && !actual_result.born_target_used
      && !actual_result.new_selected_type_added);

  CanonicalSourceCenteredGaussInput generic = actual;
  generic.mode.field_conjugate = 0.4;
  generic.mode.port_conjugate = -0.6;
  generic.reference_action = 10.0;
  const auto generic_forward =
      evolve_canonical_source_centered_gauss_cycle(generic);
  check("generic phase-space cycle is positive symplectic and oriented",
      generic_forward.valid()
      && generic_forward.endpoint_symplectic
      && generic_forward.endpoint_orthogonal
      && generic_forward.endpoint_orientation_preserving
      && generic_forward.angular_momentum != 0.0
      && generic_forward.minimum_reference_action > 0.0);
  check("generic cycle has exact clock-action endpoint ledger",
      close(
          generic_forward.reference_action_before,
          generic_forward.reference_action_after)
      && close(
          generic_forward.maximum_clock_action_excursion,
          0.5 * std::abs(generic_forward.angular_momentum))
      && close(
          generic_forward.pulse_angle,
          kPi / 2.0));

  CanonicalSourceCenteredGaussInput reverse;
  reverse.mode = generic_forward.after;
  reverse.orientation = TernaryQuarterTurnOrientation::Reverse;
  reverse.clock_frequency = generic.clock_frequency;
  reverse.reference_action = generic.reference_action;
  const auto reverse_result =
      evolve_canonical_source_centered_gauss_cycle(reverse);
  check("public opposite cycle recovers the generic input",
      reverse_result.valid()
      && close(reverse_result.after.field_normal, generic.mode.field_normal)
      && close(reverse_result.after.port, generic.mode.port)
      && close(
          reverse_result.after.field_conjugate,
          generic.mode.field_conjugate)
      && close(
          reverse_result.after.port_conjugate,
          generic.mode.port_conjugate));

  const auto battery_slice =
      audit_square_root_battery_phase_completion(4.0, 0.0, 1.5);
  check("square-root battery is exact on its Lagrangian section",
      battery_slice.valid()
      && battery_slice.lagrangian_section
      && battery_slice.cotangent_lift_symplectic
      && battery_slice.exact_work_ledger
      && battery_slice.sign_preserved);
  const auto battery_generic =
      audit_square_root_battery_phase_completion(4.0, 2.0, 1.5);
  check("phase-complete battery exposes the extra energy change",
      battery_generic.valid()
      && !battery_generic.lagrangian_section
      && battery_generic.cotangent_lift_symplectic
      && !battery_generic.exact_work_ledger
      && !close(battery_generic.energy_change_residual, 0.0));
  check("nonzero action translation is not globally Hamiltonian",
      battery_generic.constant_action_translation_locally_symplectic
      && !battery_generic.constant_action_translation_globally_hamiltonian
      && close(battery_generic.phase_circle_flux, -3.0 * kPi)
      && !battery_generic.phase_blind_state_dependent_drain_symplectic
      && !battery_generic.square_root_law_promoted_to_physical_reservoir);
  const auto zero_work =
      audit_square_root_battery_phase_completion(4.0, 2.0, 0.0);
  check("zero-work control has zero cylinder flux and exact energy",
      zero_work.valid()
      && zero_work.constant_action_translation_globally_hamiltonian
      && close(zero_work.phase_circle_flux, 0.0)
      && zero_work.exact_work_ledger);

  const std::vector<CanonicalHistoryPort> rail = {
      {1.0, -0.25},
      {-2.0, 0.5},
      {0.75, 1.25},
  };
  const CanonicalHistoryPort incoming{-0.5, 0.125};
  const auto shifted = shift_open_canonical_history_right(rail, incoming);
  check("open canonical history shifts the complete pair",
      shifted.valid()
      && shifted.complete_pair_shifted
      && close(shifted.after.front().coordinate, incoming.coordinate)
      && close(shifted.after.front().conjugate, incoming.conjugate)
      && close(shifted.outgoing.coordinate, rail.back().coordinate)
      && close(shifted.outgoing.conjugate, rail.back().conjugate));
  check("open pair shift has exact inverse and energy ledger",
      shifted.symplectic_with_boundaries
      && shifted.exact_inverse_verified
      && close(shifted.open_energy_residual, 0.0)
      && !shifted.scalar_energy_only_export_sufficient
      && !shifted.finite_closed_recycler_claimed);

  auto bad_frequency = actual;
  bad_frequency.clock_frequency = 0.0;
  check("nonpositive clock frequency fails closed",
      evolve_canonical_source_centered_gauss_cycle(bad_frequency).status
          == CanonicalSourceCenteredGaussStatus::InvalidClockFrequency);
  auto off_phase = actual;
  off_phase.reference_phase = 0.25;
  check("off-phase gate fails closed",
      evolve_canonical_source_centered_gauss_cycle(off_phase).status
          == CanonicalSourceCenteredGaussStatus::InvalidReferencePhase);
  auto low_reserve = generic;
  low_reserve.reference_action = 1e-15;
  check("insufficient clock reserve fails closed",
      evolve_canonical_source_centered_gauss_cycle(low_reserve).status
          == CanonicalSourceCenteredGaussStatus::InsufficientReferenceReserve);
  auto nonfinite = actual;
  nonfinite.mode.port = std::numeric_limits<double>::infinity();
  check("nonfinite canonical mode fails closed",
      evolve_canonical_source_centered_gauss_cycle(nonfinite).status
          == CanonicalSourceCenteredGaussStatus::NonFiniteInput);
  check("depleted battery reserve fails closed",
      audit_square_root_battery_phase_completion(1.0, 0.0, 1.0).status
          == SquareRootBatteryPhaseStatus::ReserveDepleted);
  check("empty history rail fails closed",
      shift_open_canonical_history_right({}, incoming).status
          == OpenCanonicalHistoryStatus::EmptyRail);

  std::cout << "FTD-0885/0886 canonical source-centered Gauss gate EFT: "
            << (checks - failures) << '/' << checks << " PASS\n";
  std::cout << "source_work=INTERACTION_ENERGY_EXCHANGE\n";
  std::cout << "square_root_battery=LAGRANGIAN_SECTION_ONLY\n";
  std::cout << "canonical_history=COMPLETE_PAIR_OPEN_BOUNDARY\n";
  std::cout << "production_gstar=OPEN\n";
  return failures == 0 ? 0 : 1;
}
