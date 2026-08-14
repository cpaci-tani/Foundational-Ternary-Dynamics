/** FTD-0873 isolated Hamiltonian ternary quarter-turn actuator verifier. */

#include "ftd/eft/hamiltonian_ternary_quarter_turn_actuator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

namespace {

int checks = 0;
int failures = 0;

void check(const char* label, bool condition) {
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

ftd::eft::HamiltonianTernaryActuatorInput base_input() {
  ftd::eft::HamiltonianTernaryActuatorInput input;
  input.amplitude = 2.0;
  input.clock_frequency = 3.0;
  input.reference_action = 10.0;
  input.tolerance = 1e-12;
  return input;
}

}  // namespace

int main() {
  using namespace ftd::eft;

  for (std::int8_t latch = -1; latch <= 1; ++latch) {
    for (std::int8_t port = -1; port <= 1; ++port) {
      auto hold_input = base_input();
      hold_input.latch = latch;
      hold_input.port = port;
      const auto hold = evolve_hamiltonian_ternary_quarter_turn_cycle(hold_input);
      check("hold branch is valid", hold.valid());
      check("hold branch is exact", hold.exact_hold);
      check("hold continuous lift closes", hold.exact_hamiltonian_lift);
      check("hold endpoint energy closes", close(hold.endpoint_energy_residual, 0.0));

      auto forward_input = hold_input;
      forward_input.eligible = true;
      forward_input.orientation = TernaryQuarterTurnOrientation::Forward;
      const auto forward =
          evolve_hamiltonian_ternary_quarter_turn_cycle(forward_input);
      check("forward branch is valid", forward.valid());
      check("forward branch is exact", forward.exact_forward_quarter_turn);
      check("forward continuous lift closes", forward.exact_hamiltonian_lift);
      check("forward action is preserved", forward.carrier_action_preserved);
      check("forward endpoint energy closes", close(forward.endpoint_energy_residual, 0.0));

      auto reverse_input = forward_input;
      reverse_input.orientation = TernaryQuarterTurnOrientation::Reverse;
      const auto reverse =
          evolve_hamiltonian_ternary_quarter_turn_cycle(reverse_input);
      check("reverse branch is valid", reverse.valid());
      check("reverse branch is exact", reverse.exact_reverse_quarter_turn);
      check("reverse continuous lift closes", reverse.exact_hamiltonian_lift);
      check("reverse endpoint energy closes", close(reverse.endpoint_energy_residual, 0.0));
    }
  }

  auto emission_input = base_input();
  emission_input.latch = 1;
  emission_input.eligible = true;
  const auto emission =
      evolve_hamiltonian_ternary_quarter_turn_cycle(emission_input);
  check("ready emission reaches the output", emission.valid()
      && emission.logical_transfer.latch_after == 0
      && emission.logical_transfer.port_after == 1);
  check("imposed energy scale is explicit", close(emission.imposed_record_energy_scale, 6.0));
  check("record energy is transported without change",
      close(emission.record_energy_before, emission.record_energy_after));
  check("maximum action excursion is A over two",
      close(emission.maximum_clock_action_excursion,
            0.5 * emission.carrier_action_before));
  check("reference and interaction exchanges agree",
      close(emission.maximum_reference_energy_exchange,
            emission.maximum_interaction_energy_magnitude));
  check("gate-zero switch work vanishes", emission.gate_zero_switch_work == 0.0);
  check("off-phase switch work is booked",
      emission.antiphase_switch_work_magnitude > 0.0);
  check("controller exchange ledger is supplied",
      emission.controller_exchange_ledger_supplied
      && emission.gate_zero_switching_booked
      && emission.complete_cycle_net_work_zero);

  auto absorption_input = base_input();
  absorption_input.port = -1;
  absorption_input.eligible = true;
  absorption_input.orientation = TernaryQuarterTurnOrientation::Reverse;
  const auto absorption =
      evolve_hamiltonian_ternary_quarter_turn_cycle(absorption_input);
  check("reciprocal absorption restores the latch", absorption.valid()
      && absorption.logical_transfer.latch_after == -1
      && absorption.logical_transfer.port_after == 0);

  auto low_reserve = emission_input;
  low_reserve.reference_action = 1.0;
  const auto rejected_reserve =
      evolve_hamiltonian_ternary_quarter_turn_cycle(low_reserve);
  check("insufficient strict reserve fails closed",
      rejected_reserve.status
          == HamiltonianTernaryActuatorStatus::InsufficientReferenceReserve);

  auto off_phase = emission_input;
  off_phase.reference_phase = 0.5;
  const auto rejected_phase =
      evolve_hamiltonian_ternary_quarter_turn_cycle(off_phase);
  check("off-phase cycle request fails closed",
      rejected_phase.status
          == HamiltonianTernaryActuatorStatus::InvalidReferencePhase);

  auto invalid_amplitude = emission_input;
  invalid_amplitude.amplitude = 0.0;
  check("zero amplitude fails closed",
      evolve_hamiltonian_ternary_quarter_turn_cycle(invalid_amplitude).status
          == HamiltonianTernaryActuatorStatus::InvalidAmplitude);

  auto invalid_frequency = emission_input;
  invalid_frequency.clock_frequency = -1.0;
  check("negative clock frequency fails closed",
      evolve_hamiltonian_ternary_quarter_turn_cycle(invalid_frequency).status
          == HamiltonianTernaryActuatorStatus::InvalidClockFrequency);

  check("reference scale is not called native",
      emission.imposed_record_energy_scale_supplied
      && !emission.native_record_energy_scale_derived);
  check("one-shot scheduling remains open",
      !emission.dynamic_one_shot_scheduler_supplied
      && !emission.repeated_active_cycle_is_one_shot);
  check("production and Gstar remain open",
      !emission.protected_cubic_transport_supplied
      && !emission.production_coupling_supplied
      && !emission.native_gstar_synchronization_supplied);

  std::cout << "FTD-0873 Hamiltonian ternary quarter-turn EFT: "
            << (checks - failures) << '/' << checks << " PASS\n";
  return failures == 0 ? 0 : 1;
}

