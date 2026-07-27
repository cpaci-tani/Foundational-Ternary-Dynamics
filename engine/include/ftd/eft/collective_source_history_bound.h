#pragma once
/**
 * @file collective_source_history_bound.h
 * @brief Observer-only collective causal-source bounds (FTD-0588).
 */

#include <cstdint>
#include <vector>

namespace ftd::eft {

enum class CollectiveSourceHistoryKind : std::uint8_t {
  LockedStep = 0,
  SynchronousPulse = 1,
  NativeUnlocked = 2,
};

struct CollectiveSourceHistoryVolume {
  int lattice_size = 0;
  double maximum_mode_eigenvalue = 0.0;
  double one_source_step_triangle_bound = 0.0;
  double common_step_coefficient = 0.0;
  double common_pulse_five_source_bound = 0.0;
  double common_pulse_six_source_bound = 0.0;
  double common_five_source_margin = 0.0;
  double asynchronous_four_source_bound = 0.0;
  double asynchronous_four_source_margin = 0.0;
  double five_source_while_original_remains_bound = 0.0;
  double five_source_while_original_remains_margin = 0.0;
  double five_source_all_removed_envelope = 0.0;
  double five_source_all_removed_margin = 0.0;
  double maximum_gradient_stencil_ratio = 0.0;
};

struct CollectiveSourceHistoryArm {
  CollectiveSourceHistoryKind history =
      CollectiveSourceHistoryKind::LockedStep;
  int lattice_size = 0;
  int source_count = 0;
  int polarity = 0;
  int chirality = 0;
  int translation = 0;
  std::uint32_t seed = 0;
  int ticks = 0;
  int genesis_events = 0;
  int evaporation_events = 0;
  int first_genesis_tick = -1;
  int originals_remaining_before_first_genesis = -1;
  int all_originals_removed_tick = -1;
  double analytic_bound = 0.0;
  double maximum_flux_in_analytic_scope = 0.0;
  double maximum_bound_excess = 0.0;
  double maximum_velocity = 0.0;
  double maximum_remainder = 0.0;
  bool all_originals_removed = false;
  bool analytic_scope_respected = false;
  bool valid = false;
};

struct CollectiveSourceHistoryBoundResult {
  bool valid = false;
  std::vector<CollectiveSourceHistoryVolume> volumes;
  std::vector<CollectiveSourceHistoryArm> arms;
  int spectral_volume_count = 0;
  int common_history_arms = 0;
  int native_unlocked_arms = 0;
  int total_arms = 0;
  int total_ticks = 0;
  int common_history_genesis_events = 0;
  int asynchronous_four_source_genesis_events = 0;
  int unlocked_five_source_genesis_events = 0;
  int five_source_residual_tail_genesis_events = 0;
  int analytic_contradiction_events = 0;
  int unlocked_arms_all_sources_removed = 0;
  int evaporation_events = 0;
  int common_history_minimum_sources_not_excluded = 0;
  int asynchronous_minimum_sources_not_excluded = 0;
  double maximum_parseval_error = 0.0;
  double maximum_bound_excess = 0.0;
  double maximum_velocity = 0.0;
  double maximum_remainder = 0.0;
  bool stencil_dominance_derived = false;
  bool finite_group_parseval_derived = false;
  bool common_history_n_le_five_closed = false;
  bool asynchronous_n_le_four_closed = false;
  bool five_source_while_original_remains_closed = false;
  bool five_source_residual_tail_observed = false;
  bool five_source_residual_tail_unresolved = false;
  bool observer_neutral = false;
  bool production_changed = false;
};

const char* collective_source_history_name(CollectiveSourceHistoryKind kind);

CollectiveSourceHistoryBoundResult analyze_collective_source_history_bound();

}  // namespace ftd::eft
