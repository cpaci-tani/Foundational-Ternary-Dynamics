#pragma once
/**
 * @file native_active_mode_backreaction.h
 * @brief Observer-only frozen field-to-matter backreaction audit (FTD-0582).
 */

namespace ftd::eft {

struct NativeActiveModeBackreactionResult {
  bool valid = false;
  int active_mode_arms = 0;
  int active_mode_ticks = 0;
  int active_field_changed_arms = 0;
  int ballistic_arms = 0;
  int selected_force_control_arms = 0;
  int coupling_control_pairs = 0;
  int coupling_control_differences = 0;
  double maximum_initial_energy_relative_residual = 0.0;
  double minimum_initial_energy_to_barrier_ratio = 0.0;
  double maximum_native_velocity_response = 0.0;
  double maximum_native_remainder_response = 0.0;
  double maximum_native_anchor_displacement = 0.0;
  int maximum_native_movement_events = 0;
  int minimum_ballistic_movement_events = 0;
  double maximum_ballistic_speed_residual = 0.0;
  int maximum_ballistic_reaction_events = 0;
  double minimum_selected_force_response = 0.0;
  double maximum_selected_force_mirror_residual = 0.0;
  bool source_graph_one_way = false;
  bool active_native_backreaction_absent = false;
  bool sensitivity_controls_pass = false;
  bool native_common_action_implemented = false;
  bool production_changed = false;
};

NativeActiveModeBackreactionResult analyze_native_active_mode_backreaction();

}  // namespace ftd::eft

