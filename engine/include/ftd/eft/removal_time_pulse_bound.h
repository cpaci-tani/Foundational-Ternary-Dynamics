#pragma once
/**
 * @file removal_time_pulse_bound.h
 * @brief Observer-only exact removal-history pulse bounds (FTD-0589).
 */

#include <cstdint>
#include <vector>

namespace ftd::eft {

enum class RemovalHistoryKind : std::uint8_t {
  PermanentStep = 0,
  SynchronousPulse = 1,
  StaggeredPulse = 2,
  PairedPulse = 3,
  NativeUnlocked = 4,
};

struct RemovalTimePulseVolume {
  int lattice_size = 0;
  int uniform_closed_source_count = 0;
  int first_source_count_not_excluded = 0;
  int maximizing_removed_at_closed_count = 0;
  int maximizing_removed_at_first_open_count = 0;
  double maximum_mode_eigenvalue = 0.0;
  double one_source_step_triangle_bound = 0.0;
  double exact_one_source_pulse_bound = 0.0;
  double common_step_coefficient = 0.0;
  double closed_count_history_bound = 0.0;
  double closed_count_margin = 0.0;
  double first_open_count_history_bound = 0.0;
  double first_open_count_margin = 0.0;
  double continuous_relaxation_at_closed_count = 0.0;
};

struct RemovalTimePulseArm {
  RemovalHistoryKind history = RemovalHistoryKind::PermanentStep;
  int lattice_size = 0;
  int source_count = 0;
  int polarity = 0;
  int shape_variant = 0;
  std::uint32_t seed = 0;
  int ticks = 0;
  int genesis_events = 0;
  int evaporation_events = 0;
  int all_originals_removed_tick = -1;
  double analytic_bound = 0.0;
  double maximum_flux = 0.0;
  double maximum_bound_excess = 0.0;
  double maximum_velocity = 0.0;
  double maximum_remainder = 0.0;
  bool all_originals_removed = false;
  bool analytic_scope_respected = false;
  bool valid = false;
};

struct RemovalTimePulseBoundResult {
  bool valid = false;
  std::vector<RemovalTimePulseVolume> volumes;
  std::vector<RemovalTimePulseArm> arms;
  int spectral_volume_count = 0;
  int uniform_closed_source_count = 0;
  int first_source_count_not_excluded = 0;
  int prescribed_history_arms = 0;
  int native_unlocked_arms = 0;
  int total_arms = 0;
  int total_ticks = 0;
  int genesis_events = 0;
  int evaporation_events = 0;
  int analytic_contradiction_events = 0;
  int unlocked_cells_with_complete_removal = 0;
  int pulse_identity_checks = 0;
  int gram_checks = 0;
  int proper_cubic_rotation_arms = 0;
  double maximum_pulse_identity_residual = 0.0;
  double maximum_gram_residual = 0.0;
  double maximum_translation_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double maximum_observed_flux = 0.0;
  double maximum_bound_excess = 0.0;
  double maximum_velocity = 0.0;
  double maximum_remainder = 0.0;
  bool exact_pulse_identity_derived = false;
  bool continuous_relaxation_derived = false;
  bool arbitrary_removal_n_le_six_closed = false;
  bool seven_source_bound_inconclusive = false;
  bool gram_identity_verified = false;
  bool translation_covariant = false;
  bool cubic_covariant = false;
  bool residual_branch_exercised = false;
  bool observer_neutral = false;
  bool production_changed = false;
};

const char* removal_history_name(RemovalHistoryKind kind);

RemovalTimePulseBoundResult analyze_removal_time_pulse_bound();

}  // namespace ftd::eft
