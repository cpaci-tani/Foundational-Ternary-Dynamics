/** FTD-0869 isolated signal-acknowledged two-stroke reset verifier. */

#include "ftd/eft/signal_acknowledged_two_stroke_reset.h"

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

CanonicalCarrierPair signed_event_mode(std::int8_t sign, double energy) {
  const double amplitude = static_cast<double>(sign) * std::sqrt(2.0 * energy);
  return {-0.8 * amplitude, 0.6 * amplitude};
}

SignalAcknowledgedResetInput registered_input(
    std::int8_t latch,
    double event_energy,
    double reset_force_scale = 1.0) {
  SignalAcknowledgedResetInput input;
  input.latch = latch;
  input.latch_amplitude = 1.5;
  input.latch_coordinate = static_cast<double>(latch) * input.latch_amplitude;
  input.orientation_reference = {3.0, 4.0};
  input.reference_phase = 0.0;
  input.reference_action = 8.0;
  input.clock_frequency = 2.0;
  input.reset_drag = 0.7;
  const double minimum_force = input.reset_drag * input.latch_amplitude
      * input.clock_frequency / std::acos(-1.0);
  input.reset_force = reset_force_scale * minimum_force;
  input.tolerance = 1e-11;
  if (latch != 0) input.matter = signed_event_mode(latch, event_energy);
  return input;
}

}  // namespace

