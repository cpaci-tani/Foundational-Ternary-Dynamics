/** FTD-0563: Gauss monopole / mobile-dressing dichotomy. */

#include "ftd/eft/native_gauss_monopole_dichotomy.h"

#include <iomanip>
#include <iostream>
#include <string>

int main() {
  int failures = 0;
  const auto check = [&](const std::string& label,bool condition) {
    if (!condition) {
      std::cerr << "FAIL: " << label << '\n';
      ++failures;
    }
  };
  const auto result =
      ftd::eft::analyze_native_gauss_monopole_dichotomy();
  check("observer verdict",result.valid);
  check("periodic divergence zero sum",
        result.periodic_divergence_zero_sum);
  check("production zero mode subtracted",
        result.production_zero_mode_subtracted);
  check("non-neutral matched source rejected",
        result.matched_non_neutral_rejected);
  check("neutral matched source accepted",
        result.matched_neutral_accepted);
  check("open monopole equals net polarity",
        result.infinite_volume_monopole_equals_net_polarity);
  check("neutral finite profile has no monopole",
        result.neutral_finite_profile_has_no_monopole);
  check("solenoidal dressing preserves monopole",
        result.solenoidal_dressing_cannot_change_monopole);
  check("native susceptibility finite",
        result.native_ir_susceptibility_is_finite);
  check("fixed finite charged carrier closed",
        result.fixed_finite_linear_charged_carrier_closed);
  check("nonlinear/topological branch retained",
        result.nonlinear_topological_effective_charge_remains_open);
  check("locked arm count",result.arms.size()==384);
  check("all witness groups",result.witness_groups==96
        && result.witness_groups==result.expected_witness_groups);
  check("zero-mode numerator exact",
        result.maximum_zero_mode_numerator_sum==0);
  check("periodic telescope",result.periodic_telescope_residual<=1e-12);
  check("neutral Gauss solve",result.matched_neutral_gauss_residual<=1e-9);
  check("div curl",result.maximum_curl_divergence<=1e-12);
  check("closed flux invariant",
        result.maximum_closed_surface_flux_change<=1e-12);
  check("face Gauss Fourier identity",
        result.maximum_face_gauss_identity_residual<=1e-12);
  check("point monopole coefficient",
        result.maximum_point_monopole_error<=1e-12);
  check("neutral infrared estimator",
        result.maximum_l256_neutral_monopole_estimator<0.1);
  check("multipole asymptotic",
        result.maximum_l256_asymptotic_error<0.02);
  check("polarity mirror",result.maximum_polarity_mirror_residual<=1e-12);
  check("cyclic covariance",
        result.maximum_cyclic_covariance_residual<=1e-12);

  std::cout << std::setprecision(17)
            << "arms=" << result.arms.size() << '\n'
            << "witness_groups=" << result.witness_groups << '\n'
            << "monotone_neutral_witnesses="
            << result.monotone_neutral_witnesses << '\n'
            << "maximum_zero_mode_numerator_sum="
            << result.maximum_zero_mode_numerator_sum << '\n'
            << "periodic_telescope_residual="
            << result.periodic_telescope_residual << '\n'
            << "matched_neutral_gauss_residual="
            << result.matched_neutral_gauss_residual << '\n'
            << "maximum_curl_divergence="
            << result.maximum_curl_divergence << '\n'
            << "maximum_closed_surface_flux_change="
            << result.maximum_closed_surface_flux_change << '\n'
            << "maximum_face_gauss_identity_residual="
            << result.maximum_face_gauss_identity_residual << '\n'
            << "maximum_point_monopole_error="
            << result.maximum_point_monopole_error << '\n'
            << "maximum_l256_neutral_monopole_estimator="
            << result.maximum_l256_neutral_monopole_estimator << '\n'
            << "maximum_l256_asymptotic_error="
            << result.maximum_l256_asymptotic_error << '\n'
            << "maximum_polarity_mirror_residual="
            << result.maximum_polarity_mirror_residual << '\n'
            << "maximum_cyclic_covariance_residual="
            << result.maximum_cyclic_covariance_residual << '\n'
            << "verdict="
            << (result.valid
                ? "GAUSS_MONOPOLE_MOBILE_DRESSING_DICHOTOMY_PROVED"
                : "GAUSS_MONOPOLE_MOBILE_DRESSING_DICHOTOMY_FAILED")
            << '\n'
            << "failures=" << failures << '\n';
  return failures==0 ? 0 : 1;
}
