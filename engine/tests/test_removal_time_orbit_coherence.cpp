#include "ftd/eft/removal_time_orbit_coherence.h"

#include <cmath>
#include <iomanip>
#include <iostream>

int main() {
  const ftd::eft::RemovalTimeOrbitCoherenceResult result =
      ftd::eft::analyze_removal_time_orbit_coherence();

  std::cout << std::setprecision(17);
  std::cout << "FTD-0590 removal-time cubic-orbit coherence v1\n";
  for (const auto& volume : result.volumes) {
    std::cout
        << "volume,L," << volume.lattice_size
        << ",modes," << volume.nonzero_mode_count
        << ",mode_orbits," << volume.mode_orbit_count
        << ",displacement_orbits," << volume.displacement_orbit_count
        << ",max_d," << volume.maximizing_dx << ':'
        << volume.maximizing_dy << ':' << volume.maximizing_dz
        << ",A," << volume.pulse_cauchy_sum
        << ",W," << volume.gradient_weight_sum
        << ",Q," << volume.pulse_operator_coefficient
        << ",mu," << volume.maximum_orbit_coherence
        << ",C," << volume.common_step_coefficient
        << ",H7," << volume.seven_source_orbit_bound
        << ",r_star," << volume.maximizing_removed_count_at_seven
        << ",margin," << volume.seven_source_margin
        << ",invariance," << volume.maximum_orbit_invariance_residual
        << ",character," << volume.maximum_character_residual
        << ",coverage," << std::boolalpha << volume.exact_orbit_coverage
        << ",closed," << volume.seven_source_closed
        << ",valid," << volume.valid << '\n';
  }
  std::cout
      << "summary,volumes," << result.spectral_volume_count
      << ",orbit_bound," << result.cubic_orbit_bound_derived
      << ",coverage," << result.all_orbit_partitions_exact
      << ",character," << result.all_direct_character_checks_pass
      << ",n7_closed," << result.arbitrary_removal_n_le_seven_closed
      << ",n7_inconclusive," << result.seven_source_bound_inconclusive
      << ",production_changed," << result.production_changed
      << ",valid," << result.valid << '\n';

  if (result.arbitrary_removal_n_le_seven_closed) {
    std::cout << "verdict,ARBITRARY_REMOVAL_N_LE_7_CLOSED_BY_ORBIT_COHERENCE\n";
  } else if (result.seven_source_bound_inconclusive) {
    std::cout << "verdict,ORBIT_COHERENCE_BOUND_INCONCLUSIVE_AT_N7\n";
  } else {
    std::cout << "verdict,PROTOCOL_INVALID\n";
  }

  if (!result.valid || result.production_changed) return 1;
  for (const auto& volume : result.volumes) {
    if (!volume.valid || !volume.exact_orbit_coverage
        || !volume.orbit_invariance_verified
        || !volume.direct_character_verified
        || !volume.coherence_in_unit_interval) return 2;
  }
  return 0;
}

