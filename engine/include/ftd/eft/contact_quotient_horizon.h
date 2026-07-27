#pragma once
/**
 * @file contact_quotient_horizon.h
 * @brief Actual-production quotient horizon for identical contact (FTD-0526).
 */

#include "ftd/eft/native_contact_active_set.h"

namespace ftd::eft {

struct ContactQuotientHorizonResult {
  bool valid = false;
  bool symmetric_movement_order = false;
  bool commensurate_horizon = false;
  bool quotient_equivalent_before_horizon = false;
  bool rejoined_at_commensurate_horizon = false;
  bool overshoot_breaks_quotient_at_horizon = false;
  int predicted_horizon_tick = 0;
  int first_physical_divergence_tick = 0;
  int pre_horizon_ticks_compared = 0;
  int maximum_journal_events = 0;
  double overshoot = 0.0;
  double expected_horizon_phase_residual = 0.0;
  double worst_pre_horizon_phase_residual = 0.0;
  double worst_pre_horizon_density_residual = 0.0;
  double worst_pre_horizon_current_residual = 0.0;
  double minimum_raw_label_residual = 0.0;
  double horizon_phase_residual = 0.0;
  double horizon_density_residual = 0.0;
  double horizon_site_state_residual = 0.0;
  double horizon_invariant_residual = 0.0;
  double crossing_reset_residual = 0.0;
  double bounce_overshoot_residual = 0.0;
  double commensurate_extra_tick_residual = 0.0;
  double field_residual = 0.0;
  NativeContactActiveSetGeometry geometry{};
};

/** Compare actual crossing and momentum-exchanged raw representatives.
 *
 * Both branches begin at the same contact positions. They are identical as an
 * unlabeled physical phase-space multiset, but attach the two opposite
 * velocities to opposite raw charts. The observer advances both through the
 * frozen production movement phase and measures where that quotient ceases to
 * be respected.
 */
ContactQuotientHorizonResult analyze_contact_quotient_horizon(
    int L,
    const Vec3& contact_position,
    Coord chart_direction,
    int polarity,
    double speed,
    bool symmetric_movement_order,
    unsigned int movement_seed = 13,
    double tolerance = 1e-12);

}  // namespace ftd::eft
