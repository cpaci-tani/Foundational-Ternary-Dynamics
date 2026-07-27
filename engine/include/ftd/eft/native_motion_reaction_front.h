#pragma once
/**
 * @file native_motion_reaction_front.h
 * @brief Observer-only transport/reaction/source-memory discriminator
 *        (FTD-0585).
 */

namespace ftd::eft {

struct NativeMotionReactionFrontResult {
  bool valid = false;
  int reaction_free_rest_arms = 0;
  int reaction_free_rest_ticks = 0;
  int ballistic_control_arms = 0;
  int minimum_ballistic_hops = 0;
  int transport_fixtures = 0;
  int reaction_front_fixtures = 0;
  int moment_identity_samples = 0;
  int stale_kinematics_arms = 0;
  int maximum_evaporation_ticks = 0;
  double maximum_rest_velocity = 0.0;
  double maximum_rest_remainder = 0.0;
  double maximum_rest_displacement = 0.0;
  double maximum_continuity_residual = 0.0;
  double maximum_charge_balance_residual = 0.0;
  double maximum_first_moment_residual = 0.0;
  double maximum_snapshot_difference = 0.0;
  double maximum_stale_velocity_residual = 0.0;
  double maximum_stale_remainder_residual = 0.0;
  bool reaction_free_zero_kinematics_invariant = false;
  bool same_snapshot_admits_transport_or_reaction_decomposition = false;
  bool globally_balanced_reaction_source_is_local_current = false;
  bool support_translation_implies_particle_worldline = false;
  bool evaporation_preserves_hidden_kinematics = false;
  bool genesis_reuses_hidden_kinematics = false;
  bool selected_force_is_common_action = false;
  bool production_changed = false;
};

NativeMotionReactionFrontResult analyze_native_motion_reaction_front();

}  // namespace ftd::eft
