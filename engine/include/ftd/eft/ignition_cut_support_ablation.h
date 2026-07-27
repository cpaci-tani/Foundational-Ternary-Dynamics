#pragma once
/**
 * @file ignition_cut_support_ablation.h
 * @brief Observer-only ignition-cut mechanism ablation (FTD-0587).
 */

#include <array>
#include <cstdint>
#include <vector>

namespace ftd::eft {

enum class IgnitionCutArm {
  IntactReservoir = 0,
  IntactCausal = 1,
  IntactProjected = 2,
  ClearedControl = 3,
  ClearedCausal = 4,
  ClearedProjected = 5,
};

const char* ignition_cut_arm_name(IgnitionCutArm arm);

struct IgnitionCutRunRecord {
  IgnitionCutArm arm = IgnitionCutArm::IntactReservoir;
  int lattice_size = 0;
  int amplitude = 0;
  std::uint32_t seed = 0;
  std::uint64_t prefix_state_hash = 0;
  std::uint64_t prefix_rng_hash = 0;
  std::uint64_t support_hash_before = 0;
  std::uint64_t support_hash_after_intervention = 0;
  int cut_occupancy = 0;
  int final_occupancy = 0;
  int sample_count = 0;
  int minimum_sample_occupancy = 0;
  int maximum_sample_occupancy = 0;
  int final_positive = 0;
  int final_negative = 0;
  int genesis_events = 0;
  int evaporation_events = 0;
  int movement_events = 0;
  int annihilation_events = 0;
  double cut_quadratic_field_norm = 0.0;
  double post_intervention_field_norm = 0.0;
  double final_quadratic_field_norm = 0.0;
  double maximum_quadratic_field_norm = 0.0;
  double maximum_flux = 0.0;
  double maximum_wave_velocity = 0.0;
  double maximum_gauss_error = 0.0;
  double maximum_velocity_before_rebase = 0.0;
  double maximum_remainder_before_rebase = 0.0;
  double maximum_velocity_after_rebase = 0.0;
  double maximum_remainder_after_rebase = 0.0;
  double occupancy_cv = 0.0;
  double radius_cv = 0.0;
  bool stable = false;
  bool finite = false;
  bool all_samples_valid = false;
  bool size_gate = false;
  bool prefix_kinematics_clean = false;
  bool support_preserved_by_intervention = false;
};

struct IgnitionCutArmSummary {
  IgnitionCutArm arm = IgnitionCutArm::IntactReservoir;
  int run_count = 0;
  int stable_runs = 0;
  int passing_cells = 0;
  int genesis_events = 0;
  int evaporation_events = 0;
  int movement_events = 0;
  int annihilation_events = 0;
  double mean_cut_field_norm = 0.0;
  double mean_post_intervention_field_norm = 0.0;
  double mean_final_field_norm = 0.0;
  double maximum_gauss_error = 0.0;
  bool support_qualified = false;
};

struct IgnitionCutSupportAblationResult {
  bool valid = false;
  std::vector<IgnitionCutRunRecord> runs;
  std::array<IgnitionCutArmSummary, 6> arms{};
  int run_count = 0;
  int prefix_ticks = 0;
  int continuation_ticks = 0;
  int total_ticks = 0;
  int distinct_prefix_cells = 0;
  int prefix_hash_mismatches = 0;
  int prefix_rng_mismatches = 0;
  int intervention_support_mismatches = 0;
  int nonfinite_runs = 0;
  int dirty_prefix_kinematics_runs = 0;
  int post_rebase_kinematics_runs = 0;
  int forbidden_event_runs = 0;
  bool intact_projected_reproduced = false;
  bool observer_neutral = false;
  bool reservoir_sufficient = false;
  bool causal_state_source_sufficient = false;
  bool gauss_constraint_sufficient = false;
  bool state_only_persistence = false;
  bool mixed_or_unresolved = false;
  bool no_registered_support_mechanism = false;
  bool production_changed = false;
};

IgnitionCutSupportAblationResult
analyze_ignition_cut_support_ablation();

}  // namespace ftd::eft
