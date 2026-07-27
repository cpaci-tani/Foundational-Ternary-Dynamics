/** FTD-0583: noncompact matched-face cohomology/local-carrier gate. */

#include "ftd/eft/noncompact_face_cohomology.h"

#include <iostream>
#include <string>

namespace {
int failures = 0;
void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}
}  // namespace

int main() {
  const auto r = ftd::eft::analyze_noncompact_face_cohomology();
  check("all nonzero Fourier modes are exact and Betti numbers are 1,3,3,1",
        r.periodic_complex_exact_off_zero_mode
        && r.fourier_mode_arms == 728
        && r.zero_momentum_mode_arms == 4
        && r.nonzero_momentum_mode_arms == 724
        && r.fourier_rank_mismatches == 0
        && r.betti_volume_mismatches == 0
        && r.betti_0 == 1 && r.betti_1 == 3
        && r.betti_2 == 3 && r.betti_3 == 1);
  check("face cohomology consists only of three global real fluxes",
        r.face_cohomology_is_three_global_real_fluxes
        && r.harmonic_arms == 48
        && r.maximum_divergence_of_curl <= 1e-12
        && r.maximum_curl_plane_flux <= 1e-12
        && r.maximum_harmonic_plane_residual <= 1e-12
        && r.maximum_harmonic_flux_change_under_curl <= 1e-12);
  check("all localized zero-harmonic fixtures contract to vacuum",
        r.localized_zero_harmonic_fields_contractible
        && r.localized_curl_arms == 24
        && r.contraction_samples == 120
        && r.minimum_localized_support > 0
        && r.minimum_nonzero_localized_energy > 0.0
        && r.maximum_contraction_divergence <= 1e-12
        && r.maximum_contraction_harmonic_flux <= 1e-12
        && r.maximum_contraction_energy_residual <= 1e-12
        && r.maximum_contraction_curl_residual <= 1e-12
        && r.maximum_contraction_support_excess == 0);
  check("periodic Gauss dipoles scale continuously with zero total charge",
        r.real_gauss_charge_continuously_scalable
        && r.charge_scaling_arms == 120
        && r.maximum_periodic_charge_sum <= 1e-12
        && r.maximum_charge_scaling_residual <= 1e-12
        && r.maximum_off_source_divergence <= 1e-12
        && r.maximum_surface_telescope_residual <= 1e-12);
  check("proper cubic rotations preserve curl, energy, and harmonic flux",
        r.cubic_rotation_arms == 24
        && r.maximum_cubic_covariance_residual <= 1e-12);
  check("no compact bundle or localized protected carrier is promoted",
        !r.compact_u1_structure_derived
        && !r.localized_protected_carrier_in_current_variables
        && !r.production_changed);
  check("registered FTD-0583 verdict closes", r.valid);

  std::cout.precision(17);
  std::cout << "fourier_mode_arms=" << r.fourier_mode_arms << '\n'
    << "zero_momentum_mode_arms=" << r.zero_momentum_mode_arms << '\n'
    << "nonzero_momentum_mode_arms=" << r.nonzero_momentum_mode_arms << '\n'
    << "fourier_rank_mismatches=" << r.fourier_rank_mismatches << '\n'
    << "betti_volume_mismatches=" << r.betti_volume_mismatches << '\n'
    << "harmonic_arms=" << r.harmonic_arms << '\n'
    << "localized_curl_arms=" << r.localized_curl_arms << '\n'
    << "contraction_samples=" << r.contraction_samples << '\n'
    << "charge_scaling_arms=" << r.charge_scaling_arms << '\n'
    << "cubic_rotation_arms=" << r.cubic_rotation_arms << '\n'
    << "betti=" << r.betti_0 << ',' << r.betti_1 << ','
    << r.betti_2 << ',' << r.betti_3 << '\n'
    << "minimum_localized_support=" << r.minimum_localized_support << '\n'
    << "maximum_localized_support=" << r.maximum_localized_support << '\n'
    << "maximum_symbol_complex_residual="
    << r.maximum_symbol_complex_residual << '\n'
    << "maximum_divergence_of_curl=" << r.maximum_divergence_of_curl << '\n'
    << "maximum_curl_plane_flux=" << r.maximum_curl_plane_flux << '\n'
    << "maximum_harmonic_plane_residual="
    << r.maximum_harmonic_plane_residual << '\n'
    << "maximum_harmonic_flux_change_under_curl="
    << r.maximum_harmonic_flux_change_under_curl << '\n'
    << "maximum_contraction_energy_residual="
    << r.maximum_contraction_energy_residual << '\n'
    << "minimum_nonzero_localized_energy="
    << r.minimum_nonzero_localized_energy << '\n'
    << "maximum_periodic_charge_sum="
    << r.maximum_periodic_charge_sum << '\n'
    << "maximum_charge_scaling_residual="
    << r.maximum_charge_scaling_residual << '\n'
    << "maximum_surface_telescope_residual="
    << r.maximum_surface_telescope_residual << '\n'
    << "maximum_cubic_covariance_residual="
    << r.maximum_cubic_covariance_residual << '\n'
    << "maximum_curl_covariance_residual="
    << r.maximum_curl_covariance_residual << '\n'
    << "maximum_rotated_divergence="
    << r.maximum_rotated_divergence << '\n'
    << "maximum_rotation_energy_residual="
    << r.maximum_rotation_energy_residual << '\n'
    << "maximum_rotated_harmonic_plane_residual="
    << r.maximum_rotated_harmonic_plane_residual << '\n'
    << "maximum_harmonic_rotation_residual="
    << r.maximum_harmonic_rotation_residual << '\n'
    << "noncompact_face_cohomology failures=" << failures << '\n'
    << "verdict=MATCHED_NONCOMPACT_COHOMOLOGY_GLOBAL_ONLY_LOCAL_PROTECTED_DEFECT_CLOSED\n";
  return failures == 0 ? 0 : 1;
}
