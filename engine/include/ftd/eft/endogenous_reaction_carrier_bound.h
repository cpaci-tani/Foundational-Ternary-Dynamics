#pragma once
/**
 * @file endogenous_reaction_carrier_bound.h
 * @brief Observer-only endogenous genesis/autocatalysis bound (FTD-0586).
 */

#include <array>

namespace ftd::eft {

struct EndogenousReactionCarrierVolume {
  int lattice_size = 0;
  double maximum_mode_eigenvalue = 0.0;
  double single_source_step_bound = 0.0;
  double single_source_pulse_bound = 0.0;
  double three_source_pulse_bound = 0.0;
  double threshold_margin = 0.0;
};

struct EndogenousReactionCarrierBoundResult {
  bool valid = false;
  std::array<EndogenousReactionCarrierVolume, 4> volumes{};
  int spectral_volume_count = 0;
  int endogenous_arms = 0;
  int endogenous_ticks = 0;
  int constant_source_arms = 0;
  int pulse_source_arms = 0;
  int external_control_arms = 0;
  int external_control_genesis_events = 0;
  int endogenous_genesis_events = 0;
  int endogenous_evaporation_events = 0;
  int minimum_sources_not_excluded = 0;
  int maximum_initial_sources_closed = 0;
  double maximum_single_source_pulse_bound = 0.0;
  double maximum_three_source_pulse_bound = 0.0;
  double minimum_threshold_margin = 0.0;
  double maximum_observed_flux = 0.0;
  double maximum_bound_excess = 0.0;
  double maximum_velocity = 0.0;
  double maximum_remainder = 0.0;
  bool modal_step_bound_derived = false;
  bool rectangular_pulse_bound_derived = false;
  bool no_first_genesis_for_three_sources = false;
  bool manifested_support_remained_subset = false;
  bool pulse_removal_exercised = false;
  bool void_kinematics_sanitized = false;
  bool observer_neutral = false;
  bool external_genesis_control_live = false;
  bool four_sources_sufficient = false;
  bool self_sustaining_reaction_carrier_established = false;
  bool production_changed = false;
};

EndogenousReactionCarrierBoundResult
analyze_endogenous_reaction_carrier_bound();

}  // namespace ftd::eft
