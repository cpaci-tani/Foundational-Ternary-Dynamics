/** FTD-0561: periodic-hop finite-source multipole hierarchy. */

#include "ftd/constants.h"
#include "ftd/eft/hop_source_multipole_hierarchy.h"

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
  const auto result = ftd::eft::analyze_hop_source_multipole_hierarchy(
      ftd::C_WAVE*ftd::C_WAVE);
  check("observer verdict",result.valid);
  check("multipole theorem",result.finite_source_multipole_theorem);
  check("charged T^-2 term",result.charged_extension_retains_t2_forcing);
  check("neutral hierarchy",result.neutrality_raises_suppression_order);
  check("plane-neutral cancellation condition",
        result.axial_interval_cancellation_requires_plane_neutrality);
  check("axial cancellation is insufficient",
        result.axial_cancellation_is_not_full_surface_cancellation);
  check("locked arm count",result.arms.size()==96);
  check("pole residual",result.maximum_denominator_residual<=1e-12);
  check("form factor residual",result.maximum_form_factor_residual<=1e-12);
  check("positive forcing",result.minimum_normalized_forcing>0.0);
  check("polarity mirror",result.maximum_polarity_mirror_residual<=1e-12);
  check("cubic covariance",result.maximum_cubic_covariance_residual<=1e-12);
  check("point asymptotic",result.point_t256_error<0.01);
  check("pair asymptotic",result.pair_t256_error<0.01);
  check("dipole asymptotic",result.dipole_t256_error<0.02);
  check("quadrupole asymptotic",result.quadrupole_t256_error<0.03);
  check("same-plane axial zero",result.same_plane_axial_residual<=1e-12);
  check("same-plane oblique nonzero",
        result.same_plane_oblique_amplitude>1e-3);

  std::cout << std::setprecision(17)
            << "arms=" << result.arms.size() << '\n'
            << "maximum_denominator_residual="
            << result.maximum_denominator_residual << '\n'
            << "maximum_form_factor_residual="
            << result.maximum_form_factor_residual << '\n'
            << "minimum_normalized_forcing="
            << result.minimum_normalized_forcing << '\n'
            << "maximum_polarity_mirror_residual="
            << result.maximum_polarity_mirror_residual << '\n'
            << "maximum_cubic_covariance_residual="
            << result.maximum_cubic_covariance_residual << '\n'
            << "point_t256_error=" << result.point_t256_error << '\n'
            << "pair_t256_error=" << result.pair_t256_error << '\n'
            << "dipole_t256_error=" << result.dipole_t256_error << '\n'
            << "quadrupole_t256_error="
            << result.quadrupole_t256_error << '\n'
            << "same_plane_oblique_amplitude="
            << result.same_plane_oblique_amplitude << '\n'
            << "verdict="
            << (result.valid
                ? "HOP_SOURCE_MULTIPOLE_HIERARCHY_DERIVED"
                : "HOP_SOURCE_MULTIPOLE_HIERARCHY_FAILED") << '\n'
            << "failures=" << failures << '\n';
  return failures==0 ? 0 : 1;
}
