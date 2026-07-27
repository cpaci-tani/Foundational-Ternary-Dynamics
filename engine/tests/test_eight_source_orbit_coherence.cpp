#include "ftd/eft/eight_source_orbit_coherence.h"

#include <iomanip>
#include <iostream>

int main() {
  const ftd::eft::EightSourceOrbitCoherenceResult result =
      ftd::eft::analyze_eight_source_orbit_coherence();

  std::cout << std::setprecision(17);
  std::cout << "FTD-0591 eight-source cubic-orbit coherence v1\n";
  for (const auto& volume : result.volumes) {
    const auto& spectral = volume.spectral;
    std::cout
        << "volume,L," << spectral.lattice_size
        << ",modes," << spectral.nonzero_mode_count
        << ",mode_orbits," << spectral.mode_orbit_count
        << ",displacement_orbits," << spectral.displacement_orbit_count
        << ",max_d," << spectral.maximizing_dx << ':'
        << spectral.maximizing_dy << ':' << spectral.maximizing_dz
        << ",A," << spectral.pulse_cauchy_sum
        << ",W," << spectral.gradient_weight_sum
        << ",Q," << spectral.pulse_operator_coefficient
        << ",mu," << spectral.maximum_orbit_coherence
        << ",C," << spectral.common_step_coefficient;
    for (int removed = 0; removed <= 8; ++removed) {
      std::cout << ",H8_r" << removed << ','
                << volume.removal_partition_bounds[
                       static_cast<std::size_t>(removed)];
    }
    std::cout
        << ",H8," << volume.eight_source_orbit_bound
        << ",r_star," << volume.maximizing_removed_count
        << ",margin," << volume.eight_source_margin
        << ",invariance," << spectral.maximum_orbit_invariance_residual
        << ",character," << spectral.maximum_character_residual
        << ",coverage," << std::boolalpha << spectral.exact_orbit_coverage
        << ",closed," << volume.eight_source_closed
        << ",valid," << volume.valid << '\n';
  }
  std::cout
      << "summary,volumes," << result.spectral_volume_count
      << ",parent_valid," << result.parent_orbit_analysis_valid
      << ",finite," << result.all_partition_bounds_finite
      << ",n8_closed," << result.arbitrary_removal_n_le_eight_closed
      << ",n8_inconclusive," << result.eight_source_bound_inconclusive
      << ",geometry_search," << result.geometry_search_performed
      << ",schedule_search," << result.removal_schedule_search_performed
      << ",production_changed," << result.production_changed
      << ",valid," << result.valid << '\n';

  if (result.arbitrary_removal_n_le_eight_closed) {
    std::cout
        << "verdict,ARBITRARY_REMOVAL_N_LE_8_CLOSED_BY_ORBIT_COHERENCE\n";
  } else if (result.eight_source_bound_inconclusive) {
    std::cout << "verdict,EIGHT_SOURCE_ORBIT_BOUND_INCONCLUSIVE\n";
  } else {
    std::cout << "verdict,PROTOCOL_INVALID\n";
  }

  if (!result.valid || result.production_changed
      || result.geometry_search_performed
      || result.removal_schedule_search_performed) return 1;
  for (const auto& volume : result.volumes) {
    if (!volume.valid || !volume.spectral.valid
        || !volume.spectral.exact_orbit_coverage
        || !volume.spectral.orbit_invariance_verified
        || !volume.spectral.direct_character_verified
        || !volume.all_partition_bounds_finite) return 2;
  }
  return 0;
}

