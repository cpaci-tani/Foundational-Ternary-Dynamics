/** FTD-0871 isolated reversible ternary signal-uncomputation verifier. */

#include "ftd/eft/reversible_ternary_signal_uncomputation.h"

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

CanonicalCarrierPair signed_signal(std::int8_t sign, double energy) {
  const double amplitude = static_cast<double>(sign) * std::sqrt(2.0 * energy);
  return {-0.8 * amplitude, 0.6 * amplitude};
}

TernarySignalUncomputationInput registered_input(
    std::int8_t latch,
    double energy) {
  TernarySignalUncomputationInput input;
  input.latch = latch;
  input.orientation_reference = {3.0, 4.0};
  if (latch != 0) input.completed_signal = signed_signal(latch, energy);
  input.tolerance = 1e-11;
  return input;
}

}  // namespace

int main() {
  const double event_energy = 1.75;
  for (const std::int8_t sign : {std::int8_t{-1}, std::int8_t{1}}) {
    const auto input = registered_input(sign, event_energy);
    const auto result = execute_reversible_ternary_signal_uncomputation(input);
    check("signed registered uncomputation is valid", result.valid());
    check("completed signal supplies acknowledgement without another bit",
        result.signal_completion_acknowledged
        && result.no_extra_acknowledgement_bit);
    check("Z3 subtraction resets the matching actual latch",
        result.latch_after_uncomputation == 0);
    check("Z3 addition exactly reconstructs the original latch",
        result.ternary_group_bijection_verified
        && result.reversible_uncomputation_verified
        && result.inverse_recovered_latch == sign);
    check("signal workspace is unchanged before handoff",
        same_pair(result.signal_after_uncomputation, input.completed_signal)
        && close(result.signal_energy_before, event_energy)
        && close(result.signal_energy_after, event_energy)
        && close(result.signal_energy_residual, 0.0));
    check("empty output handoff returns the local actual state ready",
        result.output_handoff_reciprocal
        && result.local_actual_state_ready
        && same_pair(result.local_signal_after_handoff, {0.0, 0.0})
        && same_pair(result.exported_signal, input.completed_signal)
        && close(result.decoded_output_energy, event_energy));
    check("logical reset adds neither a reset trit nor scalar bath",
        result.no_reset_history_trit
        && result.no_logical_bath_required
        && close(result.logical_bath_energy, 0.0)
        && close(result.endpoint_latch_storage_energy_difference, 0.0));
    check("sign reversal covariance is retained",
        result.sign_reversal_equivariant
        && result.decoded_signal_sign == sign);
    check("scope flags preserve the physical realization debts",
        !result.continuous_latch_reset_supplied
        && !result.controller_work_ledger_supplied
        && !result.protected_cubic_transport_supplied
        && !result.production_coupling_supplied
        && !result.native_gstar_synchronization_supplied);
  }

  const auto hold = execute_reversible_ternary_signal_uncomputation(
      registered_input(0, 0.0));
  check("no-event zero state remains ready without acknowledgement",
      hold.valid()
      && !hold.signal_completion_acknowledged
      && hold.latch_after_uncomputation == 0
      && hold.local_actual_state_ready);

  auto mismatch = registered_input(-1, event_energy);
  mismatch.completed_signal = signed_signal(1, event_energy);
  check("signal and latch sign mismatch fails closed",
      execute_reversible_ternary_signal_uncomputation(mismatch).status
          == TernarySignalUncomputationStatus::SignalLatchMismatch);

  auto missing_signal = registered_input(1, event_energy);
  missing_signal.completed_signal = {0.0, 0.0};
  check("active latch without a completed signal fails closed",
      execute_reversible_ternary_signal_uncomputation(missing_signal).status
          == TernarySignalUncomputationStatus::InvalidSignal);

  auto parallel_signal = registered_input(1, event_energy);
  parallel_signal.completed_signal = {3.0, 4.0};
  check("unoriented parallel signal fails closed",
      execute_reversible_ternary_signal_uncomputation(parallel_signal).status
          == TernarySignalUncomputationStatus::InvalidSignal);

  auto occupied_output = registered_input(1, event_energy);
  occupied_output.output_port = {0.1, 0.0};
  check("occupied output backpressure fails closed",
      execute_reversible_ternary_signal_uncomputation(occupied_output).status
          == TernarySignalUncomputationStatus::NonemptyOutputPort);

  auto invalid_latch = registered_input(0, 0.0);
  invalid_latch.latch = 2;
  check("nonternary latch fails closed",
      execute_reversible_ternary_signal_uncomputation(invalid_latch).status
          == TernarySignalUncomputationStatus::InvalidLatch);

  auto invalid_reference = registered_input(0, 0.0);
  invalid_reference.orientation_reference = {0.0, 0.0};
  check("zero orientation reference fails closed",
      execute_reversible_ternary_signal_uncomputation(invalid_reference).status
          == TernarySignalUncomputationStatus::InvalidReference);

  auto invalid_tolerance = registered_input(0, 0.0);
  invalid_tolerance.tolerance = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite tolerance fails closed",
      execute_reversible_ternary_signal_uncomputation(invalid_tolerance).status
          == TernarySignalUncomputationStatus::InvalidTolerance);

  std::cout << "FTD-0871 reversible ternary signal uncomputation EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "actual_reset=REVERSIBLE_Z3_SIGNAL_UNCOMPUTATION\n";
  std::cout << "extra_ack_reset_trit_logical_bath=NOT_REQUIRED\n";
  std::cout << "continuous_controller_transport_production_gstar=OPEN\n";
  return failures == 0 ? 0 : 1;
}
