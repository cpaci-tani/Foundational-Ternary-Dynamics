#include "ftd/eft/ten_source_distance_distribution_lp.h"

#include <iomanip>
#include <iostream>

#ifndef FTD_0596_CERTIFICATE_PATH
#error "FTD_0596_CERTIFICATE_PATH must be defined"
#endif

int main() {
  const auto result = ftd::eft::analyze_ten_source_distance_distribution_lp(
      FTD_0596_CERTIFICATE_PATH);
  std::cout << std::setprecision(17);
  std::cout << "FTD-0596 ten-source distance-distribution LP v1\n";
  for (const auto& volume : result.volumes) {
    std::cout
        << "volume,L," << volume.lattice_size
        << ",orbits," << volume.orbit_count
        << ",shells," << volume.shell_count
        << ",kappa_max," << volume.maximum_kernel
        << ",kappa_residual," << volume.kernel_table_residual
        << ",r_star," << volume.maximizing_removed_count
        << ",bound," << volume.distance_distribution_bound
        << ",margin," << volume.margin
        << ",coverage," << std::boolalpha << volume.exact_orbit_coverage
        << ",shell_partition," << volume.exact_shell_partition
        << ",certificate," << volume.certificate_matches
        << ",valid," << volume.valid << '\n';
    for (int removed = 2; removed <= 9; ++removed) {
      const auto& partition = volume.partitions[
          static_cast<std::size_t>(removed)];
      std::cout
          << "partition,L," << volume.lattice_size
          << ",r," << removed
          << ",a_support," << partition.primal_support_count
          << ",y_support," << partition.active_dual_count
          << ",primal," << partition.primal_objective
          << ",certified," << partition.certified_objective
          << ",gap," << partition.primal_dual_gap
          << ",min_fourier," << partition.minimum_fourier_value
          << ",min_dual," << partition.minimum_dual_slack
          << ",character," << partition.maximum_character_residual
          << ",normalization," << partition.normalization_residual
          << ",upper," << partition.upper_bound_residual
          << ",epsilon," << partition.epsilon_residual
          << ",delta," << partition.delta_residual
          << ",bound," << partition.partition_bound
          << ",primal_ok," << partition.primal_feasible
          << ",dual_ok," << partition.dual_certified
          << ",valid," << partition.valid << '\n';
    }
  }
  std::cout
      << "summary,volumes," << result.spectral_volume_count
      << ",primal," << result.all_primal_feasible
      << ",dual," << result.all_dual_certified
      << ",n10_closed," << result.arbitrary_removal_n_le_ten_closed
      << ",n10_inconclusive," << result.distance_distribution_lp_inconclusive
      << ",configuration_search," << result.configuration_search_performed
      << ",history_search," << result.history_search_performed
      << ",extra_cut," << result.extra_cut_added
      << ",production_changed," << result.production_changed
      << ",valid," << result.valid << '\n';
  if (result.arbitrary_removal_n_le_ten_closed) {
    std::cout
        << "verdict,ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_"
           "DISTANCE_DISTRIBUTION_LP\n";
  } else if (result.distance_distribution_lp_inconclusive) {
    std::cout
        << "verdict,TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_INCONCLUSIVE\n";
  } else {
    std::cout << "verdict,PROTOCOL_INVALID\n";
  }
  return result.valid && !result.production_changed
      && !result.configuration_search_performed && !result.history_search_performed
      && !result.extra_cut_added ? 0 : 1;
}
