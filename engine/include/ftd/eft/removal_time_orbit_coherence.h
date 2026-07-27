#pragma once
/**
 * @file removal_time_orbit_coherence.h
 * @brief Cubic-orbit coherence bound for arbitrary removal histories (FTD-0590).
 *
 * Observer-only.  This module evaluates a finite spectral upper bound and
 * does not modify production state, toggles, scenarios, or tick ordering.
 */

#include <vector>

namespace ftd::eft {

struct RemovalTimeOrbitCoherenceVolume {
  int lattice_size = 0;
  int nonzero_mode_count = 0;
  int mode_orbit_count = 0;
  int displacement_orbit_count = 0;
  int maximizing_dx = 0;
  int maximizing_dy = 0;
  int maximizing_dz = 0;
  int maximizing_removed_count_at_seven = 0;
  double pulse_cauchy_sum = 0.0;
  double gradient_weight_sum = 0.0;
  double pulse_operator_coefficient = 0.0;
  double maximum_orbit_coherence = 0.0;
  double common_step_coefficient = 0.0;
  double seven_source_orbit_bound = 0.0;
  double seven_source_margin = 0.0;
  double maximum_orbit_invariance_residual = 0.0;
  double maximum_character_residual = 0.0;
  bool exact_orbit_coverage = false;
  bool orbit_invariance_verified = false;
  bool direct_character_verified = false;
  bool coherence_in_unit_interval = false;
  bool seven_source_closed = false;
  bool valid = false;
};

struct RemovalTimeOrbitCoherenceResult {
  std::vector<RemovalTimeOrbitCoherenceVolume> volumes;
  int spectral_volume_count = 0;
  int uniform_closed_source_count = 6;
  int first_source_count_tested = 7;
  bool cubic_orbit_bound_derived = false;
  bool all_orbit_partitions_exact = false;
  bool all_direct_character_checks_pass = false;
  bool arbitrary_removal_n_le_seven_closed = false;
  bool seven_source_bound_inconclusive = false;
  bool production_changed = false;
  bool valid = false;
};

RemovalTimeOrbitCoherenceResult analyze_removal_time_orbit_coherence();

}  // namespace ftd::eft

