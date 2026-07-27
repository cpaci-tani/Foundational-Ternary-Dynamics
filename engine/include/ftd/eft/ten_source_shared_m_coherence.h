#pragma once
/**
 * @file ten_source_shared_m_coherence.h
 * @brief Exact shared-stencil-eigenvalue refinement at N=10 (FTD-0594).
 *
 * Observer-only.  Exact cyclotomic keys group every cubic orbit on which the
 * native pulse coefficient is forced to be identical.
 */

#include "ftd/eft/removal_time_orbit_coherence.h"

#include <array>
#include <string>
#include <vector>

namespace ftd::eft {

struct SharedMEigenshellRecord {
  std::string exact_key;
  int orbit_count = 0;
  int mode_count = 0;
};

struct TenSourceSharedMCoherenceVolume {
  RemovalTimeOrbitCoherenceVolume parent{};
  std::vector<SharedMEigenshellRecord> shells;
  std::array<double, 11> removal_partition_bounds{};
  int cyclotomic_degree = 0;
  int eigenvalue_shell_count = 0;
  int multi_orbit_shell_count = 0;
  int maximum_orbits_per_shell = 0;
  int shell_mode_count = 0;
  int maximizing_dx = 0;
  int maximizing_dy = 0;
  int maximizing_dz = 0;
  int maximizing_removed_count = -1;
  double maximum_shared_m_coherence = 0.0;
  double orbit_coherence_recomputed = 0.0;
  double coherence_improvement = 0.0;
  double pulse_operator_coefficient = 0.0;
  double common_step_coefficient = 0.0;
  double ten_source_shared_m_bound = 0.0;
  double ten_source_margin = 0.0;
  double maximum_orbit_invariance_residual = 0.0;
  double maximum_character_residual = 0.0;
  double shell_regrouping_residual = 0.0;
  bool cyclotomic_identity_exact = false;
  bool exact_key_invariance = false;
  bool exact_shell_coverage = false;
  bool exact_orbit_coverage = false;
  bool direct_character_verified = false;
  bool shell_regrouping_verified = false;
  bool shared_m_no_weaker = false;
  bool parent_scalars_reproduced = false;
  bool all_partition_bounds_finite = false;
  bool ten_source_closed = false;
  bool valid = false;
};

struct TenSourceSharedMCoherenceResult {
  std::vector<TenSourceSharedMCoherenceVolume> volumes;
  int registered_source_count = 10;
  int spectral_volume_count = 0;
  bool exact_shared_m_bound_derived = false;
  bool all_cyclotomic_identities_exact = false;
  bool all_shell_partitions_exact = false;
  bool all_cross_checks_pass = false;
  bool arbitrary_removal_n_le_ten_closed = false;
  bool ten_source_shared_m_bound_inconclusive = false;
  bool approximate_eigenvalue_clustering_used = false;
  bool geometry_search_performed = false;
  bool removal_schedule_search_performed = false;
  bool production_changed = false;
  bool valid = false;
};

TenSourceSharedMCoherenceResult analyze_ten_source_shared_m_coherence();

}  // namespace ftd::eft
