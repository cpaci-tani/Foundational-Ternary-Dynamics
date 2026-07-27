#pragma once
/**
 * @file overshoot_preserving_contact_rebase.h
 * @brief Quotient-correct paired contact rebase and raw inverse audit
 *        (FTD-0527).
 */

#include "ftd/eft/contact_quotient_horizon.h"

#include <array>

namespace ftd::eft {

struct ContactCarrierRecord {
  Coord anchor{};
  Vec3 remainder{};
  Vec3 velocity{};
  int polarity = 0;
  int bookkeeping_identity = 0;
};

struct ContactPairRecord {
  std::array<ContactCarrierRecord, 2> carrier{};
};

struct OvershootPreservingContactRebaseResult {
  bool valid = false;
  bool physical_repair_constructive = false;
  bool raw_inverse_exists_without_record = false;
  bool one_bit_lift_constructive = false;
  int preimage_multiplicity = 0;
  int minimum_history_bits = 0;
  int horizon_tick = 0;
  double overshoot = 0.0;
  double raw_preimage_residual = 0.0;
  double quotient_phase_residual = 0.0;
  double density_residual = 0.0;
  double current_residual = 0.0;
  double continuity_residual = 0.0;
  double common_output_residual = 0.0;
  double identity_output_residual = 0.0;
  double overshoot_residual = 0.0;
  double invariant_residual = 0.0;
  double causal_residual = 0.0;
  double physical_reversal_residual = 0.0;
  double history_recovery_residual = 0.0;
  NativeContactActiveSetGeometry geometry{};
  ContactPairRecord crossing_preimage{};
  ContactPairRecord bounce_preimage{};
  ContactPairRecord crossing_rebased_output{};
  ContactPairRecord bounce_free_output{};
};

/// Construct the exact paired rebase that preserves the FTD-0526 overshoot,
/// then audit its two-to-one raw projection and one-bit event lift.
OvershootPreservingContactRebaseResult
analyze_overshoot_preserving_contact_rebase(
    int L,
    const Vec3& contact_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft
