/** FTD-0862 isolated phase-referenced action export rail verifier. */

#include "ftd/eft/phase_referenced_action_rail.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

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

double action(const CanonicalCarrierPair& pair) {
  return 0.5 * (pair.q * pair.q + pair.p * pair.p);
}

bool same_pair(
    const CanonicalCarrierPair& first,
    const CanonicalCarrierPair& second) {
  return close(first.q, second.q) && close(first.p, second.p);
}

PhaseCalendar make_calendar() {
  PhaseCalendar calendar;
  calendar.baseline_action = 2.5;
  calendar.phase_origin = 0.37;
  calendar.spatial_twist = 0.41;
  calendar.temporal_advance = 0.41;
  return calendar;
}

std::vector<CanonicalCarrierPair> baseline_rail(
    const PhaseCalendar& calendar, std::size_t length, std::uint64_t tick) {
  std::vector<CanonicalCarrierPair> rail;
  rail.reserve(length);
  for (std::size_t depth = 0; depth < length; ++depth) {
    rail.push_back(phase_calendar_baseline(
        calendar, static_cast<std::int64_t>(depth), tick));
  }
  return rail;
}

}  // namespace

int main() {
  const auto calendar = make_calendar();
  check("coherent calendar is compliant", phase_calendar_compliant(calendar));
  check("coherent calendar mismatch is zero",
      close(phase_calendar_mismatch(calendar), 0.0));

  auto incoherent = calendar;
  incoherent.spatial_twist += 0.125;
  check("incoherent calendar fails closed",
      !phase_calendar_compliant(incoherent));

  const auto baseline = phase_calendar_baseline(calendar, 3, 7);
  check("baseline has declared action", close(action(baseline), 2.5));
  check("incoming phase equals next-tick port phase",
      same_pair(
          phase_calendar_baseline(calendar, -1, 7),
          phase_calendar_baseline(calendar, 0, 8)));
  check("baseline follows the one-cell characteristic",
      same_pair(
          phase_calendar_baseline(calendar, 2, 7),
          phase_calendar_baseline(calendar, 3, 8)));

  PhaseReferencedRailStepInput positive_input;
  positive_input.calendar = calendar;
  positive_input.tick = 0;
  positive_input.event_sign = 1;
  positive_input.event_energy = 1.75;
  positive_input.retained = baseline_rail(calendar, 4, 0);
  const auto positive = step_phase_referenced_action_rail(positive_input);
  check("positive event rail step is valid", positive.valid());
  check("positive event closes excess-action ledger",
      close(positive.ledger_residual, 0.0));
  check("positive event raises retained excess by event energy",
      close(positive.excess_action_after, 1.75));
  const auto positive_readout = read_phase_referenced_carrier(
      positive.retained[0], phase_calendar_baseline(calendar, 0, 1));
  check("positive readout recovers sign", positive_readout.valid()
      && positive_readout.event_sign == 1);
  check("positive readout recovers energy",
      close(positive_readout.event_energy, 1.75));
  check("loaded pair is quarter-turn orthogonal",
      close(positive_readout.dot_with_baseline, 0.0));

  auto negative_input = positive_input;
  negative_input.event_sign = -1;
  const auto negative = step_phase_referenced_action_rail(negative_input);
  const auto negative_readout = read_phase_referenced_carrier(
      negative.retained[0], phase_calendar_baseline(calendar, 0, 1));
  check("negative readout recovers sign", negative_readout.valid()
      && negative_readout.event_sign == -1);
  check("opposite events have opposite oriented area",
      positive_readout.oriented_area > 0.0
      && negative_readout.oriented_area < 0.0);

  PhaseReferencedRailStepInput no_event_input;
  no_event_input.calendar = calendar;
  no_event_input.tick = 0;
  no_event_input.event_sign = 0;
  no_event_input.event_energy = 0.0;
  no_event_input.retained = baseline_rail(calendar, 4, 0);
  const auto no_event = step_phase_referenced_action_rail(no_event_input);
  const auto no_event_readout = read_phase_referenced_carrier(
      no_event.retained[0], phase_calendar_baseline(calendar, 0, 1));
  check("no-event step injects the next prepared baseline",
      no_event.valid() && no_event_readout.valid()
      && no_event_readout.event_sign == 0
      && close(no_event_readout.event_energy, 0.0));

  std::vector<CanonicalCarrierPair> state = baseline_rail(calendar, 3, 0);
  const std::int8_t signs[] = {1, -1, 1, -1};
  const double energies[] = {0.5, 1.0, 1.5, 2.0};
  CanonicalCarrierPair first_export;
  for (std::uint64_t tick = 0; tick < 4; ++tick) {
    PhaseReferencedRailStepInput input;
    input.calendar = calendar;
    input.tick = tick;
    input.event_sign = signs[tick];
    input.event_energy = energies[tick];
    input.retained = state;
    const auto result = step_phase_referenced_action_rail(input);
    check("multi-event step remains valid", result.valid());
    check("multi-event step ledger remains closed",
        close(result.ledger_residual, 0.0));
    if (tick == 3) first_export = result.exported_tail;
    state = result.retained;
  }
  const auto newest = read_phase_referenced_carrier(
      state[0], phase_calendar_baseline(calendar, 0, 4));
  const auto middle = read_phase_referenced_carrier(
      state[1], phase_calendar_baseline(calendar, 1, 4));
  const auto oldest_retained = read_phase_referenced_carrier(
      state[2], phase_calendar_baseline(calendar, 2, 4));
  check("rail preserves event age order",
      newest.event_sign == -1 && close(newest.event_energy, 2.0)
      && middle.event_sign == 1 && close(middle.event_energy, 1.5)
      && oldest_retained.event_sign == -1
      && close(oldest_retained.event_energy, 1.0));
  const auto exported = read_phase_referenced_carrier(
      first_export, phase_calendar_baseline(calendar, 3, 4));
  check("complete tail pair retains exported event",
      exported.valid() && exported.event_sign == 1
      && close(exported.event_energy, 0.5));
  check("finite length-three retained excess is bounded by three Bmax",
      newest.event_energy + middle.event_energy + oldest_retained.event_energy
          <= 3.0 * 2.0 + 1e-11);

  const auto recovered = recover_prior_retained_rail(
      positive.retained, positive.exported_tail);
  check("tail-completed inverse recovers prior rail size",
      recovered.size() == positive_input.retained.size());
  check("tail-completed inverse recovers every prior pair",
      recovered.size() == positive_input.retained.size()
      && std::equal(
          recovered.begin(), recovered.end(), positive_input.retained.begin(),
          [](const auto& first, const auto& second) {
            return same_pair(first, second);
          }));

  auto empty_input = positive_input;
  empty_input.retained.clear();
  check("empty rail fails closed",
      step_phase_referenced_action_rail(empty_input).status
          == PhaseReferencedRailStatus::EmptyRail);
  auto bad_event = positive_input;
  bad_event.event_sign = 0;
  bad_event.event_energy = 1.0;
  check("inconsistent event fails closed",
      step_phase_referenced_action_rail(bad_event).status
          == PhaseReferencedRailStatus::InvalidEvent);
  auto bad_calendar_input = positive_input;
  bad_calendar_input.calendar = incoherent;
  check("incoherent step fails closed",
      step_phase_referenced_action_rail(bad_calendar_input).status
          == PhaseReferencedRailStatus::IncoherentCalendar);
  CanonicalCarrierPair nonfinite;
  nonfinite.q = std::numeric_limits<double>::infinity();
  nonfinite.p = 0.0;
  check("nonfinite readout fails closed",
      read_phase_referenced_carrier(nonfinite, baseline).status
          == PhaseReferencedRailStatus::InvalidCarrier);

  std::cout << "FTD-0862 phase-referenced action rail EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "scope=PREPARED_BASELINE_SELECTED_OUTWARD_RAIL\n";
  std::cout << "signed_event_readout=EXACT_ON_REGISTERED_SUBSPACE\n";
  std::cout << "production_c18_equivalence=REJECTED\n";
  std::cout << "baseline_clock_controller=OPEN\n";
  std::cout << "production_integration=NONE\n";
  return failures == 0 ? 0 : 1;
}
