#pragma once
/**
 * @file production_same_sign_bounce.h
 * @brief Read-only production same-sign collision reciprocity audit
 *        (FTD-0506).
 */

#include "ftd/eft/boundary_collision_resolution.h"
#include "ftd/eft/production_hop_kinematics.h"

namespace ftd::eft {

struct ProductionSameSignBounceResult {
  bool valid = false;
  int L = 0;
  Coord source_anchor{};
  Coord target_anchor{};
  Coord hop{};
  int charge = 0;
  double dt = 0.0;
  Vec3 effective_position_before{};
  Vec3 effective_position_after{};
  Vec3 specular_position_after{};
  Vec3 specular_remainder_after{};
  double source_velocity_reflection_residual = 0.0;
  double source_remainder_reset_residual = 0.0;
  double target_unchanged_residual = 0.0;
  double manifestation_residual = 0.0;
  double specular_remainder_residual = 0.0;
  double production_effective_causal_residual = 0.0;
  double specular_arc_causal_residual = 0.0;
  double pair_energy_residual = 0.0;
  double pair_momentum_defect = 0.0;
  double field_state_change_residual = 0.0;
  double exact_current_difference = 0.0;
  double exact_current_continuity_residual = 0.0;
  double missing_journal_current_residual = 0.0;
  double inverse_phase_space_residual = 0.0;
  int journal_event_count = 0;
  PiecewiseCurrentSignature production_endpoint_current{};
  PiecewiseCurrentSignature specular_bounce_current{};
};

/// Compare one measured production same-sign bounce and its next unchanged
/// tick against a specular occupied-target reflection.
ProductionSameSignBounceResult analyze_production_same_sign_bounce(
    int L,
    Coord source_anchor,
    Coord hop,
    const Voxel& source_before,
    const Voxel& target_before,
    const Voxel& source_after,
    const Voxel& target_after,
    const Voxel& source_after_second_tick,
    const Voxel& target_after_second_tick,
    int journal_event_count,
    double dt,
    double tolerance = 1e-12);

}  // namespace ftd::eft
