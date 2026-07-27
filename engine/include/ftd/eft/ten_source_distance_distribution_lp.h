#pragma once
/**
 * @file ten_source_distance_distribution_lp.h
 * @brief Sparse dual-certificate verifier for FTD-0596.
 *
 * Observer-only.  The verifier reconstructs the complete cubic translation
 * association scheme and checks externally generated Delsarte LP certificates.
 * It never modifies production state.
 */

#include <array>
#include <string>
#include <vector>

namespace ftd::eft {

struct DistanceDistributionPartitionRecord {
  int removed_count = -1;
  int primal_support_count = 0;
  int active_dual_count = 0;
  double primal_objective = 0.0;
  double certified_objective = 0.0;
  double primal_dual_gap = 0.0;
  double minimum_fourier_value = 0.0;
  double minimum_dual_slack = 0.0;
  double maximum_character_residual = 0.0;
  double normalization_residual = 0.0;
  double upper_bound_residual = 0.0;
  double epsilon_residual = 0.0;
  double delta_residual = 0.0;
  double gram_factor = 0.0;
  double partition_bound = 0.0;
  bool primal_feasible = false;
  bool dual_certified = false;
  bool valid = false;
};

struct TenSourceDistanceDistributionVolume {
  int lattice_size = 0;
  int orbit_count = 0;
  int shell_count = 0;
  int maximizing_removed_count = -1;
  std::array<int, 3> maximum_kernel_displacement{};
  std::array<DistanceDistributionPartitionRecord, 11> partitions{};
  double maximum_kernel = 0.0;
  double maximum_kernel_residual = 0.0;
  double kernel_table_residual = 0.0;
  double pulse_operator_coefficient = 0.0;
  double common_step_coefficient = 0.0;
  double distance_distribution_bound = 0.0;
  double margin = 0.0;
  bool exact_orbit_coverage = false;
  bool exact_shell_partition = false;
  bool certificate_matches = false;
  bool valid = false;
};

struct TenSourceDistanceDistributionResult {
  std::vector<TenSourceDistanceDistributionVolume> volumes;
  int registered_source_count = 10;
  int spectral_volume_count = 0;
  bool all_primal_feasible = false;
  bool all_dual_certified = false;
  bool arbitrary_removal_n_le_ten_closed = false;
  bool distance_distribution_lp_inconclusive = false;
  bool configuration_search_performed = false;
  bool history_search_performed = false;
  bool extra_cut_added = false;
  bool production_changed = false;
  bool valid = false;
};

TenSourceDistanceDistributionResult
analyze_ten_source_distance_distribution_lp(
    const std::string& certificate_csv_path);

}  // namespace ftd::eft
