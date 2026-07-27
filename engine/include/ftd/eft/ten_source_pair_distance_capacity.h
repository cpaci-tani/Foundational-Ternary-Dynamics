#pragma once
/**
 * @file ten_source_pair_distance_capacity.h
 * @brief Two-class pair-distance capacity bound at N=10 (FTD-0595).
 *
 * Observer-only.  The module combines the exact shared-M kernel with an exact
 * cubic-animal axial-edge capacity; production state is never modified.
 */

#include "ftd/eft/ten_source_shared_m_coherence.h"

#include <array>
#include <cstdint>
#include <vector>

namespace ftd::eft {

struct CubicAnimalCapacityRecord {
  int lattice_size = 0;
  std::array<std::uint64_t, 10> canonical_animal_counts{};
  std::array<int, 10> maximum_axial_edges{};
  bool connected_growth_complete = false;
  bool valid = false;
};

struct TenSourcePairDistanceCapacityVolume {
  TenSourceSharedMCoherenceVolume parent{};
  std::array<double, 11> pair_partition_bounds{};
  std::array<double, 10> pair_gram_factors{};
  std::array<int, 10> axial_edge_caps{};
  int maximizing_removed_count = -1;
  int second_kernel_dx = 0;
  int second_kernel_dy = 0;
  int second_kernel_dz = 0;
  double axial_kernel = 0.0;
  double second_kernel = 0.0;
  double axial_covariance_residual = 0.0;
  double direct_kernel_residual = 0.0;
  double pair_distance_bound = 0.0;
  double pair_distance_margin = 0.0;
  bool exact_parent_shell_partition = false;
  bool exact_displacement_coverage = false;
  bool axial_kernel_is_maximal = false;
  bool cubic_covariance_verified = false;
  bool direct_kernel_verified = false;
  bool pair_bound_no_weaker = false;
  bool all_partition_bounds_finite = false;
  bool ten_source_closed = false;
  bool valid = false;
};

struct TenSourcePairDistanceCapacityResult {
  std::vector<TenSourcePairDistanceCapacityVolume> volumes;
  CubicAnimalCapacityRecord animals_l9{};
  CubicAnimalCapacityRecord animals_l17{};
  int registered_source_count = 10;
  int spectral_volume_count = 0;
  bool pair_distance_bound_derived = false;
  bool exact_animal_enumeration_complete = false;
  bool all_kernel_checks_pass = false;
  bool arbitrary_removal_n_le_ten_closed = false;
  bool ten_source_pair_distance_bound_inconclusive = false;
  bool threshold_dependent_shape_selected = false;
  bool geometry_history_search_performed = false;
  bool removal_schedule_search_performed = false;
  bool production_changed = false;
  bool valid = false;
};

TenSourcePairDistanceCapacityResult
analyze_ten_source_pair_distance_capacity();

}  // namespace ftd::eft
