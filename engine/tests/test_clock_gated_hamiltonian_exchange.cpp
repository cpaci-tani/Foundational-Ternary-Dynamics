/** FTD-0865 isolated clock-gated Hamiltonian exchange verifier. */

#include "ftd/eft/clock_gated_hamiltonian_exchange.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

using namespace ftd::eft;

int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

bool close(double first, double second, double tolerance = 2e-11) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

bool same_pair(
    const CanonicalCarrierPair& first,
    const CanonicalCarrierPair& second) {
  return close(first.q, second.q) && close(first.p, second.p);
}

double action(const CanonicalCarrierPair& pair) {
  return 0.5 * (pair.q * pair.q + pair.p * pair.p);
}

ClockGatedHamiltonianParameters registered_parameters(
    RecordPortEligibility eligibility) {
  ClockGatedHamiltonianParameters parameters;
  parameters.clock_frequency = 2.0;
  parameters.common_frequency = 2.0;
  parameters.coupling = 1.0;
  parameters.eligibility = eligibility;
  parameters.tolerance = 1e-12;
  return parameters;
}

}  // namespace

int main() {
  ClockGatedHamiltonianState emission_state;
  emission_state.reference_phase = 0.0;
  emission_state.reference_action = 8.0;
  emission_state.matter = {1.2, -0.7};
  emission_state.signal = {0.0, 0.0};
  const double event_energy = action(emission_state.matter);

  const auto exchanged = evolve_clock_gated_hamiltonian_cycle(
      emission_state,
      registered_parameters(RecordPortEligibility::Exchange));
  check("registered active cycle is valid", exchanged.valid());
  check("registered active winding is an exact swap", exchanged.exact_swap);
  check("emission empties matter mode",
      same_pair(exchanged.after.matter, {0.0, 0.0}));
  check("emission transfers complete canonical mode",
      same_pair(exchanged.after.signal, emission_state.matter));
  check("emission signal energy equals event energy",
      close(action(exchanged.after.signal), event_energy));
  check("relative action is half empty-signal event energy",
      close(exchanged.relative_action, event_energy / 2.0));
  check("minimum reference action books exact reserve",
      close(
          exchanged.minimum_reference_action,
          emission_state.reference_action - event_energy / 2.0));
  check("reference action returns after complete cycle",
      close(exchanged.after.reference_action, emission_state.reference_action));
  check("reference phase advances one full cycle",
      close(exchanged.after.reference_phase, 2.0 * std::acos(-1.0)));
  check("interaction energy equals reference energy loan",
      close(
          exchanged.maximum_interaction_energy,
          exchanged.maximum_reference_energy_loan));
  check("endpoint Hamiltonian closes",
      close(exchanged.endpoint_energy_residual, 0.0));
  check("mode action closes",
      close(exchanged.mode_action_before, exchanged.mode_action_after));

  ClockGatedHamiltonianState arbitrary_state = emission_state;
  arbitrary_state.matter = {-0.4, 1.1};
  arbitrary_state.signal = {0.8, -0.3};
  const auto arbitrary_swap = evolve_clock_gated_hamiltonian_cycle(
      arbitrary_state,
      registered_parameters(RecordPortEligibility::Exchange));
  check("arbitrary two-mode active cycle is valid", arbitrary_swap.valid());
  check("arbitrary active cycle swaps matter and signal",
      same_pair(arbitrary_swap.after.matter, arbitrary_state.signal)
      && same_pair(arbitrary_swap.after.signal, arbitrary_state.matter));

  const auto swapped_twice = evolve_clock_gated_hamiltonian_cycle(
      arbitrary_swap.after,
      registered_parameters(RecordPortEligibility::Exchange));
  check("Hamiltonian swap is reciprocal and involutive stroboscopically",
      swapped_twice.valid()
      && same_pair(swapped_twice.after.matter, arbitrary_state.matter)
      && same_pair(swapped_twice.after.signal, arbitrary_state.signal));

  const auto held = evolve_clock_gated_hamiltonian_cycle(
      arbitrary_state,
      registered_parameters(RecordPortEligibility::Hold));
  check("registered inactive cycle is valid exact hold",
      held.valid() && held.exact_hold);
  check("hold leaves both complete modes unchanged",
      same_pair(held.after.matter, arbitrary_state.matter)
      && same_pair(held.after.signal, arbitrary_state.signal));
  check("hold requires no reference reserve loan",
      close(held.minimum_reference_action, arbitrary_state.reference_action)
      && close(held.maximum_reference_energy_loan, 0.0));

  ClockGatedHamiltonianState absorption_state = emission_state;
  absorption_state.matter = {0.0, 0.0};
  absorption_state.signal = emission_state.matter;
  const auto absorbed = evolve_clock_gated_hamiltonian_cycle(
      absorption_state,
      registered_parameters(RecordPortEligibility::Exchange));
  check("same cycle absorbs signal into matter",
      absorbed.valid()
      && same_pair(absorbed.after.matter, absorption_state.signal)
      && same_pair(absorbed.after.signal, {0.0, 0.0}));

  for (const double amplitude : {0.25, 1.0, 2.5}) {
    ClockGatedHamiltonianState load = emission_state;
    load.reference_action = 20.0;
    load.matter = {amplitude, -0.3 * amplitude};
    const auto result = evolve_clock_gated_hamiltonian_cycle(
        load,
        registered_parameters(RecordPortEligibility::Exchange));
    check("harmonic registered winding is exact for held-out load",
        result.valid()
        && result.exact_swap
        && same_pair(result.after.signal, load.matter));
  }

  ClockGatedHamiltonianState insufficient = emission_state;
  insufficient.reference_action = event_energy / 2.0;
  check("insufficient strict reference reserve fails closed",
      evolve_clock_gated_hamiltonian_cycle(
          insufficient,
          registered_parameters(RecordPortEligibility::Exchange)).status
          == ClockGatedHamiltonianStatus::InsufficientReferenceReserve);

  ClockGatedHamiltonianState wrong_phase = emission_state;
  wrong_phase.reference_phase = 0.2;
  check("cycle must start at gate-zero phase",
      evolve_clock_gated_hamiltonian_cycle(
          wrong_phase,
          registered_parameters(RecordPortEligibility::Exchange)).status
          == ClockGatedHamiltonianStatus::InvalidReferencePhase);

  auto nonregistered = registered_parameters(RecordPortEligibility::Exchange);
  nonregistered.coupling = 0.7;
  const auto nonregistered_result = evolve_clock_gated_hamiltonian_cycle(
      emission_state,
      nonregistered);
  check("nonregistered exact flow remains valid but is not called a swap",
      nonregistered_result.valid() && !nonregistered_result.exact_swap);

  auto invalid_frequency = registered_parameters(RecordPortEligibility::Hold);
  invalid_frequency.clock_frequency = 0.0;
  check("nonpositive clock frequency fails closed",
      evolve_clock_gated_hamiltonian_cycle(
          emission_state,
          invalid_frequency).status
          == ClockGatedHamiltonianStatus::InvalidClockFrequency);
  auto invalid_coupling = registered_parameters(RecordPortEligibility::Hold);
  invalid_coupling.coupling = -1.0;
  check("negative coupling fails closed",
      evolve_clock_gated_hamiltonian_cycle(
          emission_state,
          invalid_coupling).status
          == ClockGatedHamiltonianStatus::InvalidCoupling);
  auto invalid_eligibility = registered_parameters(RecordPortEligibility::Hold);
  invalid_eligibility.eligibility = static_cast<RecordPortEligibility>(9);
  check("invalid eligibility fails closed",
      evolve_clock_gated_hamiltonian_cycle(
          emission_state,
          invalid_eligibility).status
          == ClockGatedHamiltonianStatus::InvalidEligibility);
  ClockGatedHamiltonianState invalid_mode = emission_state;
  invalid_mode.signal.p = std::numeric_limits<double>::infinity();
  check("nonfinite signal mode fails closed",
      evolve_clock_gated_hamiltonian_cycle(
          invalid_mode,
          registered_parameters(RecordPortEligibility::Hold)).status
          == ClockGatedHamiltonianStatus::InvalidSignalMode);

  check("eligibility remains frozen rather than dynamically derived",
      exchanged.valid() && !exchanged.dynamic_eligibility_supplied);
  check("quartic load-blind controller remains unestablished",
      exchanged.valid() && !exchanged.quartic_load_blind_controller_established);

  std::cout << "FTD-0865 clock-gated Hamiltonian exchange EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "scope=AUTONOMOUS_HARMONIC_REFERENCE_EXACT_CYCLE\n";
  std::cout << "active_branch=FULL_CANONICAL_MODE_SWAP\n";
  std::cout << "reference_backreaction=TRANSIENT_RESERVE_EXACT_RETURN\n";
  std::cout << "quartic_load_blind_swap=SCOPED_CLOSED_NEGATIVE\n";
  std::cout << "dynamic_eligibility_gstar_production=OPEN\n";
  return failures == 0 ? 0 : 1;
}
