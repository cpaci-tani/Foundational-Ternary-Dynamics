/** FTD-0575: native Hodge reciprocity and static-pole audit. */

#include "ftd/eft/native_hodge_reciprocity.h"

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
  const auto result = ftd::eft::analyze_native_hodge_reciprocity();

  check("27 infrared symbol arms execute", result.infrared_symbol_arms == 27);
  check("lattice symbol identities close",
        result.maximum_kernel_identity_residual <= 1e-12);
  check("static kernel is bounded by three",
        result.minimum_static_kernel >= -1e-12
        && result.maximum_static_kernel <= 3.0 + 1e-12
        && result.maximum_kernel_bound_excess <= 1e-12);
  check("infrared kernel approaches three monotonically",
        result.static_charge_pole_canceled);
  check("24 proper-cubic rotations preserve the kernel",
        result.proper_cubic_rotation_arms == 24
        && result.maximum_cubic_covariance_residual <= 1e-12);
  check("12 static charge arms reproduce the Hodge potential",
        result.static_charge_arms == 12
        && result.maximum_charge_response_residual <= 1e-12);
  check("12 static transverse-current arms use the same kernel",
        result.static_transverse_current_arms == 12
        && result.maximum_current_response_residual <= 1e-12
        && result.static_current_pole_canceled);
  check("four Brillouin-corner controls vanish",
        result.brillouin_corner_controls == 4
        && result.maximum_corner_response <= 1e-12);
  check("periodic homogeneous identities close",
        result.periodic_operator_identity_arms == 4
        && result.homogeneous_identities_exact
        && result.maximum_divergence_of_b_residual <= 1e-12
        && result.maximum_faraday_residual <= 1e-12);
  check("source interaction rewrites as minimal Hodge coupling",
        result.smooth_path_variation_arms == 8
        && result.hodge_potentials_rewrite_interaction
        && result.maximum_interaction_rewrite_residual <= 1e-12);
  check("path variation gives the Hodge Lorentz form",
        result.lorentz_form_path_variation
        && result.maximum_path_variation_residual <= 1e-10);
  check("magnetic curvature performs zero scalar work",
        result.maximum_magnetic_scalar_work <= 1e-12);
  check("same polarity attracts and opposite polarity repels statically",
        result.same_polarity_static_interaction_attractive
        && result.largest_same_polarity_cross_energy < 0.0
        && result.smallest_opposite_polarity_cross_energy > 0.0);
  check("soft radiative residue vanishes quadratically",
        result.soft_radiative_residue_quadratic);
  check("no Coulomb, total-energy, or mobile-matter claim is promoted",
        !result.reciprocal_force_is_coulomb_electromagnetism
        && !result.exact_finite_step_total_energy_derived
        && !result.mobile_manifested_solution_derived
        && !result.production_changed);
  check("registered FTD-0575 verdict closes", result.valid);

  std::cout.precision(17);
  std::cout << "infrared_symbol_arms=" << result.infrared_symbol_arms << '\n'
            << "proper_cubic_rotation_arms="
            << result.proper_cubic_rotation_arms << '\n'
            << "static_charge_arms=" << result.static_charge_arms << '\n'
            << "static_transverse_current_arms="
            << result.static_transverse_current_arms << '\n'
            << "brillouin_corner_controls="
            << result.brillouin_corner_controls << '\n'
            << "periodic_operator_identity_arms="
            << result.periodic_operator_identity_arms << '\n'
            << "smooth_path_variation_arms="
            << result.smooth_path_variation_arms << '\n'
            << "minimum_static_kernel=" << result.minimum_static_kernel << '\n'
            << "maximum_static_kernel=" << result.maximum_static_kernel << '\n'
            << "maximum_kernel_identity_residual="
            << result.maximum_kernel_identity_residual << '\n'
            << "maximum_kernel_bound_excess="
            << result.maximum_kernel_bound_excess << '\n'
            << "maximum_charge_response_residual="
            << result.maximum_charge_response_residual << '\n'
            << "maximum_current_response_residual="
            << result.maximum_current_response_residual << '\n'
            << "maximum_cubic_covariance_residual="
            << result.maximum_cubic_covariance_residual << '\n'
            << "maximum_corner_response="
            << result.maximum_corner_response << '\n'
            << "maximum_divergence_of_b_residual="
            << result.maximum_divergence_of_b_residual << '\n'
            << "maximum_faraday_residual="
            << result.maximum_faraday_residual << '\n'
            << "maximum_interaction_rewrite_residual="
            << result.maximum_interaction_rewrite_residual << '\n'
            << "maximum_path_variation_residual="
            << result.maximum_path_variation_residual << '\n'
            << "maximum_magnetic_scalar_work="
            << result.maximum_magnetic_scalar_work << '\n'
            << "largest_same_polarity_cross_energy="
            << result.largest_same_polarity_cross_energy << '\n'
            << "smallest_opposite_polarity_cross_energy="
            << result.smallest_opposite_polarity_cross_energy << '\n'
            << "minimum_soft_residue=" << result.minimum_soft_residue << '\n'
            << "maximum_soft_residue=" << result.maximum_soft_residue << '\n'
            << "native_hodge_reciprocity failures=" << failures << '\n'
            << "verdict=NATIVE_HODGE_FORCE_DERIVED_STATIC_POLE_CANCELED_SAME_SIGN_ATTRACTIVE\n";
  return failures == 0 ? 0 : 1;
}
