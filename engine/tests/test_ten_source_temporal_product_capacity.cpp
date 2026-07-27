#include "ftd/eft/ten_source_temporal_product_capacity.h"

#include <cstddef>
#include <iomanip>
#include <iostream>

#ifndef FTD_0597_CERTIFICATE_PATH
#error "FTD_0597_CERTIFICATE_PATH must be defined"
#endif

#ifndef FTD_0596_CERTIFICATE_PATH
#error "FTD_0596_CERTIFICATE_PATH must be defined"
#endif

int main() {
  const auto result = ftd::eft::analyze_ten_source_temporal_product_capacity(
      FTD_0597_CERTIFICATE_PATH, FTD_0596_CERTIFICATE_PATH);
  std::cout << std::setprecision(17);
  std::cout << "FTD-0597 ten-source temporal product capacity v1\n";
  for (const auto& volume : result.volumes) {
    std::cout
        << "volume,L," << volume.lattice_size
        << ",orbits," << volume.orbit_count
        << ",shells," << volume.shell_count
        << ",tau_max," << volume.maximum_temporal_kernel
        << ",parent_max," << volume.maximum_parent_kernel
        << ",tau_residual," << volume.temporal_kernel_table_residual
        << ",parent_residual," << volume.parent_kernel_table_residual
        << ",positive_residual," << volume.positive_mass_table_residual
        << ",negative_residual," << volume.negative_mass_table_residual
        << ",alternate," << volume.maximum_alternate_formula_residual
        << ",parent_excess," << volume.maximum_parent_excess
        << ",r_star," << volume.maximizing_removed_count
        << ",bound," << volume.temporal_product_bound
        << ",parent_bound," << volume.parent_distance_distribution_bound
        << ",margin," << volume.margin
        << ",product_interval," << std::boolalpha
        << volume.product_interval_verified
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
          << ",bound," << partition.partition_bound
          << ",primal_ok," << partition.primal_feasible
          << ",dual_ok," << partition.dual_certified
          << ",valid," << partition.valid << '\n';
    }
  }
  std::cout
      << "summary,volumes," << result.spectral_volume_count
      << ",product_lemma," << result.exact_pulse_product_lemma
      << ",primal," << result.all_primal_feasible
      << ",dual," << result.all_dual_certified
      << ",n10_closed," << result.arbitrary_removal_n_le_ten_closed
      << ",n10_inconclusive," << result.temporal_product_bound_inconclusive
      << ",configuration_search," << result.configuration_search_performed
      << ",polarity_search," << result.polarity_search_performed
      << ",history_search," << result.history_search_performed
      << ",time_scan," << result.time_scan_performed
      << ",extra_cut," << result.extra_cut_added
      << ",production_changed," << result.production_changed
      << ",valid," << result.valid << '\n';
  if (result.arbitrary_removal_n_le_ten_closed) {
    std::cout
        << "verdict,ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_"
           "TEMPORAL_PRODUCT_CAPACITY\n";
  } else if (result.temporal_product_bound_inconclusive) {
    std::cout
        << "verdict,TEN_SOURCE_TEMPORAL_PRODUCT_BOUND_INCONCLUSIVE\n";
  } else {
    std::cout << "verdict,PROTOCOL_INVALID\n";
  }
  return result.valid && result.exact_pulse_product_lemma
      && result.arbitrary_removal_n_le_ten_closed
      && !result.production_changed && !result.configuration_search_performed
      && !result.polarity_search_performed && !result.history_search_performed
      && !result.time_scan_performed && !result.extra_cut_added ? 0 : 1;
}
