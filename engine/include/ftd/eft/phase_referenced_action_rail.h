#pragma once

/**
 * @file phase_referenced_action_rail.h
 * @brief FTD-0862 isolated phase-referenced action export rail witness.
 *
 * A prepared nonzero canonical baseline supplies the phase reference that the
 * FTD-0860 action pump lacks on an arbitrary background.  When spatial twist
 * and temporal phase advance agree modulo 2*pi, a one-cell outward shift
 * transports both the loaded pair and its reference phase on the same
 * characteristic.  Relative action recovers event energy and the oriented
 * area against the baseline recovers event sign.
 *
 * This is a selected open reference rail under ftd::eft.  It is not production
 * C18 propagation and supplies no physical baseline source, clock controller,
 * G* cadence, Born weight, vacuum claim, cubic embedding, or Voxel consumer.
 */

#include <cstdint>
#include <vector>

namespace ftd::eft {

struct CanonicalCarrierPair {
  double q = 0.0;
  double p = 0.0;
};

struct PhaseCalendar {
  double baseline_action = 0.0;
  double phase_origin = 0.0;
  double spatial_twist = 0.0;
  double temporal_advance = 0.0;
};

enum class PhaseReferencedRailStatus : std::uint8_t {
  Valid = 0,
  EmptyRail,
  InvalidCalendar,
  IncoherentCalendar,
  InvalidEvent,
  InvalidCarrier,
  NonFiniteOutput,
  ReadoutMismatch,
};

struct PhaseReferencedReadout {
  PhaseReferencedRailStatus status =
      PhaseReferencedRailStatus::InvalidCarrier;
  std::int8_t event_sign = 0;
  double event_energy = 0.0;
  double carrier_action = 0.0;
  double baseline_action = 0.0;
  double dot_with_baseline = 0.0;
  double oriented_area = 0.0;

  bool valid() const { return status == PhaseReferencedRailStatus::Valid; }
};

struct PhaseReferencedRailStepInput {
  PhaseCalendar calendar;
  std::uint64_t tick = 0;
  std::int8_t event_sign = 0;
  double event_energy = 0.0;
  std::vector<CanonicalCarrierPair> retained;
};

struct PhaseReferencedRailStepResult {
  PhaseReferencedRailStatus status =
      PhaseReferencedRailStatus::InvalidCalendar;
  std::vector<CanonicalCarrierPair> retained;
  CanonicalCarrierPair injected;
  CanonicalCarrierPair exported_tail;
  double excess_action_before = 0.0;
  double excess_action_after = 0.0;
  double exported_tail_excess = 0.0;
  double ledger_residual = 0.0;

  bool valid() const { return status == PhaseReferencedRailStatus::Valid; }
};

/** Principal phase mismatch in [-pi,pi] for one outward cell per tick. */
double phase_calendar_mismatch(const PhaseCalendar& calendar);

/** True only for a finite positive baseline and mismatch within tolerance. */
bool phase_calendar_compliant(
    const PhaseCalendar& calendar, double tolerance = 1e-12);

/** Prepared baseline beta_j^n at signed causal depth and global tick. */
CanonicalCarrierPair phase_calendar_baseline(
    const PhaseCalendar& calendar,
    std::int64_t depth,
    std::uint64_t tick);

/**
 * Recover event energy and orientation relative to the local prepared phase.
 * A baseline pair reads as the no-event result (sign zero, energy zero).
 */
PhaseReferencedReadout read_phase_referenced_carrier(
    const CanonicalCarrierPair& carrier,
    const CanonicalCarrierPair& baseline,
    double tolerance = 1e-12);

/** Load one event/no-event input, shift retained pairs, and export the tail. */
PhaseReferencedRailStepResult step_phase_referenced_action_rail(
    const PhaseReferencedRailStepInput& input,
    double tolerance = 1e-12);

/** Recover the prior retained rail from the new rail and complete tail pair. */
std::vector<CanonicalCarrierPair> recover_prior_retained_rail(
    const std::vector<CanonicalCarrierPair>& retained_after,
    const CanonicalCarrierPair& exported_tail);

}  // namespace ftd::eft

