/** FTD-0562: finite rigid source full-resonance obstruction. */

#include "ftd/constants.h"
#include "ftd/eft/full_surface_source_obstruction.h"

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
  const auto result = ftd::eft::analyze_full_surface_source_obstruction(
      ftd::C_WAVE*ftd::C_WAVE);
  check("observer verdict",result.valid);
  check("full-direction slow branch",
        result.full_direction_slow_branch_exists);
  check("finite-source analyticity",
        result.finite_source_form_factor_is_analytic);
  check("lowest homogeneous moment",
        result.lowest_homogeneous_moment_is_decisive);
  check("finite rigid cancellation closed",
        result.finite_rigid_universal_cancellation_closed);
  check("square-summable slow-hop dressing closed",
        result.square_summable_linear_dressing_closed_for_slow_hops);
  check("nonlinear branch preserved",
        result.nonlinear_deforming_carrier_remains_open);
  check("locked arm count",result.arms.size()==768);
  check("all witness groups",result.witness_groups==96
        && result.witness_groups==result.expected_witness_groups);
  check("denominator residual",result.maximum_denominator_residual<=1e-12);
  check("regularity",result.minimum_scaled_radial_derivative>1.0);
  check("polarity mirror",result.maximum_polarity_mirror_residual<=1e-12);
  check("cyclic covariance",result.maximum_cyclic_covariance_residual<=1e-12);
  check("radius asymptotic",
        result.maximum_t512_radius_correction_residual<0.25);
  check("forcing asymptotic",
        result.maximum_t512_asymptotic_error<0.20);
  check("positive scaled forcing",result.minimum_witness_scaled_forcing>0.0);

  std::cout << std::setprecision(17)
            << "arms=" << result.arms.size() << '\n'
            << "witness_groups=" << result.witness_groups << '\n'
            << "maximum_denominator_residual="
            << result.maximum_denominator_residual << '\n'
            << "minimum_scaled_radial_derivative="
            << result.minimum_scaled_radial_derivative << '\n'
            << "maximum_polarity_mirror_residual="
            << result.maximum_polarity_mirror_residual << '\n'
            << "maximum_cyclic_covariance_residual="
            << result.maximum_cyclic_covariance_residual << '\n'
            << "maximum_t512_radius_correction_residual="
            << result.maximum_t512_radius_correction_residual << '\n'
            << "maximum_t512_asymptotic_error="
            << result.maximum_t512_asymptotic_error << '\n'
            << "minimum_witness_scaled_forcing="
            << result.minimum_witness_scaled_forcing << '\n'
            << "verdict="
            << (result.valid
                ? "FINITE_RIGID_FULL_SURFACE_CANCELLATION_OBSTRUCTED"
                : "FULL_SURFACE_SOURCE_OBSTRUCTION_FAILED") << '\n'
            << "failures=" << failures << '\n';
  return failures==0 ? 0 : 1;
}
