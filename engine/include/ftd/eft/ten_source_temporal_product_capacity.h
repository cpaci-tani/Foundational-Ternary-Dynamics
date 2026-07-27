#pragma once
/**
 * @file ten_source_temporal_product_capacity.h
 * @brief Observer-only verifier for the FTD-0597 temporal product capacity.
 */

#include "ftd/eft/ten_source_distance_distribution_lp.h"

#include <array>
#include <string>
#include <vector>

namespace ftd::eft {

struct TenSourceTemporalProductVolume {
  int lattice_size = 0;
  int orbit_count = 0;
  int shell_count = 0;
  int maximizing_removed_count = -1;
  std::array<int, 3> maximum_temporal_kernel_displacement{};
  std::array<DistanceDistributionPartitionRecord, 11> partitions{};
  double maximum_temporal_kernel = 0.0;
  double maximum_parent_kernel = 0.0;
  double temporal_kernel_table_residual = 0.0;
  double parent_kernel_table_residual = 0.0;
  double positive_mass_table_residual = 0.0;
  double negative_mass_table_residual = 0.0;
  double maximum_alternate_formula_residual = 0.0;
  double maximum_parent_excess = 0.0;
  double pulse_operator_coefficient = 0.0;
  double common_step_coefficient = 0.0;
  double temporal_product_bound = 0.0;
  double parent_distance_distribution_bound = 0.0;
  double margin = 0.0;
  bool exact_orbit_coverage = false;
  bool exact_shell_partition = false;
  bool product_interval_verified = false;
  bool certificate_matches = false;
  bool valid = false;
};

struct TenSourceTemporalProductResult {
  std::vector<TenSourceTemporalProductVolume> volumes;
  int registered_source_count = 10;
  int spectral_volume_count = 0;
  bool exact_pulse_product_lemma = false;
  bool all_primal_feasible = false;
  bool all_dual_certified = false;
  bool arbitrary_removal_n_le_ten_closed = false;
  bool temporal_product_bound_inconclusive = false;
  bool configuration_search_performed = false;
  bool polarity_search_performed = false;
  bool history_search_performed = false;
  bool time_scan_performed = false;
  bool extra_cut_added = false;
  bool production_changed = false;
  bool valid = false;
};

TenSourceTemporalProductResult
analyze_ten_source_temporal_product_capacity(
    const std::string& certificate_csv_path,
    const std::string& parent_certificate_csv_path);

}  // namespace ftd::eft
