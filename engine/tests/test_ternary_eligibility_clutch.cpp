/** FTD-0867 isolated ternary eligibility clutch/handshake verifier. */

#include "ftd/eft/ternary_eligibility_clutch.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
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

TernaryEligibilityClutchInput registered_input(std::int8_t latch) {
  TernaryEligibilityClutchInput input;
  input.latch = latch;
  input.orientation_reference = {3.0, 4.0};
  input.clock_state.reference_phase = 0.0;
  input.clock_state.reference_action = 8.0;
  input.clock_parameters.clock_frequency = 2.0;
  input.clock_parameters.common_frequency = 2.0;
  input.clock_parameters.coupling = 1.0;
  input.clock_parameters.tolerance = 1e-11;
  return input;
}

CanonicalCarrierPair signed_event_mode(std::int8_t sign, double energy) {
  const double amplitude = static_cast<double>(sign) * std::sqrt(2.0 * energy);
  return {-0.8 * amplitude, 0.6 * amplitude};
}

}  // namespace

int main() {
  const double event_energy = 1.75;
  for (const std::int8_t sign : {std::int8_t{-1}, std::int8_t{1}}) {
    auto input = registered_input(sign);
    input.clock_state.matter = signed_event_mode(sign, event_energy);
    input.clock_state.signal = {0.0, 0.0};
    const auto result = execute_ternary_eligibility_handshake(input);
    check("signed event handshake is valid", result.valid());
    check("ternary square derives exchange eligibility",
        result.latch_square_is_eligibility
        && result.derived_eligibility == RecordPortEligibility::Exchange);
    check("registered active branch is exact swap",
        result.clock_cycle.valid() && result.clock_cycle.exact_swap);
    check("complete signed mode moves into outgoing signal",
        same_pair(result.clock_cycle.after.matter, {0.0, 0.0})
        && same_pair(result.clock_cycle.after.signal, input.clock_state.matter));
    check("outgoing signal decodes original latch sign",
        result.decoded_latch == sign);
    check("outgoing signal decodes event energy once",
        close(result.event_energy_before, event_energy)
        && close(result.decoded_event_energy, event_energy));
    check("clutch release is requested only after gate-zero exchange",
        result.clutch_release_requested
        && result.requested_latch_after == 0
        && result.clutch_release_at_gate_zero
        && close(result.clutch_switch_work_at_release, 0.0));
    check("declared event survives requested local reset",
        result.declared_event_recoverable_after_requested_reset);
    check("unreleased active clutch would undo on next cycle",
        result.second_active_cycle_would_undo_exchange);
    check("physical controller debts remain explicit",
        !result.microscopic_latch_reset_supplied
        && !result.autonomous_acknowledgement_supplied
        && !result.clock_synchronization_supplied
        && !result.cubic_production_coupling_supplied);
  }

  auto hold_input = registered_input(0);
  hold_input.clock_state.matter = {0.0, 0.0};
  hold_input.clock_state.signal = {0.0, 0.0};
  const auto held = execute_ternary_eligibility_handshake(hold_input);
  check("zero latch selects valid exact hold",
      held.valid()
      && held.derived_eligibility == RecordPortEligibility::Hold
      && held.clock_cycle.exact_hold
      && held.decoded_latch == 0
      && close(held.decoded_event_energy, 0.0));

  auto bad_latch = hold_input;
  bad_latch.latch = 2;
  check("nonternary latch fails closed",
      execute_ternary_eligibility_handshake(bad_latch).status
          == TernaryEligibilityClutchStatus::InvalidLatch);

  auto bad_reference = hold_input;
  bad_reference.orientation_reference = {0.0, 0.0};
  check("zero orientation reference fails closed",
      execute_ternary_eligibility_handshake(bad_reference).status
          == TernaryEligibilityClutchStatus::InvalidOrientationReference);

  auto bad_hold = hold_input;
  bad_hold.clock_state.matter = {0.1, 0.0};
  check("zero latch rejects a nonempty event preparation",
      execute_ternary_eligibility_handshake(bad_hold).status
          == TernaryEligibilityClutchStatus::InvalidHoldPreparation);

  auto bad_signal = registered_input(1);
  bad_signal.clock_state.matter = signed_event_mode(1, event_energy);
  bad_signal.clock_state.signal = {0.1, 0.0};
  check("active one-shot domain rejects nonzero incoming signal",
      execute_ternary_eligibility_handshake(bad_signal).status
          == TernaryEligibilityClutchStatus::InvalidActiveSignal);

  auto sign_mismatch = registered_input(-1);
  sign_mismatch.clock_state.matter = signed_event_mode(1, event_energy);
  check("latch and oriented matter sign mismatch fails closed",
      execute_ternary_eligibility_handshake(sign_mismatch).status
          == TernaryEligibilityClutchStatus::InvalidMatterOrientation);

  auto parallel_matter = registered_input(1);
  parallel_matter.clock_state.matter = {1.0, 4.0 / 3.0};
  check("reference-parallel matter content fails closed",
      execute_ternary_eligibility_handshake(parallel_matter).status
          == TernaryEligibilityClutchStatus::InvalidMatterOrientation);

  auto insufficient = registered_input(1);
  insufficient.clock_state.matter = signed_event_mode(1, event_energy);
  insufficient.clock_state.reference_action = event_energy / 2.0;
  const auto insufficient_result = execute_ternary_eligibility_handshake(insufficient);
  check("inherited strict reference reserve boundary fails closed",
      insufficient_result.status
          == TernaryEligibilityClutchStatus::ClockCycleRejected
      && insufficient_result.clock_cycle.status
          == ClockGatedHamiltonianStatus::InsufficientReferenceReserve);

  auto wrong_winding = registered_input(1);
  wrong_winding.clock_state.matter = signed_event_mode(1, event_energy);
  wrong_winding.clock_parameters.coupling = 0.7;
  check("nonregistered winding is not promoted to handshake",
      execute_ternary_eligibility_handshake(wrong_winding).status
          == TernaryEligibilityClutchStatus::ClockCycleRejected);

  auto bad_tolerance = hold_input;
  bad_tolerance.clock_parameters.tolerance =
      std::numeric_limits<double>::quiet_NaN();
  check("nonfinite tolerance fails closed",
      execute_ternary_eligibility_handshake(bad_tolerance).status
          == TernaryEligibilityClutchStatus::InvalidTolerance);

  std::cout << "FTD-0867 ternary eligibility clutch EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "eligibility=TERNARY_SQUARE_HOLD_EXCHANGE\n";
  std::cout << "release=GATE_ZERO_ZERO_CLUTCH_WORK\n";
  std::cout << "event_record=SIGN_AND_ENERGY_RETAINED_IN_SIGNAL\n";
  std::cout << "autonomous_ack_reset_bath_sync_production=OPEN\n";
  return failures == 0 ? 0 : 1;
}
