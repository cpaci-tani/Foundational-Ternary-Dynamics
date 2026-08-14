/** FTD-0863 isolated catalytic phase-reference transducer verifier. */

#include "ftd/eft/catalytic_phase_reference.h"

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

bool close(double first, double second, double tolerance = 1e-11) {
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

}  // namespace

int main() {
  const CanonicalCarrierPair reference{1.25, -0.75};
  const double reference_action = action(reference);
  const double omega = 0.43;

  const auto forward = rotate_catalytic_phase_reference(reference, omega);
  check("reference rotation is valid", forward.valid());
  check("reference rotation preserves action",
      close(forward.action_before, reference_action)
      && close(forward.action_after, reference_action)
      && close(forward.action_residual, 0.0));
  check("reference rotation has unit Jacobian",
      close(forward.jacobian_determinant, 1.0));
  const auto inverse = rotate_catalytic_phase_reference(forward.after, -omega);
  check("opposite rotation recovers reference",
      inverse.valid() && same_pair(inverse.after, reference));

  const CanonicalCarrierPair reversed_reference{reference.q, -reference.p};
  const auto reversed_step =
      rotate_catalytic_phase_reference(reversed_reference, -omega);
  check("time reversal exchanges rotation senses",
      reversed_step.valid()
      && close(reversed_step.after.q, forward.after.q)
      && close(reversed_step.after.p, -forward.after.p));

  const double event_energy = 1.75;
  const double event_amplitude = std::sqrt(2.0 * event_energy);
  CatalyticPhaseExchangeInput emission_input;
  emission_input.reference = reference;
  emission_input.matter_amplitude = event_amplitude;
  emission_input.signal = {0.0, 0.0};
  emission_input.eligibility = RecordPortEligibility::Exchange;
  const auto emission = exchange_catalytic_phase_signal(emission_input);
  check("positive emission is valid", emission.valid());
  check("open gate empties matter amplitude", close(emission.matter_after, 0.0));
  check("zero-baseline signal receives exact event energy",
      close(action(emission.signal_after), event_energy));
  check("reference pair is catalytic and unchanged",
      same_pair(emission.reference_after, reference)
      && close(emission.reference_action_after, reference_action));
  check("positive emission has positive oriented area",
      emission.oriented_area_after > 0.0);
  check("positive emission closes total energy",
      close(emission.energy_residual, 0.0));
  check("positive emission closes signed interface content",
      close(emission.signed_content_residual, 0.0));

  auto negative_input = emission_input;
  negative_input.matter_amplitude = -event_amplitude;
  const auto negative = exchange_catalytic_phase_signal(negative_input);
  check("negative emission is valid", negative.valid());
  check("negative emission has negative oriented area",
      negative.oriented_area_after < 0.0);
  check("opposite event signs give opposite signal pairs",
      close(negative.signal_after.q, -emission.signal_after.q)
      && close(negative.signal_after.p, -emission.signal_after.p));

  const auto emission_readout = read_catalytic_phase_signal(
      reference, emission.signal_after);
  check("signal readout is valid", emission_readout.valid());
  check("signal readout recovers signed orthogonal amplitude",
      close(emission_readout.orthogonal_amplitude, event_amplitude));
  check("emitted signal has no parallel contamination",
      close(emission_readout.parallel_amplitude, 0.0));
  check("signal readout energy is exact",
      close(emission_readout.signal_energy, event_energy));

  CatalyticPhaseExchangeInput absorption_input;
  absorption_input.reference = reference;
  absorption_input.matter_amplitude = 0.0;
  absorption_input.signal = emission.signal_after;
  absorption_input.eligibility = RecordPortEligibility::Exchange;
  const auto absorption = exchange_catalytic_phase_signal(absorption_input);
  check("reciprocal absorption is valid", absorption.valid());
  check("reciprocal absorption restores matter amplitude",
      close(absorption.matter_after, event_amplitude));
  check("reciprocal absorption clears signal",
      close(action(absorption.signal_after), 0.0));
  check("reciprocal absorption closes energy",
      close(absorption.energy_residual, 0.0));

  CatalyticPhaseExchangeInput hold_input;
  hold_input.reference = reference;
  hold_input.matter_amplitude = -0.8;
  hold_input.signal = {0.4, 1.1};
  hold_input.eligibility = RecordPortEligibility::Hold;
  const auto held = exchange_catalytic_phase_signal(hold_input);
  check("hold gate is valid", held.valid());
  check("hold gate leaves matter unchanged",
      close(held.matter_after, hold_input.matter_amplitude));
  check("hold gate leaves full signal unchanged",
      same_pair(held.signal_after, hold_input.signal));

  const auto initial_readout = read_catalytic_phase_signal(
      reference, hold_input.signal);
  auto exchange_parallel_input = hold_input;
  exchange_parallel_input.eligibility = RecordPortEligibility::Exchange;
  const auto exchanged_parallel =
      exchange_catalytic_phase_signal(exchange_parallel_input);
  const auto exchanged_readout = read_catalytic_phase_signal(
      reference, exchanged_parallel.signal_after);
  check("parallel signal component is a spectator",
      initial_readout.valid() && exchanged_readout.valid()
      && close(
          initial_readout.parallel_amplitude,
          exchanged_readout.parallel_amplitude));

  PhaseCalendar calendar;
  calendar.baseline_action = reference_action;
  calendar.phase_origin = 0.31;
  calendar.spatial_twist = omega;
  calendar.temporal_advance = omega;
  check("local phase reference is coherent with outward rail",
      phase_calendar_compliant(calendar));
  check("coherent reference follows event characteristic",
      same_pair(
          phase_calendar_baseline(calendar, 2, 5),
          phase_calendar_baseline(calendar, 3, 6)));

  const double two_pi = 2.0 * std::acos(-1.0);
  PhaseCalendar ring_calendar = calendar;
  ring_calendar.spatial_twist = two_pi * 2.0 / 7.0;
  ring_calendar.temporal_advance = ring_calendar.spatial_twist;
  check("periodic winding closes after seven sites",
      same_pair(
          phase_calendar_baseline(ring_calendar, 0, 0),
          phase_calendar_baseline(ring_calendar, 7, 0)));

  CatalyticPhaseExchangeInput zero_reference = emission_input;
  zero_reference.reference = {0.0, 0.0};
  check("zero reference fails closed",
      exchange_catalytic_phase_signal(zero_reference).status
          == CatalyticPhaseReferenceStatus::InvalidReference);
  CatalyticPhaseExchangeInput invalid_signal = emission_input;
  invalid_signal.signal.q = std::numeric_limits<double>::infinity();
  check("nonfinite signal fails closed",
      exchange_catalytic_phase_signal(invalid_signal).status
          == CatalyticPhaseReferenceStatus::InvalidSignal);
  CatalyticPhaseExchangeInput invalid_matter = emission_input;
  invalid_matter.matter_amplitude =
      std::numeric_limits<double>::quiet_NaN();
  check("nonfinite matter amplitude fails closed",
      exchange_catalytic_phase_signal(invalid_matter).status
          == CatalyticPhaseReferenceStatus::InvalidMatterAmplitude);
  CatalyticPhaseExchangeInput invalid_gate = emission_input;
  invalid_gate.eligibility = static_cast<RecordPortEligibility>(7);
  check("invalid eligibility fails closed",
      exchange_catalytic_phase_signal(invalid_gate).status
          == CatalyticPhaseReferenceStatus::InvalidEligibility);

  std::cout << "FTD-0863 catalytic phase-reference EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "scope=SEPARATE_CONSERVED_REFERENCE_PLUS_ZERO_BASELINE_SIGNAL\n";
  std::cout << "matter_signal_exchange=RECIPROCAL_ENERGY_EXACT\n";
  std::cout << "reference_action=UNCHANGED\n";
  std::cout << "pilot_frequency_gstar_gearbox=OPEN\n";
  std::cout << "production_integration=NONE\n";
  return failures == 0 ? 0 : 1;
}

