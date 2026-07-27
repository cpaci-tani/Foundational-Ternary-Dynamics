#include "ftd/eft/ten_source_shared_m_coherence.h"

#include <cstring>
#include <iomanip>
#include <iostream>

int main(int argc, char** argv) {
  const bool dump_shells = argc == 2
      && std::strcmp(argv[1], "--dump-shells") == 0;
  const ftd::eft::TenSourceSharedMCoherenceResult result =
      ftd::eft::analyze_ten_source_shared_m_coherence();

  std::cout << std::setprecision(17);
  std::cout << "FTD-0594 ten-source exact shared-M coherence v1\n";
  for (const auto& volume : result.volumes) {
    std::cout
        << "volume,L," << volume.parent.lattice_size
        << ",degree," << volume.cyclotomic_degree
        << ",modes," << volume.parent.nonzero_mode_count
        << ",orbits," << volume.parent.mode_orbit_count
        << ",shells," << volume.eigenvalue_shell_count
        << ",multi_shells," << volume.multi_orbit_shell_count
        << ",max_orbits_per_shell," << volume.maximum_orbits_per_shell
        << ",shell_modes," << volume.shell_mode_count
        << ",max_d," << volume.maximizing_dx << ':'
        << volume.maximizing_dy << ':' << volume.maximizing_dz
        << ",mu_orbit," << volume.orbit_coherence_recomputed
        << ",mu_shared_m," << volume.maximum_shared_m_coherence
        << ",improvement," << volume.coherence_improvement
        << ",Q," << volume.pulse_operator_coefficient
        << ",C," << volume.common_step_coefficient;
    for (int removed = 0; removed <= 10; ++removed) {
      std::cout << ",HM_r" << removed << ','
                << volume.removal_partition_bounds[
                       static_cast<std::size_t>(removed)];
    }
    std::cout
        << ",HM10," << volume.ten_source_shared_m_bound
        << ",r_star," << volume.maximizing_removed_count
        << ",margin," << volume.ten_source_margin
        << ",invariance," << volume.maximum_orbit_invariance_residual
        << ",character," << volume.maximum_character_residual
        << ",regrouping," << volume.shell_regrouping_residual
        << ",cyclotomic," << std::boolalpha
        << volume.cyclotomic_identity_exact
        << ",key_invariance," << volume.exact_key_invariance
        << ",coverage," << volume.exact_shell_coverage
        << ",no_weaker," << volume.shared_m_no_weaker
        << ",closed," << volume.ten_source_closed
        << ",valid," << volume.valid << '\n';
    if (dump_shells) {
      for (const auto& shell : volume.shells) {
        std::cout << "shell,L," << volume.parent.lattice_size
                  << ",key," << shell.exact_key
                  << ",orbits," << shell.orbit_count
                  << ",modes," << shell.mode_count << '\n';
      }
    }
  }
  std::cout
      << "summary,volumes," << result.spectral_volume_count
      << ",derived," << result.exact_shared_m_bound_derived
      << ",cyclotomic," << result.all_cyclotomic_identities_exact
      << ",partitions," << result.all_shell_partitions_exact
      << ",cross_checks," << result.all_cross_checks_pass
      << ",n10_closed," << result.arbitrary_removal_n_le_ten_closed
      << ",n10_inconclusive,"
      << result.ten_source_shared_m_bound_inconclusive
      << ",approximate_clustering,"
      << result.approximate_eigenvalue_clustering_used
      << ",geometry_search," << result.geometry_search_performed
      << ",schedule_search," << result.removal_schedule_search_performed
      << ",production_changed," << result.production_changed
      << ",valid," << result.valid << '\n';

  if (result.arbitrary_removal_n_le_ten_closed) {
    std::cout << "verdict,"
              << "ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_SHARED_M_COHERENCE\n";
  } else if (result.ten_source_shared_m_bound_inconclusive) {
    std::cout << "verdict,TEN_SOURCE_SHARED_M_BOUND_INCONCLUSIVE\n";
  } else {
    std::cout << "verdict,PROTOCOL_INVALID\n";
  }

  if (!result.valid || result.production_changed
      || result.approximate_eigenvalue_clustering_used
      || result.geometry_search_performed
      || result.removal_schedule_search_performed) return 1;
  for (const auto& volume : result.volumes) {
    if (!volume.valid || !volume.cyclotomic_identity_exact
        || !volume.exact_key_invariance || !volume.exact_shell_coverage
        || !volume.exact_orbit_coverage
        || !volume.direct_character_verified
        || !volume.shell_regrouping_verified
        || !volume.shared_m_no_weaker
        || !volume.parent_scalars_reproduced) return 2;
  }
  return 0;
}