int main() {
  const double event_energy = 1.75;
  for (const std::int8_t sign : {std::int8_t{-1}, std::int8_t{1}}) {
    const auto input = registered_input(sign, event_energy);
    const auto result = execute_signal_acknowledged_two_stroke_reset(input);
    check("signed two-stroke cycle is valid", result.valid());
    check("first stroke performs exact matter-to-signal exchange",
        result.exchange_stroke_exact
        && same_pair(result.midpoint_matter, {0.0, 0.0})
        && same_pair(result.midpoint_signal, input.matter));
    check("completed local signal autonomously acknowledges",
        result.local_signal_acknowledged
        && result.acknowledgement_is_sign_even
        && result.no_extra_acknowledgement_bit);
    check("compressed exchange has exact stricter reserve",
        close(result.minimum_reference_action,
              input.reference_action - event_energy)
        && close(result.maximum_interaction_energy,
                 input.clock_frequency * event_energy)
        && close(result.maximum_reference_energy_loan,
                 input.clock_frequency * event_energy));
    check("minimum cusp force fills reset half exactly",
        result.reset_window_compliant
        && close(result.reset_time, result.reset_window)
        && close(result.reset_time_margin, 0.0));
    check("selected nonsmooth reset reaches exact zero",
        result.nonsmooth_finite_time_reset_selected
        && result.latch_after == 0
        && close(result.latch_coordinate_after, 0.0));
    check("reset work and scalar bath close",
        close(result.controller_energy_supplied,
              input.reset_force * input.latch_amplitude)
        && close(result.scalar_bath_energy_exported,
                 result.controller_energy_supplied)
        && result.scalar_bath_ledger_closed
        && close(result.reset_ledger_residual, 0.0));
    check("output port retains sign and event energy exactly",
        result.decoded_output_sign == sign
        && close(result.decoded_output_energy, event_energy)
        && close(result.event_energy_residual, 0.0)
        && same_pair(result.exported_signal, input.matter));
    check("local reference returns ready after output handoff",
        result.local_state_ready
        && same_pair(result.matter_after, {0.0, 0.0})
        && same_pair(result.signal_after, {0.0, 0.0}));
    check("scope flags preserve physical debts",
        !result.smooth_finite_time_reset_established
        && !result.microscopic_bath_state_supplied
        && !result.protected_cubic_transport_supplied
        && !result.native_gstar_synchronization_supplied
        && !result.production_latch_coupling_supplied);
  }

  const auto hold = execute_signal_acknowledged_two_stroke_reset(
      registered_input(0, 0.0));
  check("zero latch is a valid no-event ready cycle",
      hold.valid()
      && !hold.local_signal_acknowledged
      && !hold.nonsmooth_finite_time_reset_selected
      && hold.scalar_bath_ledger_closed
      && hold.local_state_ready
      && close(hold.controller_energy_supplied, 0.0));

  const auto early_reset = execute_signal_acknowledged_two_stroke_reset(
      registered_input(1, event_energy, 2.0));
  check("stronger cusp force resets early and sticks",
      early_reset.valid()
      && early_reset.reset_time < early_reset.reset_window
      && early_reset.reset_time_margin > 0.0);

  auto slow_reset = registered_input(1, event_energy, 0.5);
  check("slow reset fails the half-cycle synchronization gate",
      execute_signal_acknowledged_two_stroke_reset(slow_reset).status
          == SignalAcknowledgedResetStatus::ResetWindowViolation);

  auto insufficient = registered_input(1, event_energy);
  insufficient.reference_action = event_energy;
  check("strict compressed clock reserve fails closed at equality",
      execute_signal_acknowledged_two_stroke_reset(insufficient).status
          == SignalAcknowledgedResetStatus::InsufficientReferenceReserve);

  auto wrong_coordinate = registered_input(-1, event_energy);
  wrong_coordinate.latch_coordinate = 0.0;
  check("latch coordinate inconsistent with record fails closed",
      execute_signal_acknowledged_two_stroke_reset(wrong_coordinate).status
          == SignalAcknowledgedResetStatus::InvalidLatchCoordinate);

  auto sign_mismatch = registered_input(-1, event_energy);
  sign_mismatch.matter = signed_event_mode(1, event_energy);
  check("matter orientation inconsistent with latch fails closed",
      execute_signal_acknowledged_two_stroke_reset(sign_mismatch).status
          == SignalAcknowledgedResetStatus::InvalidMatterOrientation);

  auto nonzero_signal = registered_input(1, event_energy);
  nonzero_signal.signal = {0.1, 0.0};
  check("active cycle rejects nonempty local signal",
      execute_signal_acknowledged_two_stroke_reset(nonzero_signal).status
          == SignalAcknowledgedResetStatus::InvalidActiveSignal);

  auto occupied_output = registered_input(1, event_energy);
  occupied_output.output_port = {0.1, 0.0};
  check("occupied output backpressure fails closed",
      execute_signal_acknowledged_two_stroke_reset(occupied_output).status
          == SignalAcknowledgedResetStatus::NonemptyOutputPort);

  auto invalid_latch = registered_input(0, 0.0);
  invalid_latch.latch = 2;
  check("nonternary latch fails closed",
      execute_signal_acknowledged_two_stroke_reset(invalid_latch).status
          == SignalAcknowledgedResetStatus::InvalidLatch);

  auto invalid_frequency = registered_input(0, 0.0);
  invalid_frequency.clock_frequency = 0.0;
  check("nonpositive clock frequency fails closed",
      execute_signal_acknowledged_two_stroke_reset(invalid_frequency).status
          == SignalAcknowledgedResetStatus::InvalidClockFrequency);

  auto invalid_tolerance = registered_input(0, 0.0);
  invalid_tolerance.tolerance = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite tolerance fails closed",
      execute_signal_acknowledged_two_stroke_reset(invalid_tolerance).status
          == SignalAcknowledgedResetStatus::InvalidTolerance);

  std::cout << "FTD-0869 signal-acknowledged two-stroke reset EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "acknowledgement=COMPLETED_LOCAL_SIGNAL_NO_EXTRA_BIT\n";
  std::cout << "reset=SELECTED_NONSMOOTH_FINITE_TIME_WITH_SCALAR_BATH_LEDGER\n";
  std::cout << "local_recursion=READY_AFTER_EMPTY_OUTPUT_HANDOFF\n";
  std::cout << "smooth_reset_microscopic_bath_cubic_production_gstar=OPEN\n";
  return failures == 0 ? 0 : 1;
}
