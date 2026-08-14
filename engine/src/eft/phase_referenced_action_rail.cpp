#include "ftd/eft/phase_referenced_action_rail.h"

#include "ftd/eft/relative_action_transducer.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

bool finite_pair(const CanonicalCarrierPair& pair) {
  return std::isfinite(pair.q) && std::isfinite(pair.p);
}

double pair_action(const CanonicalCarrierPair& pair) {
  const double radius = std::hypot(pair.q, pair.p);
  return 0.5 * radius * radius;
}

bool close(double first, double second, double tolerance) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

double total_excess(
    const std::vector<CanonicalCarrierPair>& rail,
    double baseline_action) {
  double total = 0.0;
  for (const auto& pair : rail) {
    total += pair_action(pair) - baseline_action;
  }
  return total;
}

}  // namespace

double phase_calendar_mismatch(const PhaseCalendar& calendar) {
  if (!std::isfinite(calendar.spatial_twist)
      || !std::isfinite(calendar.temporal_advance)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  const double two_pi = 2.0 * std::acos(-1.0);
  return std::remainder(
      calendar.spatial_twist - calendar.temporal_advance, two_pi);
}

bool phase_calendar_compliant(
    const PhaseCalendar& calendar, double tolerance) {
  if (!std::isfinite(calendar.baseline_action)
      || !(calendar.baseline_action > 0.0)
      || !std::isfinite(calendar.phase_origin)
      || !std::isfinite(tolerance)
      || tolerance < 0.0) {
    return false;
  }
  const double mismatch = phase_calendar_mismatch(calendar);
  return std::isfinite(mismatch) && std::abs(mismatch) <= tolerance;
}

CanonicalCarrierPair phase_calendar_baseline(
    const PhaseCalendar& calendar,
    std::int64_t depth,
    std::uint64_t tick) {
  CanonicalCarrierPair baseline;
  if (!std::isfinite(calendar.baseline_action)
      || !(calendar.baseline_action > 0.0)
      || !std::isfinite(calendar.phase_origin)
      || !std::isfinite(calendar.spatial_twist)
      || !std::isfinite(calendar.temporal_advance)) {
    baseline.q = std::numeric_limits<double>::quiet_NaN();
    baseline.p = std::numeric_limits<double>::quiet_NaN();
    return baseline;
  }
  const double radius = std::sqrt(2.0 * calendar.baseline_action);
  const double phase = calendar.phase_origin
      + calendar.spatial_twist * static_cast<double>(depth)
      - calendar.temporal_advance * static_cast<double>(tick);
  baseline.q = radius * std::cos(phase);
  baseline.p = radius * std::sin(phase);
  return baseline;
}

PhaseReferencedReadout read_phase_referenced_carrier(
    const CanonicalCarrierPair& carrier,
    const CanonicalCarrierPair& baseline,
    double tolerance) {
  PhaseReferencedReadout result;
  if (!finite_pair(carrier) || !finite_pair(baseline)
      || !std::isfinite(tolerance) || tolerance < 0.0) {
    result.status = PhaseReferencedRailStatus::InvalidCarrier;
    return result;
  }
  result.carrier_action = pair_action(carrier);
  result.baseline_action = pair_action(baseline);
  if (!std::isfinite(result.carrier_action)
      || !std::isfinite(result.baseline_action)
      || !(result.baseline_action > 0.0)) {
    result.status = PhaseReferencedRailStatus::InvalidCarrier;
    return result;
  }
  result.event_energy = result.carrier_action - result.baseline_action;
  result.dot_with_baseline = carrier.q * baseline.q + carrier.p * baseline.p;
  result.oriented_area = baseline.q * carrier.p - baseline.p * carrier.q;

  if (close(result.event_energy, 0.0, tolerance)
      && close(result.dot_with_baseline, 2.0 * result.baseline_action, tolerance)
      && close(result.oriented_area, 0.0, tolerance)) {
    result.event_energy = 0.0;
    result.event_sign = 0;
    result.status = PhaseReferencedRailStatus::Valid;
    return result;
  }
  if (!(result.event_energy > 0.0)
      || !close(result.dot_with_baseline, 0.0, tolerance)
      || close(result.oriented_area, 0.0, tolerance)) {
    result.status = PhaseReferencedRailStatus::ReadoutMismatch;
    return result;
  }
  result.event_sign = result.oriented_area > 0.0 ? 1 : -1;
  result.status = PhaseReferencedRailStatus::Valid;
  return result;
}

PhaseReferencedRailStepResult step_phase_referenced_action_rail(
    const PhaseReferencedRailStepInput& input,
    double tolerance) {
  PhaseReferencedRailStepResult result;
  if (input.retained.empty()) {
    result.status = PhaseReferencedRailStatus::EmptyRail;
    return result;
  }
  if (!std::isfinite(input.calendar.baseline_action)
      || !(input.calendar.baseline_action > 0.0)) {
    result.status = PhaseReferencedRailStatus::InvalidCalendar;
    return result;
  }
  if (!phase_calendar_compliant(input.calendar, tolerance)) {
    result.status = PhaseReferencedRailStatus::IncoherentCalendar;
    return result;
  }
  for (const auto& pair : input.retained) {
    if (!finite_pair(pair) || !std::isfinite(pair_action(pair))) {
      result.status = PhaseReferencedRailStatus::InvalidCarrier;
      return result;
    }
  }

  const bool no_event = input.event_sign == 0 && input.event_energy == 0.0;
  const bool valid_event = (input.event_sign == -1 || input.event_sign == 1)
      && std::isfinite(input.event_energy) && input.event_energy > 0.0;
  if (!no_event && !valid_event) {
    result.status = PhaseReferencedRailStatus::InvalidEvent;
    return result;
  }

  const auto incoming = phase_calendar_baseline(
      input.calendar, -1, input.tick);
  if (!finite_pair(incoming)) {
    result.status = PhaseReferencedRailStatus::InvalidCalendar;
    return result;
  }
  result.injected = incoming;
  if (valid_event) {
    RelativeActionPumpInput pump_input;
    pump_input.event_sign = input.event_sign;
    pump_input.event_energy = input.event_energy;
    pump_input.canonical_q = incoming.q;
    pump_input.canonical_p = incoming.p;
    const auto pump = pump_relative_action(pump_input);
    if (!pump.valid()) {
      result.status = PhaseReferencedRailStatus::InvalidEvent;
      return result;
    }
    result.injected.q = pump.canonical_q_after;
    result.injected.p = pump.canonical_p_after;
  }

  result.retained.resize(input.retained.size());
  result.retained[0] = result.injected;
  for (std::size_t index = 1; index < input.retained.size(); ++index) {
    result.retained[index] = input.retained[index - 1];
  }
  result.exported_tail = input.retained.back();
  if (!finite_pair(result.injected)) {
    result.status = PhaseReferencedRailStatus::NonFiniteOutput;
    return result;
  }

  result.excess_action_before =
      total_excess(input.retained, input.calendar.baseline_action);
  result.excess_action_after =
      total_excess(result.retained, input.calendar.baseline_action);
  result.exported_tail_excess =
      pair_action(result.exported_tail) - input.calendar.baseline_action;
  const double injected_excess = valid_event ? input.event_energy : 0.0;
  result.ledger_residual = result.excess_action_after
      - result.excess_action_before
      - injected_excess
      + result.exported_tail_excess;
  if (!std::isfinite(result.excess_action_before)
      || !std::isfinite(result.excess_action_after)
      || !std::isfinite(result.exported_tail_excess)
      || !std::isfinite(result.ledger_residual)) {
    result.status = PhaseReferencedRailStatus::NonFiniteOutput;
    return result;
  }
  result.status = PhaseReferencedRailStatus::Valid;
  return result;
}

std::vector<CanonicalCarrierPair> recover_prior_retained_rail(
    const std::vector<CanonicalCarrierPair>& retained_after,
    const CanonicalCarrierPair& exported_tail) {
  if (retained_after.empty()) return {};
  std::vector<CanonicalCarrierPair> recovered(retained_after.size());
  for (std::size_t index = 0; index + 1 < retained_after.size(); ++index) {
    recovered[index] = retained_after[index + 1];
  }
  recovered.back() = exported_tail;
  return recovered;
}

}  // namespace ftd::eft

