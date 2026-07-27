#include "ftd/eft/ten_source_pair_distance_capacity.h"

#include <iomanip>
#include <iostream>

int main() {
  const ftd::eft::TenSourcePairDistanceCapacityResult result =
      ftd::eft::analyze_ten_source_pair_distance_capacity();
  std::cout << std::setprecision(17);
  std::cout << "FTD-0595 ten-source pair-distance capacity v1\n";
  for (const auto* animals : {&result.animals_l9, &result.animals_l17}) {
    std::cout << "animals,L," << animals->lattice_size;
    for (int size = 1; size <= 9; ++size) {
      std::cout << ",count" << size << ','
                << animals->canonical_animal_counts[
                       static_cast<std::size_t>(size)]
                << ",edges" << size << ','
                << animals->maximum_axial_edges[
                       static_cast<std::size_t>(size)];
    }
    std::cout << ",valid," << std::boolalpha << animals->valid << '\n';
  }
  for (const auto& volume : result.volumes) {
    std::cout
        << "volume,L," << volume.parent.parent.lattice_size
        << ",kappa1," << volume.axial_kernel
        << ",kappa2," << volume.second_kernel
        << ",second_d," << volume.second_kernel_dx << ':'
        << volume.second_kernel_dy << ':' << volume.second_kernel_dz
        << ",covariance," << volume.axial_covariance_residual
        << ",direct," << volume.direct_kernel_residual;
    for (int removed = 0; removed <= 9; ++removed) {
      std::cout << ",e" << removed << ','
                << volume.axial_edge_caps[static_cast<std::size_t>(removed)]
                << ",G" << removed << ','
                << volume.pair_gram_factors[
                       static_cast<std::size_t>(removed)];
    }
    for (int removed = 0; removed <= 10; ++removed) {
      std::cout << ",HP_r" << removed << ','
                << volume.pair_partition_bounds[
                       static_cast<std::size_t>(removed)];
    }
    std::cout
        << ",HP10," << volume.pair_distance_bound
        << ",r_star," << volume.maximizing_removed_count
        << ",margin," << volume.pair_distance_margin
        << ",parent_shells," << std::boolalpha
        << volume.exact_parent_shell_partition
        << ",coverage," << volume.exact_displacement_coverage
        << ",axial_max," << volume.axial_kernel_is_maximal
        << ",no_weaker," << volume.pair_bound_no_weaker
        << ",closed," << volume.ten_source_closed
        << ",valid," << volume.valid << '\n';
  }
  std::cout
      << "summary,volumes," << result.spectral_volume_count
      << ",derived," << result.pair_distance_bound_derived
      << ",animals," << result.exact_animal_enumeration_complete
      << ",kernels," << result.all_kernel_checks_pass
      << ",n10_closed," << result.arbitrary_removal_n_le_ten_closed
      << ",n10_inconclusive,"
      << result.ten_source_pair_distance_bound_inconclusive
      << ",threshold_shape," << result.threshold_dependent_shape_selected
      << ",history_search," << result.geometry_history_search_performed
      << ",schedule_search," << result.removal_schedule_search_performed
      << ",production_changed," << result.production_changed
      << ",valid," << result.valid << '\n';
  if (result.arbitrary_removal_n_le_ten_closed) {
    std::cout << "verdict,"
              << "ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_PAIR_DISTANCE_CAPACITY\n";
  } else if (result.ten_source_pair_distance_bound_inconclusive) {
    std::cout << "verdict,TEN_SOURCE_PAIR_DISTANCE_BOUND_INCONCLUSIVE\n";
  } else {
    std::cout << "verdict,PROTOCOL_INVALID\n";
  }
  if (!result.valid || result.production_changed
      || result.threshold_dependent_shape_selected
      || result.geometry_history_search_performed
      || result.removal_schedule_search_performed) return 1;
  return 0;
}
